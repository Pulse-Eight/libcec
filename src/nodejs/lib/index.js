'use strict';

// Public entry point for the libCEC Node.js binding.
//
// The native addon (build/Release/cec_native.node) exposes the raw CecAdapter
// class and a handful of enum-to-string helpers. This wrapper adds an
// EventEmitter surface on top of the C callbacks so consumers write
//   adapter.on('command', cmd => ...)
// instead of passing callback functions into the constructor.

const { EventEmitter } = require('events');
const enums = require('./enums');

// bindings-free load: node-gyp puts the addon under build/Release.
const native = require('../build/Release/cec_native.node');

class CecAdapter extends EventEmitter {
  /**
   * @param {object} [options]
   *   deviceName       {string}  OSD name (<=13 chars used on the bus)
   *   deviceType       {number}  CecDeviceType.* (default RecordingDevice)
   *   physicalAddress  {number}  e.g. 0x1000; 0 to autodetect
   *   baseDevice       {number}  CecLogicalAddress.* the adapter hangs off
   *   hdmiPort         {number}  1-based HDMI port for address detection
   *   activateSource   {boolean} become active source on open
   *   monitorOnly      {boolean} passive monitor, claims no logical address
   *   cecVersion       {number}  CecVersion.*
   *   enableCommandHandler {boolean} also emit the 'commandHandler' event. Off by
   *     default: it routes *every* command through libCEC's blocking
   *     command-handler path, and carries the same data as the cheaper 'command'
   *     event. It cannot suppress libCEC's default handling (see README).
   */
  constructor(options = {}) {
    super();
    const nativeOpts = {
      ...options,
      onLogMessage: (m) => this.emit('log', m),
      onKeyPress: (k) => this.emit('keyPress', k),
      onCommand: (c) => this.emit('command', c),
      onSourceActivated: (address, activated) => this.emit('sourceActivated', address, activated),
      onAlert: (alert) => this.emit('alert', alert),
      onConfigurationChanged: (config) => this.emit('configurationChanged', config),
      onMenuStateChanged: (state) => this.emit('menuStateChanged', state),
    };
    if (options.enableCommandHandler)
      nativeOpts.onCommandHandler = (c) => this.emit('commandHandler', c);
    this._native = new native.CecAdapter(nativeOpts);
  }

  open(port, timeout = 10000) { return this._native.open(port ?? '', timeout); }
  close() { this._native.close(); }

  transmit(command) { return this._native.transmit(command); }
  powerOnDevices(address = enums.CecLogicalAddress.Broadcast) { return this._native.powerOnDevices(address); }
  standbyDevices(address = enums.CecLogicalAddress.Broadcast) { return this._native.standbyDevices(address); }
  setActiveSource(deviceType = enums.CecDeviceType.Reserved) { return this._native.setActiveSource(deviceType); }
  setInactiveView() { return this._native.setInactiveView(); }

  volumeUp(sendRelease = true) { return this._native.volumeUp(sendRelease); }
  volumeDown(sendRelease = true) { return this._native.volumeDown(sendRelease); }
  muteAudio(sendRelease = true) { return this._native.muteAudio(sendRelease); }

  sendKeypress(destination, key, wait = true) { return this._native.sendKeypress(destination, key, wait); }
  sendKeyRelease(destination, wait = true) { return this._native.sendKeyRelease(destination, wait); }
  setOSDString(destination, duration, message) { return this._native.setOSDString(destination, duration, message); }

  getActiveSource() { return this._native.getActiveSource(); }
  isActiveSource(address) { return this._native.isActiveSource(address); }
  getDevicePowerStatus(address) { return this._native.getDevicePowerStatus(address); }
  getDeviceVendorId(address) { return this._native.getDeviceVendorId(address); }
  getDevicePhysicalAddress(address) { return this._native.getDevicePhysicalAddress(address); }
  getDeviceCecVersion(address) { return this._native.getDeviceCecVersion(address); }
  getDeviceOSDName(address) { return this._native.getDeviceOSDName(address); }
  getActiveDevices() { return this._native.getActiveDevices(); }
  getLogicalAddresses() { return this._native.getLogicalAddresses(); }
  switchMonitoring(enable) { return this._native.switchMonitoring(enable); }
  setStreamPathPhysical(physicalAddress) { return this._native.setStreamPathPhysical(physicalAddress); }
  setStreamPathLogical(address) { return this._native.setStreamPathLogical(address); }
  pollDevice(address) { return this._native.pollDevice(address); }
  rescanDevices() { return this._native.rescanDevices(); }
  pingAdapters() { return this._native.pingAdapters(); }
  detectAdapters() { return this._native.detectAdapters(); }
  getLibInfo() { return this._native.getLibInfo(); }
}

module.exports = {
  CecAdapter,
  // enum-to-string helpers, straight from the native side
  cecVersionToString: native.cecVersionToString,
  powerStatusToString: native.powerStatusToString,
  logicalAddressToString: native.logicalAddressToString,
  vendorIdToString: native.vendorIdToString,
  opcodeToString: native.opcodeToString,
  userControlKeyToString: native.userControlKeyToString,
  ...enums,
};
