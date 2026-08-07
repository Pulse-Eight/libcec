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

//! What can go wrong.
//!
//! libCEC's C API reports failure as a plain `0`, with no code and no message,
//! so there is nothing to translate: the useful information is *which call*
//! failed, not a reason libCEC never gave. [`Error::Call`] therefore names the
//! operation and stops there rather than inventing detail.

use std::fmt;

/// The result of an operation that talks to libCEC.
pub type Result<T> = std::result::Result<T, Error>;

/// Something libCEC would not do.
#[derive(Clone, PartialEq, Eq, Debug)]
pub enum Error {
    /// `libcec_initialise` returned null. Usually a configuration libCEC would
    /// not accept, such as an out-of-range HDMI port.
    Initialise,

    /// The adapter would not open.
    ///
    /// Carries the port that was tried, or `None` when libCEC was left to pick
    /// one - in which case no adapter it detected could be opened. The usual
    /// causes are no adapter attached, the port being held by another process,
    /// or - on Linux - no permission on the device node.
    Open(Option<String>),

    /// A call failed. The operation is named because libCEC does not say more.
    Call(&'static str),

    /// The connection has been closed; reopen it to keep going.
    Closed,

    /// A string could not be handed to libCEC as it stands.
    InvalidString {
        /// The field that rejected it.
        field: &'static str,
        /// Why.
        reason: &'static str,
    },

    /// More parameter bytes than a single CEC message can carry
    /// ([`CEC_MAX_DATA_PACKET_SIZE`](crate::ffi::CEC_MAX_DATA_PACKET_SIZE)).
    ParametersTooLong(usize),
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Error::Initialise => f.write_str("libCEC would not accept this configuration"),
            Error::Open(Some(port)) => write!(f, "could not open the CEC adapter on {port}"),
            Error::Open(None) => f.write_str("could not open a CEC adapter"),
            Error::Call(what) => write!(f, "libCEC refused to {what}"),
            Error::Closed => f.write_str("the connection is closed"),
            Error::InvalidString { field, reason } => write!(f, "{field}: {reason}"),
            Error::ParametersTooLong(len) => write!(
                f,
                "{len} parameter bytes is more than a CEC message can carry ({} max)",
                crate::ffi::CEC_MAX_DATA_PACKET_SIZE
            ),
        }
    }
}

impl std::error::Error for Error {}
