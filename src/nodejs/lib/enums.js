'use strict';

// Mirrors the enums in include/cectypes.h. Values are the single source of
// truth for the protocol surface, so keep them in sync with that header.

const CecDeviceType = Object.freeze({
  TV: 0,
  RecordingDevice: 1,
  Reserved: 2,
  Tuner: 3,
  PlaybackDevice: 4,
  AudioSystem: 5,
});

const CecLogicalAddress = Object.freeze({
  Unknown: -1,
  TV: 0,
  RecordingDevice1: 1,
  RecordingDevice2: 2,
  Tuner1: 3,
  PlaybackDevice1: 4,
  AudioSystem: 5,
  Tuner2: 6,
  Tuner3: 7,
  PlaybackDevice2: 8,
  RecordingDevice3: 9,
  Tuner4: 10,
  PlaybackDevice3: 11,
  Reserved1: 12,
  Reserved2: 13,
  FreeUse: 14,
  Unregistered: 15,
  Broadcast: 15,
});

const CecPowerStatus = Object.freeze({
  On: 0x00,
  Standby: 0x01,
  InTransitionStandbyToOn: 0x02,
  InTransitionOnToStandby: 0x03,
  Unknown: 0x99,
});

const CecVersion = Object.freeze({
  Unknown: 0x00,
  V1_2: 0x01,
  V1_2A: 0x02,
  V1_3: 0x03,
  V1_3A: 0x04,
  V1_4: 0x05,
  V2_0: 0x06,
});

const CecDisplayControl = Object.freeze({
  DisplayForDefaultTime: 0x00,
  DisplayUntilCleared: 0x40,
  ClearPreviousMessage: 0x80,
  ReservedForFutureUse: 0xC0,
});

const CecAdapterType = Object.freeze({
  Unknown: 0,
  P8External: 0x1,
  P8Daughterboard: 0x2,
  RPI: 0x100,
  TDA995x: 0x200,
  Exynos: 0x300,
  Linux: 0x400,
  AOCEC: 0x500,
  IMX: 0x600,
  Tegra: 0x700,
});

const CecLogLevel = Object.freeze({
  Error: 1,
  Warning: 2,
  Notice: 4,
  Traffic: 8,
  Debug: 16,
  All: 31,
});

const CecAlert = Object.freeze({
  ServiceDevice: 0,
  ConnectionLost: 1,
  PermissionError: 2,
  PortBusy: 3,
  PhysicalAddressError: 4,
  TvPollFailed: 5,
});

// The common remote-control keys. See cectypes.h cec_user_control_code for the
// full list; these are the ones a controller normally sends.
const CecUserControlCode = Object.freeze({
  Select: 0x00,
  Up: 0x01,
  Down: 0x02,
  Left: 0x03,
  Right: 0x04,
  RootMenu: 0x09,
  SetupMenu: 0x0A,
  ContentsMenu: 0x0B,
  Exit: 0x0D,
  Enter: 0x2B,
  ChannelUp: 0x30,
  ChannelDown: 0x31,
  Power: 0x40,
  VolumeUp: 0x41,
  VolumeDown: 0x42,
  Mute: 0x43,
  Play: 0x44,
  Stop: 0x45,
  Pause: 0x46,
  Record: 0x47,
  Rewind: 0x48,
  FastForward: 0x49,
  Forward: 0x4B,
  Backward: 0x4C,
  RootMenuF1Blue: 0x71,
  PowerToggleFunction: 0x6B,
  PowerOffFunction: 0x6C,
  PowerOnFunction: 0x6D,
  Unknown: 0xFF,
});

// A small subset of cec_opcode — enough for hand-built transmit() calls. Use
// opcodeToString() from the addon to name any value libCEC reports.
const CecOpcode = Object.freeze({
  ActiveSource: 0x82,
  ImageViewOn: 0x04,
  TextViewOn: 0x0D,
  Standby: 0x36,
  GiveDevicePowerStatus: 0x8F,
  ReportPowerStatus: 0x90,
  GiveOSDName: 0x46,
  SetOSDName: 0x47,
  GiveDeviceVendorId: 0x8C,
  UserControlPressed: 0x44,
  UserControlRelease: 0x45,
  GetCecVersion: 0x9F,
  CecVersion: 0x9E,
  None: 0xFD,
});

module.exports = {
  CecDeviceType,
  CecLogicalAddress,
  CecPowerStatus,
  CecVersion,
  CecDisplayControl,
  CecAdapterType,
  CecLogLevel,
  CecAlert,
  CecUserControlCode,
  CecOpcode,
};
