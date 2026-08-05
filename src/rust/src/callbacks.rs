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

//! Receiving what libCEC has to say.
//!
//! libCEC calls back from its own worker thread, never from the thread that
//! opened the connection. There are two ways to take delivery, and which one
//! suits depends on what the callback needs to do:
//!
//! * **[`CecCallbacks`]** - implement the trait and libCEC calls your code
//!   directly, on its thread. Nothing is copied beyond the owned types, the
//!   latency is as low as it goes, and
//!   [`menu_state_changed`](CecCallbacks::menu_state_changed) and
//!   [`command_handler`](CecCallbacks::command_handler) can return `true` to
//!   take a decision away from libCEC. The cost is that you are running on
//!   libCEC's thread: block there and you block the CEC bus.
//!
//! * **[`channel`]** - events arrive as [`CecEvent`] values on an
//!   [`mpsc::Receiver`], and you handle them wherever you like. This is the one
//!   to reach for by default. It cannot answer the two deciding callbacks - a
//!   channel has nowhere to put the answer in time - so both keep libCEC's own
//!   behaviour.
//!
//! The Node.js binding has no such choice: JavaScript values can only be touched
//! on the event loop, so everything there goes the long way round.

use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::Mutex;

use crate::enums::{Alert, LogicalAddress, MenuState};
use crate::types::{Command, Configuration, Keypress, LogMessage};

/// Handlers for everything libCEC reports.
///
/// Every method has a default that does nothing, so implement only what you
/// care about. Methods take `&self` and the trait is [`Sync`]: libCEC may call
/// from more than one of its threads, and holding a lock across a callback is a
/// decision this crate should not make for you. Use interior mutability where
/// you need state.
///
/// **Do not block.** libCEC is waiting, and for the two methods that return
/// `bool` it gives up after a second and takes the default.
///
/// ```
/// use libcec::{CecCallbacks, Command, Keypress};
///
/// struct Logger;
///
/// impl CecCallbacks for Logger {
///     fn key_press(&self, key: &Keypress) {
///         if key.is_press() {
///             println!("{}", key.keycode);
///         }
///     }
///
///     fn command_received(&self, command: &Command) {
///         println!("{command}");
///     }
/// }
/// ```
pub trait CecCallbacks: Send + Sync {
    /// libCEC logged something.
    ///
    /// This fires a lot at the chattier levels; filter on
    /// [`LogMessage::level`](crate::LogMessage::level).
    fn log_message(&self, _message: &LogMessage) {}

    /// A remote control key was pressed or released.
    fn key_press(&self, _key: &Keypress) {}

    /// A CEC message arrived. Observation only - libCEC has already decided
    /// what to do with it. To intervene, use
    /// [`command_handler`](Self::command_handler).
    fn command_received(&self, _command: &Command) {}

    /// libCEC's configuration changed, usually because address allocation
    /// finished and the physical address is now known.
    fn configuration_changed(&self, _configuration: &Configuration) {}

    /// Something happened that is worth telling the user about - the adapter
    /// was unplugged, the port is busy, the TV stopped answering.
    fn alert(&self, _alert: Alert) {}

    /// A source this client owns was activated or deactivated.
    fn source_activated(&self, _address: LogicalAddress, _activated: bool) {}

    /// The menu state is about to change.
    ///
    /// Return `true` to let libCEC apply it, `false` (the default) to keep the
    /// device activated so keypresses keep being routed. Note that CEC gives no
    /// way to stop the *television* showing its menu either way.
    fn menu_state_changed(&self, _state: MenuState) -> bool {
        false
    }

    /// A CEC message arrived, before libCEC acts on it.
    ///
    /// Return `true` to say you have handled it and libCEC should do nothing
    /// further. The default is `false`: observe, and let libCEC get on with it.
    fn command_handler(&self, _command: &Command) -> bool {
        false
    }
}

/// Something libCEC reported, as a value.
///
/// The channel form of [`CecCallbacks`]. Each variant carries owned data, so an
/// event outlives the callback that produced it.
#[derive(Clone, PartialEq, Eq, Debug)]
pub enum CecEvent {
    /// See [`CecCallbacks::log_message`].
    LogMessage(LogMessage),
    /// See [`CecCallbacks::key_press`].
    KeyPress(Keypress),
    /// See [`CecCallbacks::command_received`].
    Command(Command),
    /// See [`CecCallbacks::configuration_changed`].
    ConfigurationChanged(Box<Configuration>),
    /// See [`CecCallbacks::alert`].
    Alert(Alert),
    /// See [`CecCallbacks::source_activated`].
    SourceActivated {
        /// The address that changed.
        address: LogicalAddress,
        /// True when activated, false when deactivated.
        activated: bool,
    },
    /// See [`CecCallbacks::menu_state_changed`]. Reported, not answered.
    MenuStateChanged(MenuState),
}

/// A [`CecCallbacks`] that forwards everything to an [`mpsc::Sender`].
///
/// Build one with [`channel`].
pub struct ChannelCallbacks {
    // Mutex, not a bare Sender: the trait is Sync and Sender is not, and the
    // lock is only ever held for the length of a send into an unbounded channel.
    sender: Mutex<Sender<CecEvent>>,
}

impl ChannelCallbacks {
    fn send(&self, event: CecEvent) {
        // A closed channel means the receiver is gone and nobody is listening;
        // that is the caller's choice, not an error to report from a callback.
        if let Ok(sender) = self.sender.lock() {
            let _ = sender.send(event);
        }
    }
}

impl CecCallbacks for ChannelCallbacks {
    fn log_message(&self, message: &LogMessage) {
        self.send(CecEvent::LogMessage(message.clone()));
    }

    fn key_press(&self, key: &Keypress) {
        self.send(CecEvent::KeyPress(*key));
    }

    fn command_received(&self, command: &Command) {
        self.send(CecEvent::Command(command.clone()));
    }

    fn configuration_changed(&self, configuration: &Configuration) {
        self.send(CecEvent::ConfigurationChanged(Box::new(
            configuration.clone(),
        )));
    }

    fn alert(&self, alert: Alert) {
        self.send(CecEvent::Alert(alert));
    }

    fn source_activated(&self, address: LogicalAddress, activated: bool) {
        self.send(CecEvent::SourceActivated { address, activated });
    }

    fn menu_state_changed(&self, state: MenuState) -> bool {
        self.send(CecEvent::MenuStateChanged(state));
        // Reported after the fact, so there is no answer to give: keep libCEC's
        // own handling rather than guessing on the application's behalf.
        false
    }
}

/// A [`CecCallbacks`] that turns every notification into a [`CecEvent`] on a
/// channel.
///
/// ```no_run
/// # fn main() -> Result<(), libcec::Error> {
/// use libcec::{callbacks::channel, CecEvent, ConnectionBuilder};
///
/// let (handler, events) = channel();
/// let _connection = ConnectionBuilder::new("RustCEC")
///     .callbacks(handler)
///     .open_first()?;
///
/// for event in events {
///     if let CecEvent::KeyPress(key) = event {
///         println!("{key}");
///     }
/// }
/// # Ok(())
/// # }
/// ```
pub fn channel() -> (std::sync::Arc<ChannelCallbacks>, Receiver<CecEvent>) {
    let (sender, receiver) = mpsc::channel();
    (
        std::sync::Arc::new(ChannelCallbacks {
            sender: Mutex::new(sender),
        }),
        receiver,
    )
}
