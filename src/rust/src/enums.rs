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

// GENERATED FILE - do not edit by hand.
//
// Regenerate with:
//
//     python support/generate-rust-enums.py
//
// The source of truth is include/cectypes.h. The generator is what keeps ~250
// protocol constants honest; see its docstring for why the enums look the way
// they do (Other(i32) catch-alls, aliased duplicate values).

//! The protocol enums: opcodes, logical addresses, key codes, vendor ids and the
//! rest of `cectypes.h`.
//!
//! Every enum here is **total**: [`from_raw`](Opcode::from_raw) never fails, and
//! a value this crate has no name for arrives as `Other(i32)` rather than an
//! error. The CEC bus carries whatever the devices on it decide to send, and a
//! binding that panicked on an unrecognised opcode would be a liability.
//!
//! Where libCEC has a `*_to_string` helper, [`Display`](std::fmt::Display) calls
//! it. The strings therefore come from libCEC itself and cannot drift from the
//! ones `cec-client` prints.
//!
//! ```no_run
//! use libcec::enums::{LogicalAddress, Opcode};
//!
//! assert_eq!(LogicalAddress::Tv.raw(), 0);
//! assert_eq!(LogicalAddress::from_raw(99), LogicalAddress::Other(99));
//! println!("{}", Opcode::ImageViewOn); // asks libCEC for the name
//! ```

use std::fmt;

use crate::ffi;
use crate::util::describe;

/// Why a device refused a command.
///
/// Mirrors the C `cec_abort_reason`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum AbortReason {
    /// `CEC_ABORT_REASON_UNRECOGNIZED_OPCODE`
    UnrecognizedOpcode,
    /// `CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND`
    NotInCorrectModeToRespond,
    /// `CEC_ABORT_REASON_CANNOT_PROVIDE_SOURCE`
    CannotProvideSource,
    /// `CEC_ABORT_REASON_INVALID_OPERAND`
    InvalidOperand,
    /// `CEC_ABORT_REASON_REFUSED`
    Refused,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl AbortReason {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            AbortReason::UnrecognizedOpcode => 0,
            AbortReason::NotInCorrectModeToRespond => 1,
            AbortReason::CannotProvideSource => 2,
            AbortReason::InvalidOperand => 3,
            AbortReason::Refused => 4,
            AbortReason::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`AbortReason::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => AbortReason::UnrecognizedOpcode,
            1 => AbortReason::NotInCorrectModeToRespond,
            2 => AbortReason::CannotProvideSource,
            3 => AbortReason::InvalidOperand,
            4 => AbortReason::Refused,
            other => AbortReason::Other(other),
        }
    }
}

impl From<i32> for AbortReason {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<AbortReason> for i32 {
    fn from(value: AbortReason) -> Self {
        value.raw()
    }
}

/// Which backend an adapter is driven by.
///
/// Mirrors the C `cec_adapter_type`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum AdapterType {
    /// `ADAPTERTYPE_UNKNOWN`
    Unknown,
    /// `ADAPTERTYPE_P8_EXTERNAL`
    P8External,
    /// `ADAPTERTYPE_P8_DAUGHTERBOARD`
    P8Daughterboard,
    /// `ADAPTERTYPE_RPI`
    Rpi,
    /// `ADAPTERTYPE_TDA995x`
    Tda995x,
    /// `ADAPTERTYPE_EXYNOS`
    Exynos,
    /// `ADAPTERTYPE_LINUX`
    Linux,
    /// `ADAPTERTYPE_AOCEC`
    Aocec,
    /// `ADAPTERTYPE_IMX`
    Imx,
    /// `ADAPTERTYPE_TEGRA`
    Tegra,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl AdapterType {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            AdapterType::Unknown => 0,
            AdapterType::P8External => 1,
            AdapterType::P8Daughterboard => 2,
            AdapterType::Rpi => 256,
            AdapterType::Tda995x => 512,
            AdapterType::Exynos => 768,
            AdapterType::Linux => 1024,
            AdapterType::Aocec => 1280,
            AdapterType::Imx => 1536,
            AdapterType::Tegra => 1792,
            AdapterType::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`AdapterType::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => AdapterType::Unknown,
            1 => AdapterType::P8External,
            2 => AdapterType::P8Daughterboard,
            256 => AdapterType::Rpi,
            512 => AdapterType::Tda995x,
            768 => AdapterType::Exynos,
            1024 => AdapterType::Linux,
            1280 => AdapterType::Aocec,
            1536 => AdapterType::Imx,
            1792 => AdapterType::Tegra,
            other => AdapterType::Other(other),
        }
    }
}

impl From<i32> for AdapterType {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<AdapterType> for i32 {
    fn from(value: AdapterType) -> Self {
        value.raw()
    }
}

impl fmt::Display for AdapterType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_adapter_type_to_string, self.raw()))
    }
}

/// What libCEC knows about a device on the bus.
///
/// Mirrors the C `cec_bus_device_status`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum BusDeviceStatus {
    /// `CEC_DEVICE_STATUS_UNKNOWN`
    Unknown,
    /// `CEC_DEVICE_STATUS_PRESENT`
    Present,
    /// `CEC_DEVICE_STATUS_NOT_PRESENT`
    NotPresent,
    /// `CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC`
    HandledByLibcec,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl BusDeviceStatus {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            BusDeviceStatus::Unknown => 0,
            BusDeviceStatus::Present => 1,
            BusDeviceStatus::NotPresent => 2,
            BusDeviceStatus::HandledByLibcec => 3,
            BusDeviceStatus::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`BusDeviceStatus::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => BusDeviceStatus::Unknown,
            1 => BusDeviceStatus::Present,
            2 => BusDeviceStatus::NotPresent,
            3 => BusDeviceStatus::HandledByLibcec,
            other => BusDeviceStatus::Other(other),
        }
    }
}

impl From<i32> for BusDeviceStatus {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<BusDeviceStatus> for i32 {
    fn from(value: BusDeviceStatus) -> Self {
        value.raw()
    }
}

/// Deck transport control.
///
/// Mirrors the C `cec_deck_control_mode`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum DeckControlMode {
    /// `CEC_DECK_CONTROL_MODE_SKIP_FORWARD_WIND`
    SkipForwardWind,
    /// `CEC_DECK_CONTROL_MODE_SKIP_REVERSE_REWIND`
    SkipReverseRewind,
    /// `CEC_DECK_CONTROL_MODE_STOP`
    Stop,
    /// `CEC_DECK_CONTROL_MODE_EJECT`
    Eject,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl DeckControlMode {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            DeckControlMode::SkipForwardWind => 1,
            DeckControlMode::SkipReverseRewind => 2,
            DeckControlMode::Stop => 3,
            DeckControlMode::Eject => 4,
            DeckControlMode::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`DeckControlMode::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            1 => DeckControlMode::SkipForwardWind,
            2 => DeckControlMode::SkipReverseRewind,
            3 => DeckControlMode::Stop,
            4 => DeckControlMode::Eject,
            other => DeckControlMode::Other(other),
        }
    }
}

impl From<i32> for DeckControlMode {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<DeckControlMode> for i32 {
    fn from(value: DeckControlMode) -> Self {
        value.raw()
    }
}

impl fmt::Display for DeckControlMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(
            ffi::libcec_deck_control_mode_to_string,
            self.raw(),
        ))
    }
}

/// The state a deck reports.
///
/// Mirrors the C `cec_deck_info`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum DeckInfo {
    /// `CEC_DECK_INFO_PLAY`
    Play,
    /// `CEC_DECK_INFO_RECORD`
    Record,
    /// `CEC_DECK_INFO_PLAY_REVERSE`
    PlayReverse,
    /// `CEC_DECK_INFO_STILL`
    Still,
    /// `CEC_DECK_INFO_SLOW`
    Slow,
    /// `CEC_DECK_INFO_SLOW_REVERSE`
    SlowReverse,
    /// `CEC_DECK_INFO_FAST_FORWARD`
    FastForward,
    /// `CEC_DECK_INFO_FAST_REVERSE`
    FastReverse,
    /// `CEC_DECK_INFO_NO_MEDIA`
    NoMedia,
    /// `CEC_DECK_INFO_STOP`
    Stop,
    /// `CEC_DECK_INFO_SKIP_FORWARD_WIND`
    SkipForwardWind,
    /// `CEC_DECK_INFO_SKIP_REVERSE_REWIND`
    SkipReverseRewind,
    /// `CEC_DECK_INFO_INDEX_SEARCH_FORWARD`
    IndexSearchForward,
    /// `CEC_DECK_INFO_INDEX_SEARCH_REVERSE`
    IndexSearchReverse,
    /// `CEC_DECK_INFO_OTHER_STATUS`
    OtherStatus,
    /// `CEC_DECK_INFO_OTHER_STATUS_LG`
    OtherStatusLg,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl DeckInfo {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            DeckInfo::Play => 17,
            DeckInfo::Record => 18,
            DeckInfo::PlayReverse => 19,
            DeckInfo::Still => 20,
            DeckInfo::Slow => 21,
            DeckInfo::SlowReverse => 22,
            DeckInfo::FastForward => 23,
            DeckInfo::FastReverse => 24,
            DeckInfo::NoMedia => 25,
            DeckInfo::Stop => 26,
            DeckInfo::SkipForwardWind => 27,
            DeckInfo::SkipReverseRewind => 28,
            DeckInfo::IndexSearchForward => 29,
            DeckInfo::IndexSearchReverse => 30,
            DeckInfo::OtherStatus => 31,
            DeckInfo::OtherStatusLg => 32,
            DeckInfo::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`DeckInfo::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            17 => DeckInfo::Play,
            18 => DeckInfo::Record,
            19 => DeckInfo::PlayReverse,
            20 => DeckInfo::Still,
            21 => DeckInfo::Slow,
            22 => DeckInfo::SlowReverse,
            23 => DeckInfo::FastForward,
            24 => DeckInfo::FastReverse,
            25 => DeckInfo::NoMedia,
            26 => DeckInfo::Stop,
            27 => DeckInfo::SkipForwardWind,
            28 => DeckInfo::SkipReverseRewind,
            29 => DeckInfo::IndexSearchForward,
            30 => DeckInfo::IndexSearchReverse,
            31 => DeckInfo::OtherStatus,
            32 => DeckInfo::OtherStatusLg,
            other => DeckInfo::Other(other),
        }
    }
}

impl From<i32> for DeckInfo {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<DeckInfo> for i32 {
    fn from(value: DeckInfo) -> Self {
        value.raw()
    }
}

impl fmt::Display for DeckInfo {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_deck_status_to_string, self.raw()))
    }
}

/// The device type a client announces on the bus.
///
/// Mirrors the C `cec_device_type`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum DeviceType {
    /// `CEC_DEVICE_TYPE_TV`
    Tv,
    /// `CEC_DEVICE_TYPE_RECORDING_DEVICE`
    RecordingDevice,
    /// `CEC_DEVICE_TYPE_RESERVED`
    Reserved,
    /// `CEC_DEVICE_TYPE_TUNER`
    Tuner,
    /// `CEC_DEVICE_TYPE_PLAYBACK_DEVICE`
    PlaybackDevice,
    /// `CEC_DEVICE_TYPE_AUDIO_SYSTEM`
    AudioSystem,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl DeviceType {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            DeviceType::Tv => 0,
            DeviceType::RecordingDevice => 1,
            DeviceType::Reserved => 2,
            DeviceType::Tuner => 3,
            DeviceType::PlaybackDevice => 4,
            DeviceType::AudioSystem => 5,
            DeviceType::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`DeviceType::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => DeviceType::Tv,
            1 => DeviceType::RecordingDevice,
            2 => DeviceType::Reserved,
            3 => DeviceType::Tuner,
            4 => DeviceType::PlaybackDevice,
            5 => DeviceType::AudioSystem,
            other => DeviceType::Other(other),
        }
    }
}

impl From<i32> for DeviceType {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<DeviceType> for i32 {
    fn from(value: DeviceType) -> Self {
        value.raw()
    }
}

/// How long an OSD string stays on screen.
///
/// Mirrors the C `cec_display_control`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum DisplayControl {
    /// `CEC_DISPLAY_CONTROL_DISPLAY_FOR_DEFAULT_TIME`
    DisplayForDefaultTime,
    /// `CEC_DISPLAY_CONTROL_DISPLAY_UNTIL_CLEARED`
    DisplayUntilCleared,
    /// `CEC_DISPLAY_CONTROL_CLEAR_PREVIOUS_MESSAGE`
    ClearPreviousMessage,
    /// `CEC_DISPLAY_CONTROL_RESERVED_FOR_FUTURE_USE`
    ReservedForFutureUse,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl DisplayControl {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            DisplayControl::DisplayForDefaultTime => 0,
            DisplayControl::DisplayUntilCleared => 64,
            DisplayControl::ClearPreviousMessage => 128,
            DisplayControl::ReservedForFutureUse => 192,
            DisplayControl::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`DisplayControl::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => DisplayControl::DisplayForDefaultTime,
            64 => DisplayControl::DisplayUntilCleared,
            128 => DisplayControl::ClearPreviousMessage,
            192 => DisplayControl::ReservedForFutureUse,
            other => DisplayControl::Other(other),
        }
    }
}

impl From<i32> for DisplayControl {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<DisplayControl> for i32 {
    fn from(value: DisplayControl) -> Self {
        value.raw()
    }
}

/// Severity of a log message from libCEC.
///
/// Mirrors the C `cec_log_level`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum LogLevel {
    /// `CEC_LOG_ERROR`
    Error,
    /// `CEC_LOG_WARNING`
    Warning,
    /// `CEC_LOG_NOTICE`
    Notice,
    /// `CEC_LOG_TRAFFIC`
    Traffic,
    /// `CEC_LOG_DEBUG`
    Debug,
    /// `CEC_LOG_ALL`
    All,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl LogLevel {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            LogLevel::Error => 1,
            LogLevel::Warning => 2,
            LogLevel::Notice => 4,
            LogLevel::Traffic => 8,
            LogLevel::Debug => 16,
            LogLevel::All => 31,
            LogLevel::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`LogLevel::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            1 => LogLevel::Error,
            2 => LogLevel::Warning,
            4 => LogLevel::Notice,
            8 => LogLevel::Traffic,
            16 => LogLevel::Debug,
            31 => LogLevel::All,
            other => LogLevel::Other(other),
        }
    }
}

impl From<i32> for LogLevel {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<LogLevel> for i32 {
    fn from(value: LogLevel) -> Self {
        value.raw()
    }
}

/// A CEC logical address - who a message is from or to.
///
/// Mirrors the C `cec_logical_address`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug, Default)]
pub enum LogicalAddress {
    /// `CECDEVICE_UNKNOWN`
    #[default]
    Unknown,
    /// `CECDEVICE_TV`
    Tv,
    /// `CECDEVICE_RECORDINGDEVICE1`
    RecordingDevice1,
    /// `CECDEVICE_RECORDINGDEVICE2`
    RecordingDevice2,
    /// `CECDEVICE_TUNER1`
    Tuner1,
    /// `CECDEVICE_PLAYBACKDEVICE1`
    PlaybackDevice1,
    /// `CECDEVICE_AUDIOSYSTEM`
    AudioSystem,
    /// `CECDEVICE_TUNER2`
    Tuner2,
    /// `CECDEVICE_TUNER3`
    Tuner3,
    /// `CECDEVICE_PLAYBACKDEVICE2`
    PlaybackDevice2,
    /// `CECDEVICE_RECORDINGDEVICE3`
    RecordingDevice3,
    /// `CECDEVICE_TUNER4`
    Tuner4,
    /// `CECDEVICE_PLAYBACKDEVICE3`
    PlaybackDevice3,
    /// `CECDEVICE_RESERVED1`
    Reserved1,
    /// `CECDEVICE_RESERVED2`
    Reserved2,
    /// `CECDEVICE_FREEUSE`
    FreeUse,
    /// `CECDEVICE_UNREGISTERED`
    Unregistered,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl LogicalAddress {
    /// `CECDEVICE_BROADCAST` - the same value as [`LogicalAddress::Unregistered`] (15).
    pub const BROADCAST: LogicalAddress = LogicalAddress::Unregistered;
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            LogicalAddress::Unknown => -1,
            LogicalAddress::Tv => 0,
            LogicalAddress::RecordingDevice1 => 1,
            LogicalAddress::RecordingDevice2 => 2,
            LogicalAddress::Tuner1 => 3,
            LogicalAddress::PlaybackDevice1 => 4,
            LogicalAddress::AudioSystem => 5,
            LogicalAddress::Tuner2 => 6,
            LogicalAddress::Tuner3 => 7,
            LogicalAddress::PlaybackDevice2 => 8,
            LogicalAddress::RecordingDevice3 => 9,
            LogicalAddress::Tuner4 => 10,
            LogicalAddress::PlaybackDevice3 => 11,
            LogicalAddress::Reserved1 => 12,
            LogicalAddress::Reserved2 => 13,
            LogicalAddress::FreeUse => 14,
            LogicalAddress::Unregistered => 15,
            LogicalAddress::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`LogicalAddress::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            -1 => LogicalAddress::Unknown,
            0 => LogicalAddress::Tv,
            1 => LogicalAddress::RecordingDevice1,
            2 => LogicalAddress::RecordingDevice2,
            3 => LogicalAddress::Tuner1,
            4 => LogicalAddress::PlaybackDevice1,
            5 => LogicalAddress::AudioSystem,
            6 => LogicalAddress::Tuner2,
            7 => LogicalAddress::Tuner3,
            8 => LogicalAddress::PlaybackDevice2,
            9 => LogicalAddress::RecordingDevice3,
            10 => LogicalAddress::Tuner4,
            11 => LogicalAddress::PlaybackDevice3,
            12 => LogicalAddress::Reserved1,
            13 => LogicalAddress::Reserved2,
            14 => LogicalAddress::FreeUse,
            15 => LogicalAddress::Unregistered,
            other => LogicalAddress::Other(other),
        }
    }
}

impl From<i32> for LogicalAddress {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<LogicalAddress> for i32 {
    fn from(value: LogicalAddress) -> Self {
        value.raw()
    }
}

impl fmt::Display for LogicalAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_logical_address_to_string, self.raw()))
    }
}

/// Whether the device menu is active.
///
/// Mirrors the C `cec_menu_state`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum MenuState {
    /// `CEC_MENU_STATE_ACTIVATED`
    Activated,
    /// `CEC_MENU_STATE_DEACTIVATED`
    Deactivated,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl MenuState {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            MenuState::Activated => 0,
            MenuState::Deactivated => 1,
            MenuState::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`MenuState::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => MenuState::Activated,
            1 => MenuState::Deactivated,
            other => MenuState::Other(other),
        }
    }
}

impl From<i32> for MenuState {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<MenuState> for i32 {
    fn from(value: MenuState) -> Self {
        value.raw()
    }
}

impl fmt::Display for MenuState {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_menu_state_to_string, self.raw()))
    }
}

/// A CEC message opcode.
///
/// Mirrors the C `cec_opcode`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum Opcode {
    /// `CEC_OPCODE_ACTIVE_SOURCE`
    ActiveSource,
    /// `CEC_OPCODE_IMAGE_VIEW_ON`
    ImageViewOn,
    /// `CEC_OPCODE_TEXT_VIEW_ON`
    TextViewOn,
    /// `CEC_OPCODE_INACTIVE_SOURCE`
    InactiveSource,
    /// `CEC_OPCODE_REQUEST_ACTIVE_SOURCE`
    RequestActiveSource,
    /// `CEC_OPCODE_ROUTING_CHANGE`
    RoutingChange,
    /// `CEC_OPCODE_ROUTING_INFORMATION`
    RoutingInformation,
    /// `CEC_OPCODE_SET_STREAM_PATH`
    SetStreamPath,
    /// `CEC_OPCODE_STANDBY`
    Standby,
    /// `CEC_OPCODE_RECORD_OFF`
    RecordOff,
    /// `CEC_OPCODE_RECORD_ON`
    RecordOn,
    /// `CEC_OPCODE_RECORD_STATUS`
    RecordStatus,
    /// `CEC_OPCODE_RECORD_TV_SCREEN`
    RecordTvScreen,
    /// `CEC_OPCODE_CLEAR_ANALOGUE_TIMER`
    ClearAnalogueTimer,
    /// `CEC_OPCODE_CLEAR_DIGITAL_TIMER`
    ClearDigitalTimer,
    /// `CEC_OPCODE_CLEAR_EXTERNAL_TIMER`
    ClearExternalTimer,
    /// `CEC_OPCODE_SET_ANALOGUE_TIMER`
    SetAnalogueTimer,
    /// `CEC_OPCODE_SET_DIGITAL_TIMER`
    SetDigitalTimer,
    /// `CEC_OPCODE_SET_EXTERNAL_TIMER`
    SetExternalTimer,
    /// `CEC_OPCODE_SET_TIMER_PROGRAM_TITLE`
    SetTimerProgramTitle,
    /// `CEC_OPCODE_TIMER_CLEARED_STATUS`
    TimerClearedStatus,
    /// `CEC_OPCODE_TIMER_STATUS`
    TimerStatus,
    /// `CEC_OPCODE_CEC_VERSION`
    CecVersion,
    /// `CEC_OPCODE_GET_CEC_VERSION`
    GetCecVersion,
    /// `CEC_OPCODE_GIVE_PHYSICAL_ADDRESS`
    GivePhysicalAddress,
    /// `CEC_OPCODE_GET_MENU_LANGUAGE`
    GetMenuLanguage,
    /// `CEC_OPCODE_REPORT_PHYSICAL_ADDRESS`
    ReportPhysicalAddress,
    /// `CEC_OPCODE_SET_MENU_LANGUAGE`
    SetMenuLanguage,
    /// `CEC_OPCODE_DECK_CONTROL`
    DeckControl,
    /// `CEC_OPCODE_DECK_STATUS`
    DeckStatus,
    /// `CEC_OPCODE_GIVE_DECK_STATUS`
    GiveDeckStatus,
    /// `CEC_OPCODE_PLAY`
    Play,
    /// `CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS`
    GiveTunerDeviceStatus,
    /// `CEC_OPCODE_SELECT_ANALOGUE_SERVICE`
    SelectAnalogueService,
    /// `CEC_OPCODE_SELECT_DIGITAL_SERVICE`
    SelectDigitalService,
    /// `CEC_OPCODE_TUNER_DEVICE_STATUS`
    TunerDeviceStatus,
    /// `CEC_OPCODE_TUNER_STEP_DECREMENT`
    TunerStepDecrement,
    /// `CEC_OPCODE_TUNER_STEP_INCREMENT`
    TunerStepIncrement,
    /// `CEC_OPCODE_DEVICE_VENDOR_ID`
    DeviceVendorId,
    /// `CEC_OPCODE_GIVE_DEVICE_VENDOR_ID`
    GiveDeviceVendorId,
    /// `CEC_OPCODE_VENDOR_COMMAND`
    VendorCommand,
    /// `CEC_OPCODE_VENDOR_COMMAND_WITH_ID`
    VendorCommandWithId,
    /// `CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN`
    VendorRemoteButtonDown,
    /// `CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP`
    VendorRemoteButtonUp,
    /// `CEC_OPCODE_SET_OSD_STRING`
    SetOsdString,
    /// `CEC_OPCODE_GIVE_OSD_NAME`
    GiveOsdName,
    /// `CEC_OPCODE_SET_OSD_NAME`
    SetOsdName,
    /// `CEC_OPCODE_MENU_REQUEST`
    MenuRequest,
    /// `CEC_OPCODE_MENU_STATUS`
    MenuStatus,
    /// `CEC_OPCODE_USER_CONTROL_PRESSED`
    UserControlPressed,
    /// `CEC_OPCODE_USER_CONTROL_RELEASE`
    UserControlRelease,
    /// `CEC_OPCODE_GIVE_DEVICE_POWER_STATUS`
    GiveDevicePowerStatus,
    /// `CEC_OPCODE_REPORT_POWER_STATUS`
    ReportPowerStatus,
    /// `CEC_OPCODE_FEATURE_ABORT`
    FeatureAbort,
    /// `CEC_OPCODE_ABORT`
    Abort,
    /// `CEC_OPCODE_GIVE_AUDIO_STATUS`
    GiveAudioStatus,
    /// `CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS`
    GiveSystemAudioModeStatus,
    /// `CEC_OPCODE_REPORT_AUDIO_STATUS`
    ReportAudioStatus,
    /// `CEC_OPCODE_SET_SYSTEM_AUDIO_MODE`
    SetSystemAudioMode,
    /// `CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST`
    SystemAudioModeRequest,
    /// `CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS`
    SystemAudioModeStatus,
    /// `CEC_OPCODE_SET_AUDIO_RATE`
    SetAudioRate,
    /// `CEC_OPCODE_REPORT_SHORT_AUDIO_DESCRIPTORS`
    ReportShortAudioDescriptors,
    /// `CEC_OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTORS`
    RequestShortAudioDescriptors,
    /// `CEC_OPCODE_START_ARC`
    StartArc,
    /// `CEC_OPCODE_REPORT_ARC_STARTED`
    ReportArcStarted,
    /// `CEC_OPCODE_REPORT_ARC_ENDED`
    ReportArcEnded,
    /// `CEC_OPCODE_REQUEST_ARC_START`
    RequestArcStart,
    /// `CEC_OPCODE_REQUEST_ARC_END`
    RequestArcEnd,
    /// `CEC_OPCODE_END_ARC`
    EndArc,
    /// `CEC_OPCODE_CDC`
    Cdc,
    /// `CEC_OPCODE_NONE`
    None,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl Opcode {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            Opcode::ActiveSource => 130,
            Opcode::ImageViewOn => 4,
            Opcode::TextViewOn => 13,
            Opcode::InactiveSource => 157,
            Opcode::RequestActiveSource => 133,
            Opcode::RoutingChange => 128,
            Opcode::RoutingInformation => 129,
            Opcode::SetStreamPath => 134,
            Opcode::Standby => 54,
            Opcode::RecordOff => 11,
            Opcode::RecordOn => 9,
            Opcode::RecordStatus => 10,
            Opcode::RecordTvScreen => 15,
            Opcode::ClearAnalogueTimer => 51,
            Opcode::ClearDigitalTimer => 153,
            Opcode::ClearExternalTimer => 161,
            Opcode::SetAnalogueTimer => 52,
            Opcode::SetDigitalTimer => 151,
            Opcode::SetExternalTimer => 162,
            Opcode::SetTimerProgramTitle => 103,
            Opcode::TimerClearedStatus => 67,
            Opcode::TimerStatus => 53,
            Opcode::CecVersion => 158,
            Opcode::GetCecVersion => 159,
            Opcode::GivePhysicalAddress => 131,
            Opcode::GetMenuLanguage => 145,
            Opcode::ReportPhysicalAddress => 132,
            Opcode::SetMenuLanguage => 50,
            Opcode::DeckControl => 66,
            Opcode::DeckStatus => 27,
            Opcode::GiveDeckStatus => 26,
            Opcode::Play => 65,
            Opcode::GiveTunerDeviceStatus => 8,
            Opcode::SelectAnalogueService => 146,
            Opcode::SelectDigitalService => 147,
            Opcode::TunerDeviceStatus => 7,
            Opcode::TunerStepDecrement => 6,
            Opcode::TunerStepIncrement => 5,
            Opcode::DeviceVendorId => 135,
            Opcode::GiveDeviceVendorId => 140,
            Opcode::VendorCommand => 137,
            Opcode::VendorCommandWithId => 160,
            Opcode::VendorRemoteButtonDown => 138,
            Opcode::VendorRemoteButtonUp => 139,
            Opcode::SetOsdString => 100,
            Opcode::GiveOsdName => 70,
            Opcode::SetOsdName => 71,
            Opcode::MenuRequest => 141,
            Opcode::MenuStatus => 142,
            Opcode::UserControlPressed => 68,
            Opcode::UserControlRelease => 69,
            Opcode::GiveDevicePowerStatus => 143,
            Opcode::ReportPowerStatus => 144,
            Opcode::FeatureAbort => 0,
            Opcode::Abort => 255,
            Opcode::GiveAudioStatus => 113,
            Opcode::GiveSystemAudioModeStatus => 125,
            Opcode::ReportAudioStatus => 122,
            Opcode::SetSystemAudioMode => 114,
            Opcode::SystemAudioModeRequest => 112,
            Opcode::SystemAudioModeStatus => 126,
            Opcode::SetAudioRate => 154,
            Opcode::ReportShortAudioDescriptors => 163,
            Opcode::RequestShortAudioDescriptors => 164,
            Opcode::StartArc => 192,
            Opcode::ReportArcStarted => 193,
            Opcode::ReportArcEnded => 194,
            Opcode::RequestArcStart => 195,
            Opcode::RequestArcEnd => 196,
            Opcode::EndArc => 197,
            Opcode::Cdc => 248,
            Opcode::None => 253,
            Opcode::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`Opcode::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            130 => Opcode::ActiveSource,
            4 => Opcode::ImageViewOn,
            13 => Opcode::TextViewOn,
            157 => Opcode::InactiveSource,
            133 => Opcode::RequestActiveSource,
            128 => Opcode::RoutingChange,
            129 => Opcode::RoutingInformation,
            134 => Opcode::SetStreamPath,
            54 => Opcode::Standby,
            11 => Opcode::RecordOff,
            9 => Opcode::RecordOn,
            10 => Opcode::RecordStatus,
            15 => Opcode::RecordTvScreen,
            51 => Opcode::ClearAnalogueTimer,
            153 => Opcode::ClearDigitalTimer,
            161 => Opcode::ClearExternalTimer,
            52 => Opcode::SetAnalogueTimer,
            151 => Opcode::SetDigitalTimer,
            162 => Opcode::SetExternalTimer,
            103 => Opcode::SetTimerProgramTitle,
            67 => Opcode::TimerClearedStatus,
            53 => Opcode::TimerStatus,
            158 => Opcode::CecVersion,
            159 => Opcode::GetCecVersion,
            131 => Opcode::GivePhysicalAddress,
            145 => Opcode::GetMenuLanguage,
            132 => Opcode::ReportPhysicalAddress,
            50 => Opcode::SetMenuLanguage,
            66 => Opcode::DeckControl,
            27 => Opcode::DeckStatus,
            26 => Opcode::GiveDeckStatus,
            65 => Opcode::Play,
            8 => Opcode::GiveTunerDeviceStatus,
            146 => Opcode::SelectAnalogueService,
            147 => Opcode::SelectDigitalService,
            7 => Opcode::TunerDeviceStatus,
            6 => Opcode::TunerStepDecrement,
            5 => Opcode::TunerStepIncrement,
            135 => Opcode::DeviceVendorId,
            140 => Opcode::GiveDeviceVendorId,
            137 => Opcode::VendorCommand,
            160 => Opcode::VendorCommandWithId,
            138 => Opcode::VendorRemoteButtonDown,
            139 => Opcode::VendorRemoteButtonUp,
            100 => Opcode::SetOsdString,
            70 => Opcode::GiveOsdName,
            71 => Opcode::SetOsdName,
            141 => Opcode::MenuRequest,
            142 => Opcode::MenuStatus,
            68 => Opcode::UserControlPressed,
            69 => Opcode::UserControlRelease,
            143 => Opcode::GiveDevicePowerStatus,
            144 => Opcode::ReportPowerStatus,
            0 => Opcode::FeatureAbort,
            255 => Opcode::Abort,
            113 => Opcode::GiveAudioStatus,
            125 => Opcode::GiveSystemAudioModeStatus,
            122 => Opcode::ReportAudioStatus,
            114 => Opcode::SetSystemAudioMode,
            112 => Opcode::SystemAudioModeRequest,
            126 => Opcode::SystemAudioModeStatus,
            154 => Opcode::SetAudioRate,
            163 => Opcode::ReportShortAudioDescriptors,
            164 => Opcode::RequestShortAudioDescriptors,
            192 => Opcode::StartArc,
            193 => Opcode::ReportArcStarted,
            194 => Opcode::ReportArcEnded,
            195 => Opcode::RequestArcStart,
            196 => Opcode::RequestArcEnd,
            197 => Opcode::EndArc,
            248 => Opcode::Cdc,
            253 => Opcode::None,
            other => Opcode::Other(other),
        }
    }
}

impl From<i32> for Opcode {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<Opcode> for i32 {
    fn from(value: Opcode) -> Self {
        value.raw()
    }
}

impl fmt::Display for Opcode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_opcode_to_string, self.raw()))
    }
}

/// How a device should play: direction and speed.
///
/// Mirrors the C `cec_play_mode`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum PlayMode {
    /// `CEC_PLAY_MODE_PLAY_FORWARD`
    PlayForward,
    /// `CEC_PLAY_MODE_PLAY_REVERSE`
    PlayReverse,
    /// `CEC_PLAY_MODE_PLAY_STILL`
    PlayStill,
    /// `CEC_PLAY_MODE_FAST_FORWARD_MIN_SPEED`
    FastForwardMinSpeed,
    /// `CEC_PLAY_MODE_FAST_FORWARD_MEDIUM_SPEED`
    FastForwardMediumSpeed,
    /// `CEC_PLAY_MODE_FAST_FORWARD_MAX_SPEED`
    FastForwardMaxSpeed,
    /// `CEC_PLAY_MODE_FAST_REVERSE_MIN_SPEED`
    FastReverseMinSpeed,
    /// `CEC_PLAY_MODE_FAST_REVERSE_MEDIUM_SPEED`
    FastReverseMediumSpeed,
    /// `CEC_PLAY_MODE_FAST_REVERSE_MAX_SPEED`
    FastReverseMaxSpeed,
    /// `CEC_PLAY_MODE_SLOW_FORWARD_MIN_SPEED`
    SlowForwardMinSpeed,
    /// `CEC_PLAY_MODE_SLOW_FORWARD_MEDIUM_SPEED`
    SlowForwardMediumSpeed,
    /// `CEC_PLAY_MODE_SLOW_FORWARD_MAX_SPEED`
    SlowForwardMaxSpeed,
    /// `CEC_PLAY_MODE_SLOW_REVERSE_MIN_SPEED`
    SlowReverseMinSpeed,
    /// `CEC_PLAY_MODE_SLOW_REVERSE_MEDIUM_SPEED`
    SlowReverseMediumSpeed,
    /// `CEC_PLAY_MODE_SLOW_REVERSE_MAX_SPEED`
    SlowReverseMaxSpeed,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl PlayMode {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            PlayMode::PlayForward => 36,
            PlayMode::PlayReverse => 32,
            PlayMode::PlayStill => 37,
            PlayMode::FastForwardMinSpeed => 5,
            PlayMode::FastForwardMediumSpeed => 6,
            PlayMode::FastForwardMaxSpeed => 7,
            PlayMode::FastReverseMinSpeed => 9,
            PlayMode::FastReverseMediumSpeed => 10,
            PlayMode::FastReverseMaxSpeed => 11,
            PlayMode::SlowForwardMinSpeed => 21,
            PlayMode::SlowForwardMediumSpeed => 22,
            PlayMode::SlowForwardMaxSpeed => 23,
            PlayMode::SlowReverseMinSpeed => 25,
            PlayMode::SlowReverseMediumSpeed => 26,
            PlayMode::SlowReverseMaxSpeed => 27,
            PlayMode::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`PlayMode::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            36 => PlayMode::PlayForward,
            32 => PlayMode::PlayReverse,
            37 => PlayMode::PlayStill,
            5 => PlayMode::FastForwardMinSpeed,
            6 => PlayMode::FastForwardMediumSpeed,
            7 => PlayMode::FastForwardMaxSpeed,
            9 => PlayMode::FastReverseMinSpeed,
            10 => PlayMode::FastReverseMediumSpeed,
            11 => PlayMode::FastReverseMaxSpeed,
            21 => PlayMode::SlowForwardMinSpeed,
            22 => PlayMode::SlowForwardMediumSpeed,
            23 => PlayMode::SlowForwardMaxSpeed,
            25 => PlayMode::SlowReverseMinSpeed,
            26 => PlayMode::SlowReverseMediumSpeed,
            27 => PlayMode::SlowReverseMaxSpeed,
            other => PlayMode::Other(other),
        }
    }
}

impl From<i32> for PlayMode {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<PlayMode> for i32 {
    fn from(value: PlayMode) -> Self {
        value.raw()
    }
}

/// The power state a device reports.
///
/// Mirrors the C `cec_power_status`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum PowerStatus {
    /// `CEC_POWER_STATUS_ON`
    On,
    /// `CEC_POWER_STATUS_STANDBY`
    Standby,
    /// `CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON`
    InTransitionStandbyToOn,
    /// `CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY`
    InTransitionOnToStandby,
    /// `CEC_POWER_STATUS_UNKNOWN`
    Unknown,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl PowerStatus {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            PowerStatus::On => 0,
            PowerStatus::Standby => 1,
            PowerStatus::InTransitionStandbyToOn => 2,
            PowerStatus::InTransitionOnToStandby => 3,
            PowerStatus::Unknown => 153,
            PowerStatus::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`PowerStatus::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => PowerStatus::On,
            1 => PowerStatus::Standby,
            2 => PowerStatus::InTransitionStandbyToOn,
            3 => PowerStatus::InTransitionOnToStandby,
            153 => PowerStatus::Unknown,
            other => PowerStatus::Other(other),
        }
    }
}

impl From<i32> for PowerStatus {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<PowerStatus> for i32 {
    fn from(value: PowerStatus) -> Self {
        value.raw()
    }
}

impl fmt::Display for PowerStatus {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_power_status_to_string, self.raw()))
    }
}

/// Whether system audio mode is engaged.
///
/// Mirrors the C `cec_system_audio_status`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum SystemAudioStatus {
    /// `CEC_SYSTEM_AUDIO_STATUS_OFF`
    Off,
    /// `CEC_SYSTEM_AUDIO_STATUS_ON`
    On,
    /// `CEC_SYSTEM_AUDIO_STATUS_UNKNOWN`
    Unknown,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl SystemAudioStatus {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            SystemAudioStatus::Off => 0,
            SystemAudioStatus::On => 1,
            SystemAudioStatus::Unknown => 2,
            SystemAudioStatus::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`SystemAudioStatus::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => SystemAudioStatus::Off,
            1 => SystemAudioStatus::On,
            2 => SystemAudioStatus::Unknown,
            other => SystemAudioStatus::Other(other),
        }
    }
}

impl From<i32> for SystemAudioStatus {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<SystemAudioStatus> for i32 {
    fn from(value: SystemAudioStatus) -> Self {
        value.raw()
    }
}

impl fmt::Display for SystemAudioStatus {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(
            ffi::libcec_system_audio_status_to_string,
            self.raw(),
        ))
    }
}

/// A remote control key.
///
/// Mirrors the C `cec_user_control_code`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum UserControlCode {
    /// `CEC_USER_CONTROL_CODE_SELECT`
    Select,
    /// `CEC_USER_CONTROL_CODE_UP`
    Up,
    /// `CEC_USER_CONTROL_CODE_DOWN`
    Down,
    /// `CEC_USER_CONTROL_CODE_LEFT`
    Left,
    /// `CEC_USER_CONTROL_CODE_RIGHT`
    Right,
    /// `CEC_USER_CONTROL_CODE_RIGHT_UP`
    RightUp,
    /// `CEC_USER_CONTROL_CODE_RIGHT_DOWN`
    RightDown,
    /// `CEC_USER_CONTROL_CODE_LEFT_UP`
    LeftUp,
    /// `CEC_USER_CONTROL_CODE_LEFT_DOWN`
    LeftDown,
    /// `CEC_USER_CONTROL_CODE_ROOT_MENU`
    RootMenu,
    /// `CEC_USER_CONTROL_CODE_SETUP_MENU`
    SetupMenu,
    /// `CEC_USER_CONTROL_CODE_CONTENTS_MENU`
    ContentsMenu,
    /// `CEC_USER_CONTROL_CODE_FAVORITE_MENU`
    FavoriteMenu,
    /// `CEC_USER_CONTROL_CODE_EXIT`
    Exit,
    /// `CEC_USER_CONTROL_CODE_TOP_MENU`
    TopMenu,
    /// `CEC_USER_CONTROL_CODE_DVD_MENU`
    DvdMenu,
    /// `CEC_USER_CONTROL_CODE_NUMBER_ENTRY_MODE`
    NumberEntryMode,
    /// `CEC_USER_CONTROL_CODE_NUMBER11`
    Number11,
    /// `CEC_USER_CONTROL_CODE_NUMBER12`
    Number12,
    /// `CEC_USER_CONTROL_CODE_NUMBER0`
    Number0,
    /// `CEC_USER_CONTROL_CODE_NUMBER1`
    Number1,
    /// `CEC_USER_CONTROL_CODE_NUMBER2`
    Number2,
    /// `CEC_USER_CONTROL_CODE_NUMBER3`
    Number3,
    /// `CEC_USER_CONTROL_CODE_NUMBER4`
    Number4,
    /// `CEC_USER_CONTROL_CODE_NUMBER5`
    Number5,
    /// `CEC_USER_CONTROL_CODE_NUMBER6`
    Number6,
    /// `CEC_USER_CONTROL_CODE_NUMBER7`
    Number7,
    /// `CEC_USER_CONTROL_CODE_NUMBER8`
    Number8,
    /// `CEC_USER_CONTROL_CODE_NUMBER9`
    Number9,
    /// `CEC_USER_CONTROL_CODE_DOT`
    Dot,
    /// `CEC_USER_CONTROL_CODE_ENTER`
    Enter,
    /// `CEC_USER_CONTROL_CODE_CLEAR`
    Clear,
    /// `CEC_USER_CONTROL_CODE_NEXT_FAVORITE`
    NextFavorite,
    /// `CEC_USER_CONTROL_CODE_CHANNEL_UP`
    ChannelUp,
    /// `CEC_USER_CONTROL_CODE_CHANNEL_DOWN`
    ChannelDown,
    /// `CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL`
    PreviousChannel,
    /// `CEC_USER_CONTROL_CODE_SOUND_SELECT`
    SoundSelect,
    /// `CEC_USER_CONTROL_CODE_INPUT_SELECT`
    InputSelect,
    /// `CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION`
    DisplayInformation,
    /// `CEC_USER_CONTROL_CODE_HELP`
    Help,
    /// `CEC_USER_CONTROL_CODE_PAGE_UP`
    PageUp,
    /// `CEC_USER_CONTROL_CODE_PAGE_DOWN`
    PageDown,
    /// `CEC_USER_CONTROL_CODE_POWER`
    Power,
    /// `CEC_USER_CONTROL_CODE_VOLUME_UP`
    VolumeUp,
    /// `CEC_USER_CONTROL_CODE_VOLUME_DOWN`
    VolumeDown,
    /// `CEC_USER_CONTROL_CODE_MUTE`
    Mute,
    /// `CEC_USER_CONTROL_CODE_PLAY`
    Play,
    /// `CEC_USER_CONTROL_CODE_STOP`
    Stop,
    /// `CEC_USER_CONTROL_CODE_PAUSE`
    Pause,
    /// `CEC_USER_CONTROL_CODE_RECORD`
    Record,
    /// `CEC_USER_CONTROL_CODE_REWIND`
    Rewind,
    /// `CEC_USER_CONTROL_CODE_FAST_FORWARD`
    FastForward,
    /// `CEC_USER_CONTROL_CODE_EJECT`
    Eject,
    /// `CEC_USER_CONTROL_CODE_FORWARD`
    Forward,
    /// `CEC_USER_CONTROL_CODE_BACKWARD`
    Backward,
    /// `CEC_USER_CONTROL_CODE_STOP_RECORD`
    StopRecord,
    /// `CEC_USER_CONTROL_CODE_PAUSE_RECORD`
    PauseRecord,
    /// `CEC_USER_CONTROL_CODE_ANGLE`
    Angle,
    /// `CEC_USER_CONTROL_CODE_SUB_PICTURE`
    SubPicture,
    /// `CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND`
    VideoOnDemand,
    /// `CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE`
    ElectronicProgramGuide,
    /// `CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING`
    TimerProgramming,
    /// `CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION`
    InitialConfiguration,
    /// `CEC_USER_CONTROL_CODE_SELECT_BROADCAST_TYPE`
    SelectBroadcastType,
    /// `CEC_USER_CONTROL_CODE_SELECT_SOUND_PRESENTATION`
    SelectSoundPresentation,
    /// `CEC_USER_CONTROL_CODE_PLAY_FUNCTION`
    PlayFunction,
    /// `CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION`
    PausePlayFunction,
    /// `CEC_USER_CONTROL_CODE_RECORD_FUNCTION`
    RecordFunction,
    /// `CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION`
    PauseRecordFunction,
    /// `CEC_USER_CONTROL_CODE_STOP_FUNCTION`
    StopFunction,
    /// `CEC_USER_CONTROL_CODE_MUTE_FUNCTION`
    MuteFunction,
    /// `CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION`
    RestoreVolumeFunction,
    /// `CEC_USER_CONTROL_CODE_TUNE_FUNCTION`
    TuneFunction,
    /// `CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION`
    SelectMediaFunction,
    /// `CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION`
    SelectAvInputFunction,
    /// `CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION`
    SelectAudioInputFunction,
    /// `CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION`
    PowerToggleFunction,
    /// `CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION`
    PowerOffFunction,
    /// `CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION`
    PowerOnFunction,
    /// `CEC_USER_CONTROL_CODE_F1_BLUE`
    F1Blue,
    /// `CEC_USER_CONTROL_CODE_F2_RED`
    F2Red,
    /// `CEC_USER_CONTROL_CODE_F3_GREEN`
    F3Green,
    /// `CEC_USER_CONTROL_CODE_F4_YELLOW`
    F4Yellow,
    /// `CEC_USER_CONTROL_CODE_F5`
    F5,
    /// `CEC_USER_CONTROL_CODE_DATA`
    Data,
    /// `CEC_USER_CONTROL_CODE_AN_RETURN`
    AnReturn,
    /// `CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST`
    AnChannelsList,
    /// `CEC_USER_CONTROL_CODE_UNKNOWN`
    Unknown,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl UserControlCode {
    /// `CEC_USER_CONTROL_CODE_MAX` - the same value as [`UserControlCode::AnChannelsList`] (150).
    pub const MAX: UserControlCode = UserControlCode::AnChannelsList;
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            UserControlCode::Select => 0,
            UserControlCode::Up => 1,
            UserControlCode::Down => 2,
            UserControlCode::Left => 3,
            UserControlCode::Right => 4,
            UserControlCode::RightUp => 5,
            UserControlCode::RightDown => 6,
            UserControlCode::LeftUp => 7,
            UserControlCode::LeftDown => 8,
            UserControlCode::RootMenu => 9,
            UserControlCode::SetupMenu => 10,
            UserControlCode::ContentsMenu => 11,
            UserControlCode::FavoriteMenu => 12,
            UserControlCode::Exit => 13,
            UserControlCode::TopMenu => 16,
            UserControlCode::DvdMenu => 17,
            UserControlCode::NumberEntryMode => 29,
            UserControlCode::Number11 => 30,
            UserControlCode::Number12 => 31,
            UserControlCode::Number0 => 32,
            UserControlCode::Number1 => 33,
            UserControlCode::Number2 => 34,
            UserControlCode::Number3 => 35,
            UserControlCode::Number4 => 36,
            UserControlCode::Number5 => 37,
            UserControlCode::Number6 => 38,
            UserControlCode::Number7 => 39,
            UserControlCode::Number8 => 40,
            UserControlCode::Number9 => 41,
            UserControlCode::Dot => 42,
            UserControlCode::Enter => 43,
            UserControlCode::Clear => 44,
            UserControlCode::NextFavorite => 47,
            UserControlCode::ChannelUp => 48,
            UserControlCode::ChannelDown => 49,
            UserControlCode::PreviousChannel => 50,
            UserControlCode::SoundSelect => 51,
            UserControlCode::InputSelect => 52,
            UserControlCode::DisplayInformation => 53,
            UserControlCode::Help => 54,
            UserControlCode::PageUp => 55,
            UserControlCode::PageDown => 56,
            UserControlCode::Power => 64,
            UserControlCode::VolumeUp => 65,
            UserControlCode::VolumeDown => 66,
            UserControlCode::Mute => 67,
            UserControlCode::Play => 68,
            UserControlCode::Stop => 69,
            UserControlCode::Pause => 70,
            UserControlCode::Record => 71,
            UserControlCode::Rewind => 72,
            UserControlCode::FastForward => 73,
            UserControlCode::Eject => 74,
            UserControlCode::Forward => 75,
            UserControlCode::Backward => 76,
            UserControlCode::StopRecord => 77,
            UserControlCode::PauseRecord => 78,
            UserControlCode::Angle => 80,
            UserControlCode::SubPicture => 81,
            UserControlCode::VideoOnDemand => 82,
            UserControlCode::ElectronicProgramGuide => 83,
            UserControlCode::TimerProgramming => 84,
            UserControlCode::InitialConfiguration => 85,
            UserControlCode::SelectBroadcastType => 86,
            UserControlCode::SelectSoundPresentation => 87,
            UserControlCode::PlayFunction => 96,
            UserControlCode::PausePlayFunction => 97,
            UserControlCode::RecordFunction => 98,
            UserControlCode::PauseRecordFunction => 99,
            UserControlCode::StopFunction => 100,
            UserControlCode::MuteFunction => 101,
            UserControlCode::RestoreVolumeFunction => 102,
            UserControlCode::TuneFunction => 103,
            UserControlCode::SelectMediaFunction => 104,
            UserControlCode::SelectAvInputFunction => 105,
            UserControlCode::SelectAudioInputFunction => 106,
            UserControlCode::PowerToggleFunction => 107,
            UserControlCode::PowerOffFunction => 108,
            UserControlCode::PowerOnFunction => 109,
            UserControlCode::F1Blue => 113,
            UserControlCode::F2Red => 114,
            UserControlCode::F3Green => 115,
            UserControlCode::F4Yellow => 116,
            UserControlCode::F5 => 117,
            UserControlCode::Data => 118,
            UserControlCode::AnReturn => 145,
            UserControlCode::AnChannelsList => 150,
            UserControlCode::Unknown => 255,
            UserControlCode::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`UserControlCode::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => UserControlCode::Select,
            1 => UserControlCode::Up,
            2 => UserControlCode::Down,
            3 => UserControlCode::Left,
            4 => UserControlCode::Right,
            5 => UserControlCode::RightUp,
            6 => UserControlCode::RightDown,
            7 => UserControlCode::LeftUp,
            8 => UserControlCode::LeftDown,
            9 => UserControlCode::RootMenu,
            10 => UserControlCode::SetupMenu,
            11 => UserControlCode::ContentsMenu,
            12 => UserControlCode::FavoriteMenu,
            13 => UserControlCode::Exit,
            16 => UserControlCode::TopMenu,
            17 => UserControlCode::DvdMenu,
            29 => UserControlCode::NumberEntryMode,
            30 => UserControlCode::Number11,
            31 => UserControlCode::Number12,
            32 => UserControlCode::Number0,
            33 => UserControlCode::Number1,
            34 => UserControlCode::Number2,
            35 => UserControlCode::Number3,
            36 => UserControlCode::Number4,
            37 => UserControlCode::Number5,
            38 => UserControlCode::Number6,
            39 => UserControlCode::Number7,
            40 => UserControlCode::Number8,
            41 => UserControlCode::Number9,
            42 => UserControlCode::Dot,
            43 => UserControlCode::Enter,
            44 => UserControlCode::Clear,
            47 => UserControlCode::NextFavorite,
            48 => UserControlCode::ChannelUp,
            49 => UserControlCode::ChannelDown,
            50 => UserControlCode::PreviousChannel,
            51 => UserControlCode::SoundSelect,
            52 => UserControlCode::InputSelect,
            53 => UserControlCode::DisplayInformation,
            54 => UserControlCode::Help,
            55 => UserControlCode::PageUp,
            56 => UserControlCode::PageDown,
            64 => UserControlCode::Power,
            65 => UserControlCode::VolumeUp,
            66 => UserControlCode::VolumeDown,
            67 => UserControlCode::Mute,
            68 => UserControlCode::Play,
            69 => UserControlCode::Stop,
            70 => UserControlCode::Pause,
            71 => UserControlCode::Record,
            72 => UserControlCode::Rewind,
            73 => UserControlCode::FastForward,
            74 => UserControlCode::Eject,
            75 => UserControlCode::Forward,
            76 => UserControlCode::Backward,
            77 => UserControlCode::StopRecord,
            78 => UserControlCode::PauseRecord,
            80 => UserControlCode::Angle,
            81 => UserControlCode::SubPicture,
            82 => UserControlCode::VideoOnDemand,
            83 => UserControlCode::ElectronicProgramGuide,
            84 => UserControlCode::TimerProgramming,
            85 => UserControlCode::InitialConfiguration,
            86 => UserControlCode::SelectBroadcastType,
            87 => UserControlCode::SelectSoundPresentation,
            96 => UserControlCode::PlayFunction,
            97 => UserControlCode::PausePlayFunction,
            98 => UserControlCode::RecordFunction,
            99 => UserControlCode::PauseRecordFunction,
            100 => UserControlCode::StopFunction,
            101 => UserControlCode::MuteFunction,
            102 => UserControlCode::RestoreVolumeFunction,
            103 => UserControlCode::TuneFunction,
            104 => UserControlCode::SelectMediaFunction,
            105 => UserControlCode::SelectAvInputFunction,
            106 => UserControlCode::SelectAudioInputFunction,
            107 => UserControlCode::PowerToggleFunction,
            108 => UserControlCode::PowerOffFunction,
            109 => UserControlCode::PowerOnFunction,
            113 => UserControlCode::F1Blue,
            114 => UserControlCode::F2Red,
            115 => UserControlCode::F3Green,
            116 => UserControlCode::F4Yellow,
            117 => UserControlCode::F5,
            118 => UserControlCode::Data,
            145 => UserControlCode::AnReturn,
            150 => UserControlCode::AnChannelsList,
            255 => UserControlCode::Unknown,
            other => UserControlCode::Other(other),
        }
    }
}

impl From<i32> for UserControlCode {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<UserControlCode> for i32 {
    fn from(value: UserControlCode) -> Self {
        value.raw()
    }
}

impl fmt::Display for UserControlCode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(
            ffi::libcec_user_control_key_to_string,
            self.raw(),
        ))
    }
}

/// A CEC vendor id, as reported by a device.
///
/// Mirrors the C `cec_vendor_id`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum VendorId {
    /// `CEC_VENDOR_TOSHIBA`
    Toshiba,
    /// `CEC_VENDOR_SAMSUNG`
    Samsung,
    /// `CEC_VENDOR_DENON`
    Denon,
    /// `CEC_VENDOR_MARANTZ`
    Marantz,
    /// `CEC_VENDOR_LOEWE`
    Loewe,
    /// `CEC_VENDOR_ONKYO`
    Onkyo,
    /// `CEC_VENDOR_MEDION`
    Medion,
    /// `CEC_VENDOR_TOSHIBA2`
    Toshiba2,
    /// `CEC_VENDOR_APPLE`
    Apple,
    /// `CEC_VENDOR_PULSE_EIGHT`
    PulseEight,
    /// `CEC_VENDOR_HARMAN_KARDON2`
    HarmanKardon2,
    /// `CEC_VENDOR_GOOGLE`
    Google,
    /// `CEC_VENDOR_AKAI`
    Akai,
    /// `CEC_VENDOR_AOC`
    Aoc,
    /// `CEC_VENDOR_PANASONIC`
    Panasonic,
    /// `CEC_VENDOR_PHILIPS`
    Philips,
    /// `CEC_VENDOR_DAEWOO`
    Daewoo,
    /// `CEC_VENDOR_YAMAHA`
    Yamaha,
    /// `CEC_VENDOR_GRUNDIG`
    Grundig,
    /// `CEC_VENDOR_PIONEER`
    Pioneer,
    /// `CEC_VENDOR_LG`
    Lg,
    /// `CEC_VENDOR_SHARP`
    Sharp,
    /// `CEC_VENDOR_SONY`
    Sony,
    /// `CEC_VENDOR_TEUFEL`
    Teufel,
    /// `CEC_VENDOR_BROADCOM`
    Broadcom,
    /// `CEC_VENDOR_SHARP2`
    Sharp2,
    /// `CEC_VENDOR_VIZIO`
    Vizio,
    /// `CEC_VENDOR_BENQ`
    Benq,
    /// `CEC_VENDOR_HARMAN_KARDON`
    HarmanKardon,
    /// `CEC_VENDOR_UNKNOWN`
    Unknown,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl VendorId {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            VendorId::Toshiba => 57,
            VendorId::Samsung => 240,
            VendorId::Denon => 1485,
            VendorId::Marantz => 1656,
            VendorId::Loewe => 2434,
            VendorId::Onkyo => 2480,
            VendorId::Medion => 3256,
            VendorId::Toshiba2 => 3303,
            VendorId::Apple => 4346,
            VendorId::PulseEight => 5506,
            VendorId::HarmanKardon2 => 6480,
            VendorId::Google => 6673,
            VendorId::Akai => 8391,
            VendorId::Aoc => 9319,
            VendorId::Panasonic => 32837,
            VendorId::Philips => 36926,
            VendorId::Daewoo => 36947,
            VendorId::Yamaha => 41182,
            VendorId::Grundig => 53461,
            VendorId::Pioneer => 57398,
            VendorId::Lg => 57489,
            VendorId::Sharp => 524319,
            VendorId::Sony => 524358,
            VendorId::Teufel => 2303013,
            VendorId::Broadcom => 1622150,
            VendorId::Sharp2 => 5458000,
            VendorId::Vizio => 7042157,
            VendorId::Benq => 8414697,
            VendorId::HarmanKardon => 10249310,
            VendorId::Unknown => 0,
            VendorId::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`VendorId::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            57 => VendorId::Toshiba,
            240 => VendorId::Samsung,
            1485 => VendorId::Denon,
            1656 => VendorId::Marantz,
            2434 => VendorId::Loewe,
            2480 => VendorId::Onkyo,
            3256 => VendorId::Medion,
            3303 => VendorId::Toshiba2,
            4346 => VendorId::Apple,
            5506 => VendorId::PulseEight,
            6480 => VendorId::HarmanKardon2,
            6673 => VendorId::Google,
            8391 => VendorId::Akai,
            9319 => VendorId::Aoc,
            32837 => VendorId::Panasonic,
            36926 => VendorId::Philips,
            36947 => VendorId::Daewoo,
            41182 => VendorId::Yamaha,
            53461 => VendorId::Grundig,
            57398 => VendorId::Pioneer,
            57489 => VendorId::Lg,
            524319 => VendorId::Sharp,
            524358 => VendorId::Sony,
            2303013 => VendorId::Teufel,
            1622150 => VendorId::Broadcom,
            5458000 => VendorId::Sharp2,
            7042157 => VendorId::Vizio,
            8414697 => VendorId::Benq,
            10249310 => VendorId::HarmanKardon,
            0 => VendorId::Unknown,
            other => VendorId::Other(other),
        }
    }
}

impl From<i32> for VendorId {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<VendorId> for i32 {
    fn from(value: VendorId) -> Self {
        value.raw()
    }
}

impl fmt::Display for VendorId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_vendor_id_to_string, self.raw()))
    }
}

/// The CEC specification version a device speaks.
///
/// Mirrors the C `cec_version`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum CecVersion {
    /// `CEC_VERSION_UNKNOWN`
    Unknown,
    /// `CEC_VERSION_1_2`
    V1_2,
    /// `CEC_VERSION_1_2A`
    V1_2a,
    /// `CEC_VERSION_1_3`
    V1_3,
    /// `CEC_VERSION_1_3A`
    V1_3a,
    /// `CEC_VERSION_1_4`
    V1_4,
    /// `CEC_VERSION_2_0`
    V2_0,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl CecVersion {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            CecVersion::Unknown => 0,
            CecVersion::V1_2 => 1,
            CecVersion::V1_2a => 2,
            CecVersion::V1_3 => 3,
            CecVersion::V1_3a => 4,
            CecVersion::V1_4 => 5,
            CecVersion::V2_0 => 6,
            CecVersion::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`CecVersion::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => CecVersion::Unknown,
            1 => CecVersion::V1_2,
            2 => CecVersion::V1_2a,
            3 => CecVersion::V1_3,
            4 => CecVersion::V1_3a,
            5 => CecVersion::V1_4,
            6 => CecVersion::V2_0,
            other => CecVersion::Other(other),
        }
    }
}

impl From<i32> for CecVersion {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<CecVersion> for i32 {
    fn from(value: CecVersion) -> Self {
        value.raw()
    }
}

impl fmt::Display for CecVersion {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&describe(ffi::libcec_cec_version_to_string, self.raw()))
    }
}

/// An out-of-band notification from libCEC.
///
/// Mirrors the C `libcec_alert`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum Alert {
    /// `CEC_ALERT_SERVICE_DEVICE`
    ServiceDevice,
    /// `CEC_ALERT_CONNECTION_LOST`
    ConnectionLost,
    /// `CEC_ALERT_PERMISSION_ERROR`
    PermissionError,
    /// `CEC_ALERT_PORT_BUSY`
    PortBusy,
    /// `CEC_ALERT_PHYSICAL_ADDRESS_ERROR`
    PhysicalAddressError,
    /// `CEC_ALERT_TV_POLL_FAILED`
    TvPollFailed,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl Alert {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            Alert::ServiceDevice => 0,
            Alert::ConnectionLost => 1,
            Alert::PermissionError => 2,
            Alert::PortBusy => 3,
            Alert::PhysicalAddressError => 4,
            Alert::TvPollFailed => 5,
            Alert::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`Alert::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => Alert::ServiceDevice,
            1 => Alert::ConnectionLost,
            2 => Alert::PermissionError,
            3 => Alert::PortBusy,
            4 => Alert::PhysicalAddressError,
            5 => Alert::TvPollFailed,
            other => Alert::Other(other),
        }
    }
}

impl From<i32> for Alert {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<Alert> for i32 {
    fn from(value: Alert) -> Self {
        value.raw()
    }
}

/// The type of the data attached to an [`Alert`].
///
/// Mirrors the C `libcec_parameter_type`.
#[derive(Copy, Clone, PartialEq, Eq, Hash, Debug)]
pub enum ParameterType {
    /// `CEC_PARAMETER_TYPE_STRING`
    String,
    /// `CEC_PARAMETER_TYPE_UNKOWN`
    Unkown,
    /// A value libCEC reported that this crate has no name for.
    ///
    /// The CEC bus carries whatever devices put on it, so this is
    /// data, not an error.
    Other(i32),
}

impl ParameterType {
    /// The value libCEC uses for this variant.
    pub fn raw(self) -> i32 {
        match self {
            ParameterType::String => 0,
            ParameterType::Unkown => 1,
            ParameterType::Other(value) => value,
        }
    }

    /// Read a value libCEC produced. Total: anything unrecognised
    /// becomes [`ParameterType::Other`].
    pub fn from_raw(value: i32) -> Self {
        match value {
            0 => ParameterType::String,
            1 => ParameterType::Unkown,
            other => ParameterType::Other(other),
        }
    }
}

impl From<i32> for ParameterType {
    fn from(value: i32) -> Self {
        Self::from_raw(value)
    }
}

impl From<ParameterType> for i32 {
    fn from(value: ParameterType) -> Self {
        value.raw()
    }
}
