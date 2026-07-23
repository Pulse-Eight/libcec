/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2025 Pulse-Eight Limited.  All rights reserved.
 * libCEC(R) is an original work, containing original code.
 *
 * libCEC(R) is a trademark of Pulse-Eight Limited.
 *
 * This program is dual-licensed; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Alternatively, you can license this library under a commercial license,
 * please contact Pulse-Eight Licensing for more information.
 *
 * For more information contact:
 * Pulse-Eight Licensing       <license@pulse-eight.com>
 *     http://www.pulse-eight.com/
 *     http://www.pulse-eight.net/
 */

// Pure C# port of the CecSharp enums. The values mirror include/cectypes.h
// exactly, so this assembly binds the native C API (include/cecc.h) directly
// over P/Invoke and needs no C++/CLI compiler. See src/dotnetlib/CecSharpTypes.h
// for the original C++/CLI definitions this replaces.

namespace CecSharp
{
  /// <summary>
  /// The device type. For client applications, libCEC only supports RecordingDevice, PlaybackDevice or Tuner.
  /// libCEC uses RecordingDevice by default.
  /// </summary>
  public enum CecDeviceType
  {
    Tv              = 0,
    RecordingDevice = 1,
    Reserved        = 2,
    Tuner           = 3,
    PlaybackDevice  = 4,
    AudioSystem     = 5
  }

  /// <summary>
  /// Log level that can be used by the logging callback method to filter messages from libCEC.
  /// </summary>
  public enum CecLogLevel
  {
    None    = 0,
    Error   = 1,
    Warning = 2,
    Notice  = 4,
    Traffic = 8,
    Debug   = 16,
    All     = 31
  }

  /// <summary>
  /// A logical address on the CEC bus
  /// </summary>
  public enum CecLogicalAddress
  {
    Unknown          = -1,
    Tv               = 0,
    RecordingDevice1 = 1,
    RecordingDevice2 = 2,
    Tuner1           = 3,
    PlaybackDevice1  = 4,
    AudioSystem      = 5,
    Tuner2           = 6,
    Tuner3           = 7,
    PlaybackDevice2  = 8,
    RecordingDevice3 = 9,
    Tuner4           = 10,
    PlaybackDevice3  = 11,
    Reserved1        = 12,
    Reserved2        = 13,
    FreeUse          = 14,
    Unregistered     = 15,
    Broadcast        = 15
  }

  /// <summary>
  /// The type of alert when libCEC calls the CecAlert callback
  /// </summary>
  public enum CecAlert
  {
    ServiceDevice = 0,
    ConnectionLost,
    PermissionError,
    PortBusy,
    PhysicalAddressError,
    TVPollFailed
  }

  /// <summary>
  /// The type of parameter that is sent with the CecAlert callback
  /// </summary>
  public enum CecParameterType
  {
    ParameterTypeString = 1
  }

  /// <summary>
  /// The power status of a CEC device
  /// </summary>
  public enum CecPowerStatus
  {
    On                      = 0x00,
    Standby                 = 0x01,
    InTransitionStandbyToOn = 0x02,
    InTransitionOnToStandby = 0x03,
    Unknown                 = 0x99
  }

  /// <summary>
  /// The CEC version of a CEC device
  /// </summary>
  public enum CecVersion
  {
    Unknown = 0x00,
    V1_2    = 0x01,
    V1_2A   = 0x02,
    V1_3    = 0x03,
    V1_3A   = 0x04,
    V1_4    = 0x05,
    V2_0    = 0x06
  }

  /// <summary>
  /// Parameter for OSD string display, that controls how to display the string
  /// </summary>
  public enum CecDisplayControl
  {
    DisplayForDefaultTime = 0x00,
    DisplayUntilCleared   = 0x40,
    ClearPreviousMessage  = 0x80,
    ReservedForFutureUse  = 0xC0
  }

  /// <summary>
  /// The menu state of a CEC device
  /// </summary>
  public enum CecMenuState
  {
    Activated   = 0,
    Deactivated = 1
  }

  /// <summary>
  /// Deck control mode for playback and recording devices
  /// </summary>
  public enum CecDeckControlMode
  {
    SkipForwardWind   = 1,
    SkipReverseRewind = 2,
    Stop              = 3,
    Eject             = 4
  }

  /// <summary>
  /// Deck status for playback and recording devices
  /// </summary>
  public enum CecDeckInfo
  {
    Play               = 0x11,
    Record             = 0x12,
    Reverse            = 0x13,
    Still              = 0x14,
    Slow               = 0x15,
    SlowReverse        = 0x16,
    FastForward        = 0x17,
    FastReverse        = 0x18,
    NoMedia            = 0x19,
    Stop               = 0x1A,
    SkipForwardWind    = 0x1B,
    SkipReverseRewind  = 0x1C,
    IndexSearchForward = 0x1D,
    IndexSearchReverse = 0x1E,
    OtherStatus        = 0x1F
  }

  /// <summary>
  /// User control code, the key code when the user presses or releases a button on the remote.
  /// Used by SendKeypress() and the CecKey callback.
  /// </summary>
  public enum CecUserControlCode
  {
    Select                      = 0x00,
    Up                          = 0x01,
    Down                        = 0x02,
    Left                        = 0x03,
    Right                       = 0x04,
    RightUp                     = 0x05,
    RightDown                   = 0x06,
    LeftUp                      = 0x07,
    LeftDown                    = 0x08,
    RootMenu                    = 0x09,
    SetupMenu                   = 0x0A,
    ContentsMenu                = 0x0B,
    FavoriteMenu                = 0x0C,
    Exit                        = 0x0D,
    Number0                     = 0x20,
    Number1                     = 0x21,
    Number2                     = 0x22,
    Number3                     = 0x23,
    Number4                     = 0x24,
    Number5                     = 0x25,
    Number6                     = 0x26,
    Number7                     = 0x27,
    Number8                     = 0x28,
    Number9                     = 0x29,
    Dot                         = 0x2A,
    Enter                       = 0x2B,
    Clear                       = 0x2C,
    NextFavorite                = 0x2F,
    ChannelUp                   = 0x30,
    ChannelDown                 = 0x31,
    PreviousChannel             = 0x32,
    SoundSelect                 = 0x33,
    InputSelect                 = 0x34,
    DisplayInformation          = 0x35,
    Help                        = 0x36,
    PageUp                      = 0x37,
    PageDown                    = 0x38,
    Power                       = 0x40,
    VolumeUp                    = 0x41,
    VolumeDown                  = 0x42,
    Mute                        = 0x43,
    Play                        = 0x44,
    Stop                        = 0x45,
    Pause                       = 0x46,
    Record                      = 0x47,
    Rewind                      = 0x48,
    FastForward                 = 0x49,
    Eject                       = 0x4A,
    Forward                     = 0x4B,
    Backward                    = 0x4C,
    StopRecord                  = 0x4D,
    PauseRecord                 = 0x4E,
    Angle                       = 0x50,
    SubPicture                  = 0x51,
    VideoOnDemand               = 0x52,
    ElectronicProgramGuide      = 0x53,
    TimerProgramming            = 0x54,
    InitialConfiguration        = 0x55,
    PlayFunction                = 0x60,
    PausePlayFunction           = 0x61,
    RecordFunction              = 0x62,
    PauseRecordFunction         = 0x63,
    StopFunction                = 0x64,
    MuteFunction                = 0x65,
    RestoreVolumeFunction       = 0x66,
    TuneFunction                = 0x67,
    SelectMediaFunction         = 0x68,
    SelectAVInputFunction       = 0x69,
    SelectAudioInputFunction    = 0x6A,
    PowerToggleFunction         = 0x6B,
    PowerOffFunction            = 0x6C,
    PowerOnFunction             = 0x6D,
    F1Blue                      = 0x71,
    F2Red                       = 0X72,
    F3Green                     = 0x73,
    F4Yellow                    = 0x74,
    F5                          = 0x75,
    Data                        = 0x76,
    Max                         = 0x76,
    SamsungReturn               = 0x91,
    Unknown                     = 0xFF
  }

  /// <summary>
  /// Vendor IDs for CEC devices
  /// </summary>
  public enum CecVendorId
  {
    Toshiba       = 0x000039,
    Samsung       = 0x0000F0,
    Denon         = 0x0005CD,
    Marantz       = 0x000678,
    Loewe         = 0x000982,
    Onkyo         = 0x0009B0,
    Medion        = 0x000CB8,
    Toshiba2      = 0x000CE7,
    Apple         = 0x0010FA,
    PulseEight    = 0x001582,
    HarmanKardon2 = 0x001950,
    Google        = 0x001A11,
    Akai          = 0x0020C7,
    AOC           = 0x002467,
    Panasonic     = 0x008045,
    Philips       = 0x00903E,
    Daewoo        = 0x009053,
    Yamaha        = 0x00A0DE,
    Grundig       = 0x00D0D5,
    Pioneer       = 0x00E036,
    LG            = 0x00E091,
    Sharp         = 0x08001F,
    Sony          = 0x080046,
    Broadcom      = 0x18C086,
    Sharp2        = 0x534850,
    Teufel        = 0x232425,
    Vizio         = 0x6B746D,
    Benq          = 0x8065E9,
    HarmanKardon  = 0x9C645E,
    Unknown       = 0
  }

  /// <summary>
  /// Audio status of audio system / AVR devices
  /// </summary>
  public enum CecAudioStatus
  {
    MuteStatusMask      = 0x80,
    VolumeStatusMask    = 0x7F,
    VolumeMin           = 0x00,
    VolumeMax           = 0x64,
    VolumeStatusUnknown = 0x7F
  }

  /// <summary>
  /// CEC opcodes, as described in the HDMI CEC specification
  /// </summary>
  public enum CecOpcode
  {
    ActiveSource                  = 0x82,
    ImageViewOn                   = 0x04,
    TextViewOn                    = 0x0D,
    InactiveSource                = 0x9D,
    RequestActiveSource           = 0x85,
    RoutingChange                 = 0x80,
    RoutingInformation            = 0x81,
    SetStreamPath                 = 0x86,
    Standby                       = 0x36,
    RecordOff                     = 0x0B,
    RecordOn                      = 0x09,
    RecordStatus                  = 0x0A,
    RecordTvScreen                = 0x0F,
    ClearAnalogueTimer            = 0x33,
    ClearDigitalTimer             = 0x99,
    ClearExternalTimer            = 0xA1,
    SetAnalogueTimer              = 0x34,
    SetDigitalTimer               = 0x97,
    SetExternalTimer              = 0xA2,
    SetTimerProgramTitle          = 0x67,
    TimerClearedStatus            = 0x43,
    TimerStatus                   = 0x35,
    CecVersion                    = 0x9E,
    GetCecVersion                 = 0x9F,
    GivePhysicalAddress           = 0x83,
    GetMenuLanguage               = 0x91,
    ReportPhysicalAddress         = 0x84,
    SetMenuLanguage               = 0x32,
    DeckControl                   = 0x42,
    DeckStatus                    = 0x1B,
    GiveDeckStatus                = 0x1A,
    Play                          = 0x41,
    GiveTunerDeviceStatus         = 0x08,
    SelectAnalogueService         = 0x92,
    SelectDigtalService           = 0x93,
    TunerDeviceStatus             = 0x07,
    TunerStepDecrement            = 0x06,
    TunerStepIncrement            = 0x05,
    DeviceVendorId                = 0x87,
    GiveDeviceVendorId            = 0x8C,
    VendorCommand                 = 0x89,
    VendorCommandWithId           = 0xA0,
    VendorRemoteButtonDown        = 0x8A,
    VendorRemoteButtonUp          = 0x8B,
    SetOsdString                  = 0x64,
    GiveOsdName                   = 0x46,
    SetOsdName                    = 0x47,
    MenuRequest                   = 0x8D,
    MenuStatus                    = 0x8E,
    UserControlPressed            = 0x44,
    UserControlRelease            = 0x45,
    GiveDevicePowerStatus         = 0x8F,
    ReportPowerStatus             = 0x90,
    FeatureAbort                  = 0x00,
    Abort                         = 0xFF,
    GiveAudioStatus               = 0x71,
    GiveSystemAudioMode           = 0x7D,
    ReportAudioStatus             = 0x7A,
    SetSystemAudioMode            = 0x72,
    SystemAudioModeRequest        = 0x70,
    SystemAudioModeStatus         = 0x7E,
    SetAudioRate                  = 0x9A,
    None                          = 0xFD
  }

  /// <summary>
  /// Audiosystem status
  /// </summary>
  public enum CecSystemAudioStatus
  {
    Off = 0,
    On  = 1
  }

  /// <summary>
  /// A setting that can be enabled, disabled or not changed
  /// </summary>
  public enum BoolSetting
  {
    Disabled = 0,
    Enabled = 1,
    NotSet = 2
  }

  /// <summary>
  /// Type of adapter to which libCEC is connected
  /// </summary>
  public enum CecAdapterType
  {
    Unknown                 = 0,
    PulseEightExternal      = 0x1,
    PulseEightDaughterboard = 0x2,
    RaspberryPi             = 0x100,
    TDA995x                 = 0x200
  }
}
