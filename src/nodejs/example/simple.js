'use strict';

// Minimal smoke test, mirroring the other language samples: open the first
// adapter, log traffic, list the devices on the bus, then power the TV on.
//
//   cd src/nodejs && npm install && node example/simple.js

const cec = require('..');

const adapter = new cec.CecAdapter({
  deviceName: 'CECNode',
  deviceType: cec.CecDeviceType.RecordingDevice,
  activateSource: false,
});

adapter.on('log', (m) => {
  // level is a CecLogLevel bit; skip the very chatty debug traffic here.
  if (m.level <= cec.CecLogLevel.Notice)
    console.log(`[cec] ${m.message}`);
});
adapter.on('keyPress', (k) =>
  console.log(`key: ${cec.userControlKeyToString(k.keycode)} (${k.duration}ms)`));
adapter.on('command', (c) =>
  console.log(`cmd: ${cec.opcodeToString(c.opcode)} ${c.initiator}->${c.destination}`));
adapter.on('sourceActivated', (addr, on) =>
  console.log(`source ${cec.logicalAddressToString(addr)} ${on ? 'activated' : 'deactivated'}`));
adapter.on('alert', (a) => console.log(`alert: ${a}`));

const found = adapter.detectAdapters();
if (found.length === 0) {
  console.error('no CEC adapters found');
  adapter.close();
  process.exit(1);
}

const port = found[0].path;
console.log(`opening ${port} ...`);
if (!adapter.open(port)) {
  console.error('failed to open adapter');
  adapter.close();
  process.exit(1);
}

console.log(adapter.getLibInfo());
console.log('devices on the bus:');
for (const addr of adapter.getActiveDevices()) {
  const name = adapter.getDeviceOSDName(addr);
  const power = cec.powerStatusToString(adapter.getDevicePowerStatus(addr));
  console.log(`  ${cec.logicalAddressToString(addr)}: ${name} [${power}]`);
}

console.log('powering on the TV ...');
adapter.powerOnDevices(cec.CecLogicalAddress.TV);

// Keep the process alive to stream events; Ctrl-C to quit.
process.on('SIGINT', () => {
  console.log('\nclosing ...');
  adapter.close();
  process.exit(0);
});
