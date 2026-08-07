// This file is part of the libCEC(R) library.
//
// libCEC(R) is Copyright (C) 2011-2026 Pulse-Eight Limited.  All rights reserved.
// libCEC(R) is an original work, containing original code.
//
// libCEC(R) is a trademark of Pulse-Eight Limited.
//
// This program is dual-licensed; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//
// Alternatively, you can license this library under a commercial license,
// please contact Pulse-Eight Licensing for more information.
//
// For more information contact:
// Pulse-Eight Licensing       <license@pulse-eight.com>
//     http://www.pulse-eight.com/
//     http://www.pulse-eight.net/

//! Opening a connection and driving the bus.

use std::ffi::CString;
use std::fmt;
use std::os::raw::{c_char, c_int, c_void};
use std::panic::{catch_unwind, AssertUnwindSafe};
use std::pin::Pin;
use std::ptr;
use std::sync::Arc;
use std::time::Duration;

use crate::callbacks::CecCallbacks;
use crate::enums::{
    Alert, CecVersion, DeckControlMode, DeckInfo, DeviceType, DisplayControl, LogicalAddress,
    MenuState, PlayMode, PowerStatus, UserControlCode,
};
use crate::error::{Error, Result};
use crate::ffi;
use crate::types::{
    set_device_name, AdapterDescriptor, AdapterStats, AudioStatus, Command, Configuration,
    Keypress, LogMessage, LogicalAddresses,
};
use crate::util::{as_c_bool, from_c_bool, read_fixed, read_ptr};

/// How long [`ConnectionBuilder::open`] waits for the adapter by default.
pub const DEFAULT_OPEN_TIMEOUT: Duration = Duration::from_secs(10);

/// The most adapters [`detect_adapters`](Connection::detect_adapters) reports.
///
/// libCEC fills a caller-supplied array, so somebody has to pick a number; 16 is
/// past any plausible number of CEC adapters on one machine.
const MAX_ADAPTERS: usize = 16;

// ---------------------------------------------------------------------------
// the pinned state libCEC holds pointers into
// ---------------------------------------------------------------------------

/// What libCEC is given the address of, and therefore what must not move.
///
/// `callbackParam` points at this struct and `callbacks` points at the table
/// inside it, so both stay valid for as long as the connection is open. It is
/// boxed and pinned for that reason, not for `async`.
struct Inner {
    handle: ffi::libcec_connection_t,
    callbacks: ffi::ICECCallbacks,
    handler: Option<Arc<dyn CecCallbacks>>,
}

impl Drop for Inner {
    fn drop(&mut self) {
        if !self.handle.is_null() {
            // SAFETY: handle came from libcec_initialise and is destroyed once.
            //
            // Order matters and it is the whole reason this impl exists:
            // libcec_destroy joins libCEC's worker thread, so once it returns no
            // callback can be running or start. `handler` is dropped after this
            // body, i.e. after that guarantee holds - which is what makes it
            // safe for a callback to have been holding a reference to it.
            unsafe {
                ffi::libcec_close(self.handle);
                ffi::libcec_destroy(self.handle);
            }
            self.handle = ptr::null_mut();
        }
    }
}

/// Find the handler behind libCEC's `cbparam` and run `f` against it.
///
/// Callbacks arrive on libCEC's thread, where a Rust panic unwinding into C
/// would be undefined behaviour, so every one is caught here and dropped. A
/// callback that panics loses its event; it does not take the process with it.
fn dispatch<F, R>(param: *mut c_void, default: R, f: F) -> R
where
    F: FnOnce(&dyn CecCallbacks) -> R,
{
    if param.is_null() {
        return default;
    }
    // SAFETY: param is the address of the pinned Inner, set at initialise time
    // and valid until libcec_destroy has returned - which is exactly the window
    // in which libCEC calls back.
    let inner = unsafe { &*(param as *const Inner) };
    let Some(handler) = inner.handler.as_ref() else {
        return default;
    };
    catch_unwind(AssertUnwindSafe(|| f(handler.as_ref()))).unwrap_or(default)
}

extern "C" fn trampoline_log(param: *mut c_void, message: *const ffi::cec_log_message) {
    if message.is_null() {
        return;
    }
    dispatch(param, (), |handler| {
        // SAFETY: libCEC guarantees the message is valid for this call only,
        // and LogMessage::from_raw copies everything it keeps.
        let message = unsafe { LogMessage::from_raw(&*message) };
        handler.log_message(&message);
    });
}

extern "C" fn trampoline_key(param: *mut c_void, key: *const ffi::cec_keypress) {
    if key.is_null() {
        return;
    }
    dispatch(param, (), |handler| {
        // SAFETY: valid for the duration of the call.
        let key = Keypress::from_raw(unsafe { &*key });
        handler.key_press(&key);
    });
}

extern "C" fn trampoline_command(param: *mut c_void, command: *const ffi::cec_command) {
    if command.is_null() {
        return;
    }
    dispatch(param, (), |handler| {
        // SAFETY: valid for the duration of the call.
        let command = Command::from_raw(unsafe { &*command });
        handler.command_received(&command);
    });
}

extern "C" fn trampoline_configuration(
    param: *mut c_void,
    configuration: *const ffi::libcec_configuration,
) {
    if configuration.is_null() {
        return;
    }
    dispatch(param, (), |handler| {
        // SAFETY: valid for the duration of the call.
        let configuration = Configuration::from_raw(unsafe { &*configuration });
        handler.configuration_changed(&configuration);
    });
}

extern "C" fn trampoline_alert(
    param: *mut c_void,
    alert: ffi::libcec_alert,
    _data: ffi::libcec_parameter,
) {
    // The parameter is a string for CEC_ALERT_SERVICE_DEVICE and unspecified
    // otherwise, with no length and no ownership rule. Not worth reading.
    dispatch(param, (), |handler| {
        handler.alert(Alert::from_raw(alert));
    });
}

extern "C" fn trampoline_source_activated(
    param: *mut c_void,
    address: ffi::cec_logical_address,
    activated: u8,
) {
    dispatch(param, (), |handler| {
        handler.source_activated(LogicalAddress::from_raw(address), activated != 0);
    });
}

extern "C" fn trampoline_menu_state(param: *mut c_void, state: ffi::cec_menu_state) -> c_int {
    dispatch(param, 0, |handler| {
        as_c_bool(handler.menu_state_changed(MenuState::from_raw(state)))
    })
}

extern "C" fn trampoline_command_handler(
    param: *mut c_void,
    command: *const ffi::cec_command,
) -> c_int {
    if command.is_null() {
        return 0;
    }
    dispatch(param, 0, |handler| {
        // SAFETY: valid for the duration of the call.
        let command = Command::from_raw(unsafe { &*command });
        as_c_bool(handler.command_handler(&command))
    })
}

// ---------------------------------------------------------------------------
// builder
// ---------------------------------------------------------------------------

/// Configures a connection, then opens it.
///
/// ```no_run
/// # fn main() -> Result<(), libcec::Error> {
/// use libcec::{ConnectionBuilder, enums::DeviceType};
///
/// let connection = ConnectionBuilder::new("RustCEC")
///     .device_type(DeviceType::PlaybackDevice)
///     .hdmi_port(1)
///     .activate_source(false)
///     .open_first()?;
/// # Ok(())
/// # }
/// ```
pub struct ConnectionBuilder {
    config: ffi::libcec_configuration,
    device_name: String,
    device_types: Vec<DeviceType>,
    handler: Option<Arc<dyn CecCallbacks>>,
}

impl ConnectionBuilder {
    /// Start from libCEC's defaults with the OSD name devices will see.
    ///
    /// The name is capped at 14 characters plus a terminator; a longer one is
    /// rejected by [`open`](Self::open) rather than quietly truncated onto
    /// somebody's television.
    pub fn new(device_name: impl Into<String>) -> Self {
        let mut config = ffi::libcec_configuration::default();
        // SAFETY: Clear() only writes; a zeroed struct is a valid target.
        unsafe { ffi::libcec_clear_configuration(&mut config) };
        ConnectionBuilder {
            config,
            device_name: device_name.into(),
            device_types: vec![DeviceType::RecordingDevice],
            handler: None,
        }
    }

    /// What to announce as. Defaults to [`DeviceType::RecordingDevice`], which
    /// is what most TVs give a usable logical address to.
    pub fn device_type(mut self, device_type: DeviceType) -> Self {
        self.device_types = vec![device_type];
        self
    }

    /// Claim more than one device type.
    pub fn device_types(mut self, device_types: impl Into<Vec<DeviceType>>) -> Self {
        self.device_types = device_types.into();
        self
    }

    /// The HDMI port the adapter is plugged into, counting from 1.
    ///
    /// Used with [`base_device`](Self::base_device) to work out the physical
    /// address when it cannot be detected.
    pub fn hdmi_port(mut self, port: u8) -> Self {
        self.config.iHDMIPort = port;
        self
    }

    /// The device the adapter is plugged into. Defaults to the TV.
    pub fn base_device(mut self, address: LogicalAddress) -> Self {
        self.config.baseDevice = address.raw();
        self
    }

    /// Set the physical address outright, disabling autodetection.
    pub fn physical_address(mut self, address: u16) -> Self {
        self.config.iPhysicalAddress = address;
        self.config.bAutodetectAddress = 0;
        self
    }

    /// Become the active source as soon as the connection opens - which on most
    /// televisions switches the input over. Defaults to libCEC's own setting.
    pub fn activate_source(mut self, activate: bool) -> Self {
        self.config.bActivateSource = as_c_bool(activate) as u8;
        self
    }

    /// Watch the bus without claiming a logical address.
    ///
    /// Nothing can be sent in this mode; it is for observing traffic.
    pub fn monitor_only(mut self, monitor: bool) -> Self {
        self.config.bMonitorOnly = as_c_bool(monitor) as u8;
        self
    }

    /// The CEC version to advertise. Defaults to 1.4.
    pub fn cec_version(mut self, version: CecVersion) -> Self {
        self.config.cecVersion = version.raw();
        self
    }

    /// Put this host in standby when the television switches off.
    pub fn power_off_on_standby(mut self, enable: bool) -> Self {
        self.config.bPowerOffOnStandby = as_c_bool(enable) as u8;
        self
    }

    /// Wake an AV receiver automatically when this client becomes the source.
    pub fn auto_wake_avr(mut self, enable: bool) -> Self {
        self.config.bAutoWakeAVR = as_c_bool(enable) as u8;
        self
    }

    /// Devices to wake on connect and on a bare
    /// [`power_on`](Connection::power_on).
    pub fn wake_devices(mut self, devices: &LogicalAddresses) -> Self {
        self.config.wakeDevices = devices.to_raw();
        self
    }

    /// Devices to put in standby on a bare [`standby`](Connection::standby).
    pub fn power_off_devices(mut self, devices: &LogicalAddresses) -> Self {
        self.config.powerOffDevices = devices.to_raw();
        self
    }

    /// Auto-repeat rate for held keys, or `None` to defer to the CEC device.
    pub fn button_repeat_rate(mut self, rate: Option<Duration>) -> Self {
        self.config.iButtonRepeatRateMs =
            rate.map_or(0, |r| r.as_millis().min(u32::MAX as u128) as u32);
        self
    }

    /// Suppress a repeated press of the same key inside this window. libCEC
    /// defaults to no suppression, forwarding every press.
    pub fn double_tap_timeout(mut self, timeout: Duration) -> Self {
        self.config.iDoubleTapTimeoutMs = timeout.as_millis().min(u32::MAX as u128) as u32;
        self
    }

    /// Where to send everything libCEC reports.
    ///
    /// Either an implementation of [`CecCallbacks`] or the handler half of
    /// [`callbacks::channel`](crate::callbacks::channel). Without this, libCEC
    /// is told to report nothing and never calls back at all.
    pub fn callbacks(mut self, handler: Arc<dyn CecCallbacks>) -> Self {
        self.handler = Some(handler);
        self
    }

    /// Open the first adapter that can be opened.
    ///
    /// Adapters that another process is already using are skipped, so this
    /// still finds the free one on a machine with several.
    pub fn open_first(self) -> Result<Connection> {
        self.open(None, DEFAULT_OPEN_TIMEOUT)
    }

    /// Open a specific port - `/dev/ttyACM0`, `COM3`, or the
    /// [`port`](AdapterDescriptor::port) from [`Connection::detect_adapters`].
    ///
    /// Pass `None` to open the first adapter that can be opened, which is what
    /// [`open_first`](Self::open_first) does. The timeout covers the call as a
    /// whole, however many adapters have to be tried.
    pub fn open(mut self, port: Option<&str>, timeout: Duration) -> Result<Connection> {
        set_device_name(&mut self.config, &self.device_name)?;

        // Clear() leaves every slot RESERVED, which is how libCEC spells "not
        // claimed"; fill from the front and leave the rest alone.
        for (slot, device_type) in self
            .config
            .deviceTypes
            .types
            .iter_mut()
            .zip(&self.device_types)
        {
            *slot = device_type.raw();
        }

        // Allocate the state libCEC will hold pointers into *before* handing it
        // those pointers, and never move it afterwards.
        let mut inner = Box::pin(Inner {
            handle: ptr::null_mut(),
            callbacks: ffi::ICECCallbacks::default(),
            handler: self.handler,
        });

        // SAFETY: nothing below moves out of the pin; the &mut is used only to
        // take addresses and to store the handle.
        let inner_mut = unsafe { inner.as_mut().get_unchecked_mut() };

        // Wire up only what the handler could possibly want. libCEC skips a null
        // slot entirely, so a connection with no handler costs nothing.
        if inner_mut.handler.is_some() {
            inner_mut.callbacks = ffi::ICECCallbacks {
                logMessage: Some(trampoline_log),
                keyPress: Some(trampoline_key),
                commandReceived: Some(trampoline_command),
                configurationChanged: Some(trampoline_configuration),
                alert: Some(trampoline_alert),
                menuStateChanged: Some(trampoline_menu_state),
                sourceActivated: Some(trampoline_source_activated),
                commandHandler: Some(trampoline_command_handler),
            };
            self.config.callbacks = &mut inner_mut.callbacks;
            self.config.callbackParam = inner_mut as *mut Inner as *mut c_void;
        }

        // SAFETY: config is fully initialised and outlives this call; libCEC
        // copies what it needs and keeps only the two pointers set above.
        let handle = unsafe { ffi::libcec_initialise(&mut self.config) };
        if handle.is_null() {
            return Err(Error::Initialise);
        }
        inner_mut.handle = handle;

        // Sets up the GPU-side EDID readers used to detect the physical address.
        // SAFETY: handle is live.
        unsafe { ffi::libcec_init_video_standalone(handle) };

        // A null port asks libCEC to open the first adapter it can, walking past
        // any that another process holds. Doing the detection here instead would
        // only be able to try one of them.
        let port_c = match port {
            Some(port) => Some(CString::new(port).map_err(|_| Error::InvalidString {
                field: "port",
                reason: "contains an interior NUL byte",
            })?),
            None => None,
        };
        let timeout_ms = timeout.as_millis().min(u32::MAX as u128) as u32;

        // SAFETY: handle is live; port_c, when there is one, is a valid C string
        // that outlives the call.
        let opened = from_c_bool(unsafe {
            ffi::libcec_open(
                handle,
                port_c.as_ref().map_or(ptr::null(), |port| port.as_ptr()),
                timeout_ms,
            )
        });
        if !opened {
            // `inner` drops here, which closes and destroys the connection.
            return Err(Error::Open(port.map(str::to_owned)));
        }

        Ok(Connection { inner })
    }
}

// ---------------------------------------------------------------------------
// connection
// ---------------------------------------------------------------------------

/// An open connection to a CEC adapter.
///
/// Closes itself when dropped.
pub struct Connection {
    inner: Pin<Box<Inner>>,
}

// SAFETY: the handle is an owned pointer to state libCEC allocated, and moving
// ownership of it to another thread is what libCEC's own API expects - the
// worker thread it starts already runs elsewhere.
unsafe impl Send for Connection {}

// SAFETY: libCEC serialises access to a connection internally - CLibCEC and the
// processor below it are behind their own locks, which is what lets its C++ API
// be driven from several threads and what the callback thread itself relies on.
// The Rust state beside the handle is only written while building the
// connection, before any of it is shared.
unsafe impl Sync for Connection {}

impl fmt::Debug for Connection {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // The handle is an opaque pointer into libCEC and the handler is a trait
        // object, so print what is actually knowable about the connection.
        f.debug_struct("Connection")
            .field("open", &!self.inner.handle.is_null())
            .field("callbacks", &self.inner.handler.is_some())
            .finish()
    }
}

impl Connection {
    fn handle(&self) -> ffi::libcec_connection_t {
        self.inner.handle
    }

    /// Every CEC adapter libCEC can find.
    ///
    /// A connection is needed to ask, so this opens a throwaway one; it does not
    /// open any adapter it finds. `quick` skips probing each port for its
    /// firmware details, which is faster but leaves
    /// [`firmware_version`](AdapterDescriptor::firmware_version) at 0.
    pub fn detect_adapters(quick: bool) -> Result<Vec<AdapterDescriptor>> {
        let mut config = ffi::libcec_configuration::default();
        // SAFETY: Clear() only writes.
        unsafe { ffi::libcec_clear_configuration(&mut config) };
        config.deviceTypes.types[0] = DeviceType::RecordingDevice.raw();

        // SAFETY: config outlives the connection below, and carries no callbacks.
        let handle = unsafe { ffi::libcec_initialise(&mut config) };
        if handle.is_null() {
            return Err(Error::Initialise);
        }

        let mut list = [ffi::cec_adapter_descriptor::default(); MAX_ADAPTERS];
        // SAFETY: handle is live and the buffer is MAX_ADAPTERS long, which is
        // the count passed.
        let found = unsafe {
            ffi::libcec_detect_adapters(
                handle,
                list.as_mut_ptr(),
                MAX_ADAPTERS as u8,
                ptr::null(),
                as_c_bool(quick),
            )
        };
        // SAFETY: handle is live and destroyed exactly once, here.
        unsafe { ffi::libcec_destroy(handle) };

        if found < 0 {
            return Err(Error::Call("detect adapters"));
        }
        Ok(list
            .iter()
            .take(found as usize)
            .map(AdapterDescriptor::from_raw)
            .collect())
    }

    /// What libCEC reports about itself: compiler, host and compiled-in backends.
    pub fn lib_info(&self) -> String {
        // SAFETY: handle is live; the string is static inside libCEC.
        unsafe { read_ptr(ffi::libcec_get_lib_info(self.handle())) }
    }

    // -- power ---------------------------------------------------------------

    /// Wake a device, or [`LogicalAddress::BROADCAST`] for everything
    /// configured through [`ConnectionBuilder::wake_devices`].
    pub fn power_on(&self, address: LogicalAddress) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_power_on_devices(self.handle(), address.raw()) },
            "power on devices",
        )
    }

    /// Put a device in standby, or [`LogicalAddress::BROADCAST`] for everything
    /// configured through [`ConnectionBuilder::power_off_devices`].
    pub fn standby(&self, address: LogicalAddress) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_standby_devices(self.handle(), address.raw()) },
            "put devices in standby",
        )
    }

    /// What a device says about its power state.
    pub fn power_status(&self, address: LogicalAddress) -> PowerStatus {
        // SAFETY: handle is live.
        PowerStatus::from_raw(unsafe {
            ffi::libcec_get_device_power_status(self.handle(), address.raw())
        })
    }

    // -- source --------------------------------------------------------------

    /// Announce this client as the active source, switching the TV's input.
    pub fn set_active_source(&self, device_type: DeviceType) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_active_source(self.handle(), device_type.raw()) },
            "set the active source",
        )
    }

    /// Give up being the active source.
    pub fn set_inactive_view(&self) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_inactive_view(self.handle()) },
            "set an inactive view",
        )
    }

    /// Who the bus currently believes is the active source.
    pub fn active_source(&self) -> LogicalAddress {
        // SAFETY: handle is live.
        LogicalAddress::from_raw(unsafe { ffi::libcec_get_active_source(self.handle()) })
    }

    /// Whether `address` is the active source.
    pub fn is_active_source(&self, address: LogicalAddress) -> bool {
        // SAFETY: handle is live.
        from_c_bool(unsafe { ffi::libcec_is_active_source(self.handle(), address.raw()) })
    }

    /// Whether this client is the active source.
    pub fn is_libcec_active_source(&self) -> bool {
        // SAFETY: handle is live.
        from_c_bool(unsafe { ffi::libcec_is_libcec_active_source(self.handle()) })
    }

    /// Ask the TV to route to a physical address.
    pub fn set_stream_path(&self, physical_address: u16) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_stream_path_physical(self.handle(), physical_address) },
            "set the stream path",
        )
    }

    /// Ask the TV to route to whatever is at a logical address.
    pub fn set_stream_path_to(&self, address: LogicalAddress) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_stream_path_logical(self.handle(), address.raw()) },
            "set the stream path",
        )
    }

    // -- messages ------------------------------------------------------------

    /// Send a raw CEC message.
    pub fn transmit(&self, command: &Command) -> Result<()> {
        let raw = command.to_raw()?;
        // SAFETY: handle is live; raw is a fully initialised local.
        self.check(
            unsafe { ffi::libcec_transmit(self.handle(), &raw) },
            "transmit a command",
        )
    }

    /// Send a keypress. With `wait`, blocks until the device answers.
    pub fn send_keypress(
        &self,
        destination: LogicalAddress,
        key: UserControlCode,
        wait: bool,
    ) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe {
                ffi::libcec_send_keypress(
                    self.handle(),
                    destination.raw(),
                    key.raw(),
                    as_c_bool(wait),
                )
            },
            "send a keypress",
        )
    }

    /// Release a key sent with [`send_keypress`](Self::send_keypress).
    pub fn send_key_release(&self, destination: LogicalAddress, wait: bool) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe {
                ffi::libcec_send_key_release(self.handle(), destination.raw(), as_c_bool(wait))
            },
            "send a key release",
        )
    }

    /// Ask a device to play, in a given direction and at a given speed.
    ///
    /// Not every device acts on `<play>`; LG players in particular only
    /// respond to the equivalent remote key, which libCEC sends instead when
    /// it knows it is talking to one.
    pub fn send_play(&self, destination: LogicalAddress, mode: PlayMode) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_send_play(self.handle(), destination.raw(), mode.raw()) },
            "send a play command",
        )
    }

    /// Put a message on a device's on-screen display.
    ///
    /// Not every television implements this, and those that do often cap the
    /// length well below what CEC allows.
    pub fn set_osd_string(
        &self,
        destination: LogicalAddress,
        duration: DisplayControl,
        message: &str,
    ) -> Result<()> {
        let message = CString::new(message).map_err(|_| Error::InvalidString {
            field: "OSD string",
            reason: "contains an interior NUL byte",
        })?;
        // SAFETY: handle is live; message outlives the call.
        self.check(
            unsafe {
                ffi::libcec_set_osd_string(
                    self.handle(),
                    destination.raw(),
                    duration.raw(),
                    message.as_ptr(),
                )
            },
            "set an OSD string",
        )
    }

    // -- audio ---------------------------------------------------------------

    /// Volume up on the amplifier. Returns the new audio status.
    pub fn volume_up(&self, send_release: bool) -> AudioStatus {
        // SAFETY: handle is live.
        AudioStatus(unsafe { ffi::libcec_volume_up(self.handle(), as_c_bool(send_release)) } as u8)
    }

    /// Volume down on the amplifier. Returns the new audio status.
    pub fn volume_down(&self, send_release: bool) -> AudioStatus {
        // SAFETY: handle is live.
        AudioStatus(
            unsafe { ffi::libcec_volume_down(self.handle(), as_c_bool(send_release)) } as u8,
        )
    }

    /// Toggle mute on the amplifier. Returns the new audio status.
    pub fn mute_audio(&self, send_release: bool) -> AudioStatus {
        // SAFETY: handle is live.
        AudioStatus(unsafe { ffi::libcec_mute_audio(self.handle(), as_c_bool(send_release)) } as u8)
    }

    /// Mute the amplifier. Returns the new audio status.
    pub fn mute(&self) -> AudioStatus {
        // SAFETY: handle is live.
        AudioStatus(unsafe { ffi::libcec_audio_mute(self.handle()) })
    }

    /// Unmute the amplifier. Returns the new audio status.
    pub fn unmute(&self) -> AudioStatus {
        // SAFETY: handle is live.
        AudioStatus(unsafe { ffi::libcec_audio_unmute(self.handle()) })
    }

    /// Ask the amplifier for its audio status.
    pub fn audio_status(&self) -> AudioStatus {
        // SAFETY: handle is live.
        AudioStatus(unsafe { ffi::libcec_audio_get_status(self.handle()) })
    }

    /// Turn system audio mode on or off.
    pub fn set_system_audio_mode(&self, enable: bool) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_system_audio_mode(self.handle(), as_c_bool(enable)) },
            "set system audio mode",
        )
    }

    // -- the bus -------------------------------------------------------------

    /// Knock at a logical address and see whether anyone answers.
    pub fn poll_device(&self, address: LogicalAddress) -> bool {
        // SAFETY: handle is live.
        from_c_bool(unsafe { ffi::libcec_poll_device(self.handle(), address.raw()) })
    }

    /// Every address that answered.
    pub fn active_devices(&self) -> LogicalAddresses {
        // SAFETY: handle is live; the struct is returned by value.
        let raw = unsafe { ffi::libcec_get_active_devices(self.handle()) };
        LogicalAddresses::from_raw(&raw)
    }

    /// Whether a device is present at `address`.
    pub fn is_active_device(&self, address: LogicalAddress) -> bool {
        // SAFETY: handle is live.
        from_c_bool(unsafe { ffi::libcec_is_active_device(self.handle(), address.raw()) })
    }

    /// Whether any device of this type is on the bus.
    pub fn is_active_device_type(&self, device_type: DeviceType) -> bool {
        // SAFETY: handle is live.
        from_c_bool(unsafe { ffi::libcec_is_active_device_type(self.handle(), device_type.raw()) })
    }

    /// The addresses this client holds.
    pub fn logical_addresses(&self) -> LogicalAddresses {
        // SAFETY: handle is live; the struct is returned by value.
        let raw = unsafe { ffi::libcec_get_logical_addresses(self.handle()) };
        LogicalAddresses::from_raw(&raw)
    }

    /// Re-poll every address. Useful after the HDMI tree changes.
    pub fn rescan_devices(&self) {
        // SAFETY: handle is live.
        unsafe { ffi::libcec_rescan_devices(self.handle()) }
    }

    /// Check the adapter is still answering.
    pub fn ping_adapter(&self) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_ping_adapters(self.handle()) },
            "ping the adapter",
        )
    }

    // -- devices -------------------------------------------------------------

    /// A device's OSD name, as it would appear in an input list.
    pub fn device_osd_name(&self, address: LogicalAddress) -> String {
        let mut name = [0 as c_char; ffi::CEC_OSD_NAME_SIZE];
        // SAFETY: handle is live; name is CEC_OSD_NAME_SIZE, which is the width
        // libCEC's cec_osd_name typedef promises.
        unsafe {
            ffi::libcec_get_device_osd_name(self.handle(), address.raw(), name.as_mut_ptr());
        }
        read_fixed(&name)
    }

    /// A device's vendor id.
    pub fn device_vendor_id(&self, address: LogicalAddress) -> u32 {
        // SAFETY: handle is live.
        unsafe { ffi::libcec_get_device_vendor_id(self.handle(), address.raw()) }
    }

    /// A device's physical address on the HDMI tree.
    pub fn device_physical_address(&self, address: LogicalAddress) -> u16 {
        // SAFETY: handle is live.
        unsafe { ffi::libcec_get_device_physical_address(self.handle(), address.raw()) }
    }

    /// The CEC version a device speaks.
    pub fn device_cec_version(&self, address: LogicalAddress) -> CecVersion {
        // SAFETY: handle is live.
        CecVersion::from_raw(unsafe {
            ffi::libcec_get_device_cec_version(self.handle(), address.raw())
        })
    }

    /// A device's menu language, as a 3-character ISO 639-2 code.
    pub fn device_menu_language(&self, address: LogicalAddress) -> Option<String> {
        let mut language = [0 as c_char; ffi::CEC_MENU_LANGUAGE_SIZE];
        // SAFETY: handle is live; the buffer is the width cec_menu_language
        // promises.
        let ok = unsafe {
            ffi::libcec_get_device_menu_language(
                self.handle(),
                address.raw(),
                language.as_mut_ptr(),
            )
        };
        if from_c_bool(ok) {
            Some(read_fixed(&language))
        } else {
            None
        }
    }

    // -- configuration -------------------------------------------------------

    /// What libCEC is currently configured to do.
    pub fn configuration(&self) -> Result<Configuration> {
        let mut raw = ffi::libcec_configuration::default();
        // SAFETY: handle is live; raw is a valid target for a full write.
        let ok = unsafe { ffi::libcec_get_current_configuration(self.handle(), &mut raw) };
        if from_c_bool(ok) {
            Ok(Configuration::from_raw(&raw))
        } else {
            Err(Error::Call("read the current configuration"))
        }
    }

    /// Whether the adapter can persist settings to its EEPROM.
    pub fn can_save_configuration(&self) -> bool {
        // SAFETY: handle is live.
        from_c_bool(unsafe { ffi::libcec_can_save_configuration(self.handle()) })
    }

    /// Move to a different HDMI port without reopening.
    pub fn set_hdmi_port(&self, base_device: LogicalAddress, port: u8) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_hdmi_port(self.handle(), base_device.raw(), port) },
            "set the HDMI port",
        )
    }

    /// Set the physical address directly.
    pub fn set_physical_address(&self, address: u16) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_physical_address(self.handle(), address) },
            "set the physical address",
        )
    }

    /// Watch the bus without claiming an address, or stop doing so.
    pub fn switch_monitoring(&self, enable: bool) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_switch_monitoring(self.handle(), as_c_bool(enable)) },
            "switch monitoring mode",
        )
    }

    /// Announce a deck control mode.
    pub fn set_deck_control_mode(&self, mode: DeckControlMode, send_update: bool) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe {
                ffi::libcec_set_deck_control_mode(self.handle(), mode.raw(), as_c_bool(send_update))
            },
            "set the deck control mode",
        )
    }

    /// Announce deck status.
    pub fn set_deck_info(&self, info: DeckInfo, send_update: bool) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_set_deck_info(self.handle(), info.raw(), as_c_bool(send_update)) },
            "set the deck info",
        )
    }

    /// Announce a menu state.
    pub fn set_menu_state(&self, state: MenuState, send_update: bool) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe {
                ffi::libcec_set_menu_state(self.handle(), state.raw(), as_c_bool(send_update))
            },
            "set the menu state",
        )
    }

    // -- adapter -------------------------------------------------------------

    /// The adapter's frame counters.
    pub fn stats(&self) -> Result<AdapterStats> {
        let mut raw = ffi::cec_adapter_stats::default();
        // SAFETY: handle is live; raw is a valid target.
        let ok = unsafe { ffi::libcec_get_stats(self.handle(), &mut raw) };
        if from_c_bool(ok) {
            Ok(AdapterStats::from_raw(&raw))
        } else {
            Err(Error::Call("read the adapter statistics"))
        }
    }

    /// The adapter's USB vendor id, or 0 for a SoC-native backend.
    pub fn adapter_vendor_id(&self) -> u16 {
        // SAFETY: handle is live.
        unsafe { ffi::libcec_get_adapter_vendor_id(self.handle()) }
    }

    /// The adapter's USB product id, or 0 for a SoC-native backend.
    pub fn adapter_product_id(&self) -> u16 {
        // SAFETY: handle is live.
        unsafe { ffi::libcec_get_adapter_product_id(self.handle()) }
    }

    /// Put the adapter into its bootloader for a firmware update.
    ///
    /// The connection is unusable afterwards: the adapter stops speaking CEC
    /// until it is flashed and power-cycled.
    pub fn start_bootloader(&self) -> Result<()> {
        // SAFETY: handle is live.
        self.check(
            unsafe { ffi::libcec_start_bootloader(self.handle()) },
            "start the bootloader",
        )
    }

    /// Turn libCEC's `int`-as-boolean into a `Result` that names the call.
    fn check(&self, ok: c_int, what: &'static str) -> Result<()> {
        if from_c_bool(ok) {
            Ok(())
        } else {
            Err(Error::Call(what))
        }
    }
}
