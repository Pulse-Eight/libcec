/**
 * Type declarations for the **libCEC** Node.js binding.
 *
 * A native N-API addon that binds libCEC through its C API, letting Node.js
 * applications control CEC-capable HDMI devices (power a TV on/standby, become
 * the active source, send remote keys, read device state) and observe bus
 * events. See the package README for build requirements and a worked example.
 *
 * @packageDocumentation
 */

/// <reference types="node" />
import { EventEmitter } from 'events';

// ---------------------------------------------------------------------------
// Enums — values mirror include/cectypes.h (the protocol source of truth).
// ---------------------------------------------------------------------------

/** CEC device type claimed by, or reported for, a device on the bus. */
export enum CecDeviceType {
  TV = 0,
  RecordingDevice = 1,
  Reserved = 2,
  Tuner = 3,
  PlaybackDevice = 4,
  AudioSystem = 5,
}

/** A device's logical address on the CEC bus (its role/slot). */
export enum CecLogicalAddress {
  Unknown = -1,
  TV = 0,
  RecordingDevice1 = 1,
  RecordingDevice2 = 2,
  Tuner1 = 3,
  PlaybackDevice1 = 4,
  AudioSystem = 5,
  Tuner2 = 6,
  Tuner3 = 7,
  PlaybackDevice2 = 8,
  RecordingDevice3 = 9,
  Tuner4 = 10,
  PlaybackDevice3 = 11,
  Reserved1 = 12,
  Reserved2 = 13,
  FreeUse = 14,
  Unregistered = 15,
  /** Same value as {@link CecLogicalAddress.Unregistered} (15). */
  Broadcast = 15,
}

/** Power state of a device on the bus. */
export enum CecPowerStatus {
  On = 0x00,
  Standby = 0x01,
  InTransitionStandbyToOn = 0x02,
  InTransitionOnToStandby = 0x03,
  Unknown = 0x99,
}

/** CEC protocol version a device reports. */
export enum CecVersion {
  Unknown = 0x00,
  V1_2 = 0x01,
  V1_2A = 0x02,
  V1_3 = 0x03,
  V1_3A = 0x04,
  V1_4 = 0x05,
  V2_0 = 0x06,
}

/** How long an on-screen-display string stays up (see {@link CecAdapter.setOSDString}). */
export enum CecDisplayControl {
  DisplayForDefaultTime = 0x00,
  DisplayUntilCleared = 0x40,
  ClearPreviousMessage = 0x80,
  ReservedForFutureUse = 0xc0,
}

/** The kind of CEC adapter/backend in use. */
export enum CecAdapterType {
  Unknown = 0,
  P8External = 0x1,
  P8Daughterboard = 0x2,
  RPI = 0x100,
  TDA995x = 0x200,
  Exynos = 0x300,
  Linux = 0x400,
  AOCEC = 0x500,
  IMX = 0x600,
  Tegra = 0x700,
}

/** Severity of a {@link CecAdapterEvents.log} message (a bitmask). */
export enum CecLogLevel {
  Error = 1,
  Warning = 2,
  Notice = 4,
  Traffic = 8,
  Debug = 16,
  All = 31,
}

/** Asynchronous alert raised by libCEC (see {@link CecAdapterEvents.alert}). */
export enum CecAlert {
  ServiceDevice = 0,
  ConnectionLost = 1,
  PermissionError = 2,
  PortBusy = 3,
  PhysicalAddressError = 4,
  TvPollFailed = 5,
}

/**
 * Common remote-control keys. This is the subset a controller normally sends;
 * see `cec_user_control_code` in cectypes.h for the full list.
 */
export enum CecUserControlCode {
  Select = 0x00,
  Up = 0x01,
  Down = 0x02,
  Left = 0x03,
  Right = 0x04,
  RootMenu = 0x09,
  SetupMenu = 0x0a,
  ContentsMenu = 0x0b,
  Exit = 0x0d,
  Enter = 0x2b,
  ChannelUp = 0x30,
  ChannelDown = 0x31,
  Power = 0x40,
  VolumeUp = 0x41,
  VolumeDown = 0x42,
  Mute = 0x43,
  Play = 0x44,
  Stop = 0x45,
  Pause = 0x46,
  Record = 0x47,
  Rewind = 0x48,
  FastForward = 0x49,
  Forward = 0x4b,
  Backward = 0x4c,
  RootMenuF1Blue = 0x71,
  PowerToggleFunction = 0x6b,
  PowerOffFunction = 0x6c,
  PowerOnFunction = 0x6d,
  Unknown = 0xff,
}

/**
 * A small subset of `cec_opcode` — enough for hand-built {@link CecAdapter.transmit}
 * calls. Use {@link opcodeToString} to name any value libCEC reports.
 */
export enum CecOpcode {
  ActiveSource = 0x82,
  ImageViewOn = 0x04,
  TextViewOn = 0x0d,
  Standby = 0x36,
  GiveDevicePowerStatus = 0x8f,
  ReportPowerStatus = 0x90,
  GiveOSDName = 0x46,
  SetOSDName = 0x47,
  GiveDeviceVendorId = 0x8c,
  UserControlPressed = 0x44,
  UserControlRelease = 0x45,
  GetCecVersion = 0x9f,
  CecVersion = 0x9e,
  None = 0xfd,
}

// ---------------------------------------------------------------------------
// Structured payloads
// ---------------------------------------------------------------------------

/** Options passed to the {@link CecAdapter} constructor. All fields are optional. */
export interface CecAdapterOptions {
  /** OSD name shown on the bus (only the first 13 characters are used). */
  deviceName?: string;
  /** Device type this instance presents as. Defaults to `RecordingDevice`. */
  deviceType?: CecDeviceType;
  /** Physical address, e.g. `0x1000`; `0` (default) autodetects it. */
  physicalAddress?: number;
  /** Logical address of the device this adapter hangs off. */
  baseDevice?: CecLogicalAddress;
  /** 1-based HDMI port used for physical-address detection. */
  hdmiPort?: number;
  /** Become the active source when the connection opens. */
  activateSource?: boolean;
  /** Passive monitor: claim no logical address, only observe traffic. */
  monitorOnly?: boolean;
  /** CEC version to advertise. */
  cecVersion?: CecVersion;
  /**
   * Also emit the {@link CecAdapterEvents.commandHandler} event. Off by default:
   * it routes *every* command through libCEC's blocking command-handler path and
   * carries the same data as the cheaper `command` event, and it cannot suppress
   * libCEC's default handling.
   */
  enableCommandHandler?: boolean;
}

/** A single log line from libCEC ({@link CecAdapterEvents.log}). */
export interface LogMessage {
  message: string;
  /** One of {@link CecLogLevel}. */
  level: number;
  /** libCEC timestamp in milliseconds. */
  time: number;
}

/** A remote-control key event ({@link CecAdapterEvents.keyPress}). */
export interface KeyPress {
  /** One of {@link CecUserControlCode}. */
  keycode: number;
  /** How long the key was held, in milliseconds. */
  duration: number;
}

/** A CEC command frame ({@link CecAdapterEvents.command} / {@link CecAdapter.transmit}). */
export interface CecCommand {
  initiator: CecLogicalAddress;
  destination: CecLogicalAddress;
  /** One of {@link CecOpcode}. */
  opcode: number;
  /** Command parameters as raw bytes. */
  parameters: number[];
  ack: boolean;
  eom: boolean;
  /** Whether {@link CecCommand.opcode} is set (some frames are opcode-less polls). */
  opcodeSet: boolean;
  transmitTimeout: number;
}

/** An adapter returned by {@link CecAdapter.detectAdapters}. */
export interface AdapterDescriptor {
  /** Port/device path to pass to {@link CecAdapter.open}, e.g. `/dev/ttyACM0`. */
  path: string;
  /** Underlying communication port. */
  comm: string;
  vendorId?: number;
  productId?: number;
  firmwareVersion?: number;
  adapterType?: CecAdapterType;
}

/** Snapshot delivered by {@link CecAdapterEvents.configurationChanged}. */
export interface CecConfiguration {
  deviceName: string;
  deviceTypes: CecDeviceType[];
  physicalAddress: number;
  logicalAddresses: CecLogicalAddress[];
  cecVersion: CecVersion;
  adapterType: CecAdapterType;
  firmwareVersion: number;
  [key: string]: unknown;
}

/** Build/version information returned by {@link CecAdapter.getLibInfo}. */
export type LibInfo = string;

/**
 * The strongly-typed event surface of {@link CecAdapter}. Names map to the
 * addon's `on(event, listener)` overloads.
 */
export interface CecAdapterEvents {
  log: (message: LogMessage) => void;
  keyPress: (key: KeyPress) => void;
  command: (command: CecCommand) => void;
  sourceActivated: (address: CecLogicalAddress, activated: boolean) => void;
  alert: (alert: CecAlert) => void;
  configurationChanged: (config: CecConfiguration) => void;
  menuStateChanged: (state: number) => void;
  /** Opt-in; enable with `{ enableCommandHandler: true }`. Observe-only. */
  commandHandler: (command: CecCommand) => void;
}

// ---------------------------------------------------------------------------
// The adapter
// ---------------------------------------------------------------------------

/**
 * An open (or openable) connection to a CEC adapter.
 *
 * `CecAdapter` is an {@link EventEmitter}: libCEC fires its callbacks from its
 * own worker thread and the addon marshals each onto the Node event loop, so
 * listeners run on the main thread like any other event. Always call
 * {@link CecAdapter.close} when finished.
 */
export class CecAdapter extends EventEmitter {
  constructor(options?: CecAdapterOptions);

  // Typed event overloads, plus a passthrough for anything else.
  on<E extends keyof CecAdapterEvents>(event: E, listener: CecAdapterEvents[E]): this;
  once<E extends keyof CecAdapterEvents>(event: E, listener: CecAdapterEvents[E]): this;
  off<E extends keyof CecAdapterEvents>(event: E, listener: CecAdapterEvents[E]): this;

  // --- Lifecycle -----------------------------------------------------------
  /** Open a connection to the adapter at `port`. Returns `true` on success. */
  open(port?: string, timeout?: number): boolean;
  /** Close the connection and stop libCEC's worker thread. */
  close(): void;

  // --- Control -------------------------------------------------------------
  /** Transmit a raw CEC command frame. */
  transmit(command: CecCommand): boolean;
  /** Power on the given device (default: broadcast). */
  powerOnDevices(address?: CecLogicalAddress): boolean;
  /** Put the given device into standby (default: broadcast). */
  standbyDevices(address?: CecLogicalAddress): boolean;
  /** Become the active source, presenting as `deviceType`. */
  setActiveSource(deviceType?: CecDeviceType): boolean;
  /** Mark this device as no longer the active source. */
  setInactiveView(): boolean;
  /** Raise the volume on the audio system. Returns the new volume (0–100, 0xFF unknown). */
  volumeUp(sendRelease?: boolean): number;
  /** Lower the volume on the audio system. Returns the new volume. */
  volumeDown(sendRelease?: boolean): number;
  /** Toggle mute on the audio system. Returns the new volume. */
  muteAudio(sendRelease?: boolean): number;
  /** Send a remote-key press to `destination`. */
  sendKeypress(destination: CecLogicalAddress, key: CecUserControlCode, wait?: boolean): boolean;
  /** Send a remote-key release to `destination`. */
  sendKeyRelease(destination: CecLogicalAddress, wait?: boolean): boolean;
  /** Show `message` on `destination`'s OSD for `duration`. */
  setOSDString(destination: CecLogicalAddress, duration: CecDisplayControl, message: string): boolean;

  // --- Queries -------------------------------------------------------------
  /** Logical address of the current active source. */
  getActiveSource(): CecLogicalAddress;
  /** Whether `address` is the current active source. */
  isActiveSource(address: CecLogicalAddress): boolean;
  getDevicePowerStatus(address: CecLogicalAddress): CecPowerStatus;
  /** Vendor id of the device (see {@link vendorIdToString}). */
  getDeviceVendorId(address: CecLogicalAddress): number;
  getDevicePhysicalAddress(address: CecLogicalAddress): number;
  getDeviceCecVersion(address: CecLogicalAddress): CecVersion;
  getDeviceOSDName(address: CecLogicalAddress): string;
  /** Logical addresses of all active devices on the bus. */
  getActiveDevices(): CecLogicalAddress[];
  /** Logical addresses this adapter has claimed. */
  getLogicalAddresses(): CecLogicalAddress[];
  /** Enable/disable passive monitoring mode. */
  switchMonitoring(enable: boolean): boolean;
  /** Set the stream path to a physical address (routes the source). */
  setStreamPathPhysical(physicalAddress: number): boolean;
  /** Set the stream path to a device by logical address. */
  setStreamPathLogical(address: CecLogicalAddress): boolean;
  /** Poll a device to check whether it is present. */
  pollDevice(address: CecLogicalAddress): boolean;
  /** Re-scan the bus for active devices. */
  rescanDevices(): void;
  /** Ping all connected adapters. */
  pingAdapters(): boolean;
  /** Enumerate the connected CEC adapters. */
  detectAdapters(): AdapterDescriptor[];
  /** libCEC build/version information. */
  getLibInfo(): LibInfo;
}

// ---------------------------------------------------------------------------
// Module-level enum-to-string helpers (implemented in the native addon)
// ---------------------------------------------------------------------------

/** Human-readable name for a {@link CecVersion} value. */
export function cecVersionToString(version: number): string;
/** Human-readable name for a {@link CecPowerStatus} value. */
export function powerStatusToString(status: number): string;
/** Human-readable name for a {@link CecLogicalAddress} value. */
export function logicalAddressToString(address: number): string;
/** Human-readable vendor name for a vendor id. */
export function vendorIdToString(vendorId: number): string;
/** Human-readable name for a {@link CecOpcode} value. */
export function opcodeToString(opcode: number): string;
/** Human-readable name for a {@link CecUserControlCode} value. */
export function userControlKeyToString(key: number): string;
