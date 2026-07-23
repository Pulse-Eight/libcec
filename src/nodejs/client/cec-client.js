#!/usr/bin/env node
'use strict';

/*
 * cec-client.js — an interactive test client for the libCEC Node.js binding.
 *
 * This is the Node.js counterpart of the C++ `cec-client` (src/cec-client): a
 * deliberately verbose REPL whose purpose is to *demonstrate* every part of the
 * binding's API and to *exercise* the library against real hardware. Clarity is
 * valued over brevity — each command handler is a small, self-contained example
 * of one API call, so this file doubles as living documentation.
 *
 * Usage:
 *   node client/cec-client.js [options] [port]
 *
 *   [port]                 the adapter to open (e.g. /dev/ttyACM0). Auto-detected
 *                          when omitted.
 *   -l, --list-devices     list the detected adapters and exit.
 *   -i, --info             print the libCEC build info and exit.
 *   -t, --type {p|r|t|a}   the device type to announce (playback/recording/
 *                          tuner/audio). Defaults to recording.
 *   -p, --port {n}         the HDMI port the adapter is connected to.
 *   -b, --base {addr}      the logical address the adapter is connected to.
 *   -d, --log-level {1-31} the initial log-level bitmask (see cectypes.h).
 *   -m, --monitor          start in monitor-only mode (claims no logical address).
 *   -h, --help             show this help and exit.
 *
 * Type `h` at the prompt for the interactive command list.
 */

const readline = require('readline');
const cec = require('..');

// ---------------------------------------------------------------------------
// argument parsing
// ---------------------------------------------------------------------------

function parseArgs(argv) {
  const opts = {
    port: null,
    deviceType: cec.CecDeviceType.RecordingDevice,
    hdmiPort: null,
    baseDevice: null,
    logMask: cec.CecLogLevel.Error | cec.CecLogLevel.Warning | cec.CecLogLevel.Notice,
    monitorOnly: false,
    listDevices: false,
    info: false,
    help: false,
  };
  const typeMap = {
    p: cec.CecDeviceType.PlaybackDevice,
    r: cec.CecDeviceType.RecordingDevice,
    t: cec.CecDeviceType.Tuner,
    a: cec.CecDeviceType.AudioSystem,
  };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    switch (a) {
      case '-l': case '--list-devices': opts.listDevices = true; break;
      case '-i': case '--info': opts.info = true; break;
      case '-h': case '--help': opts.help = true; break;
      case '-m': case '--monitor': opts.monitorOnly = true; break;
      case '-t': case '--type': opts.deviceType = typeMap[argv[++i]] ?? opts.deviceType; break;
      case '-p': case '--port': opts.hdmiPort = parseInt(argv[++i], 10); break;
      case '-b': case '--base': opts.baseDevice = parseInt(argv[++i], 10); break;
      case '-d': case '--log-level': opts.logMask = parseInt(argv[++i], 10); break;
      default:
        if (a.startsWith('-')) { console.error(`unknown option: ${a}`); process.exit(2); }
        opts.port = a; // a bare argument is the com port
    }
  }
  return opts;
}

// ---------------------------------------------------------------------------
// small formatting helpers
// ---------------------------------------------------------------------------

// libCEC reports every logical address as an enum; render it the way cec-client
// does, e.g. "TV (0)".
function fmtAddr(address) {
  return `${cec.logicalAddressToString(address)} (${address})`;
}

// a 16-bit physical address is a nibble per HDMI hop: 0x1234 -> "1.2.3.4".
function fmtPhysical(pa) {
  return [(pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, pa & 0xf].join('.');
}

// reverse-lookup a CecAdapterType value to its enum name (there is no native
// adapter-type-to-string helper).
function adapterTypeName(type) {
  const name = Object.keys(cec.CecAdapterType).find((k) => cec.CecAdapterType[k] === type);
  return name || `type ${type}`;
}

function fmtLogLevel(level) {
  switch (level) {
    case cec.CecLogLevel.Error: return 'ERROR';
    case cec.CecLogLevel.Warning: return 'WARNING';
    case cec.CecLogLevel.Notice: return 'NOTICE';
    case cec.CecLogLevel.Traffic: return 'TRAFFIC';
    case cec.CecLogLevel.Debug: return 'DEBUG';
    default: return `LVL${level}`;
  }
}

// parse a cec-client-style byte string, "1F:82:10:00" or "1f 82 10 00", into an
// array of bytes.
function parseBytes(text) {
  return text
    .trim()
    .split(/[\s:]+/)
    .filter((s) => s.length)
    .map((s) => parseInt(s, 16) & 0xff);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

function printHelp() {
  process.stdout.write(
`Interactive commands:
[tx] {bytes}              transfer bytes over the CEC line (e.g. tx 1F:82:10:00).
[txn] {bytes}             transfer bytes but don't wait for an ACK.
[on] {address}            power on the device with the given logical address.
[standby] {address}       put the device with the given address in standby.
[as]                      make the CEC adapter the active source.
[is]                      mark the CEC adapter as inactive source.
[sp] {physical address}   make the given physical address (e.g. 1.0.0.0) active.
[spl] {address}           make the given logical address active.
[osd] {addr} {string}     set an OSD message on the specified device.
[ver] {addr}              get the CEC version of the specified device.
[ven] {addr}              get the vendor id of the specified device.
[pow] {addr}              get the power status of the specified device.
[name] {addr}             get the OSD name of the specified device.
[poll] {addr}             poll the specified device.
[lad]                     list the active devices on the bus.
[scan]                    scan the CEC bus and display device info.
[self]                    show the logical addresses controlled by libCEC.
[volup]                   send a volume up command to the amp, if present.
[voldown]                 send a volume down command to the amp, if present.
[mute]                    send a mute command to the amp, if present.
[mon] {1|0}               enable or disable CEC bus monitoring.
[ping]                    send a ping command to the CEC adapter.
[log] {1-31}              change the log-level bitmask. see cectypes.h.
[r]                       reconnect to the CEC adapter.
[h] or [help]             show this help.
[q] or [quit]             quit.
`);
}

function main() {
  const opts = parseArgs(process.argv.slice(2));

  if (opts.help) {
    process.stdout.write(`${require('../package.json').description}\n\n`);
    printHelp();
    return;
  }

  // A bare adapter, used for the connection-less commands (list/info) and as the
  // handle the REPL drives. Passing the event callbacks up front demonstrates the
  // EventEmitter surface; every one maps to an ICECCallbacks slot in the addon.
  let logMask = opts.logMask;
  const adapter = new cec.CecAdapter({
    deviceName: 'CECNode',
    deviceType: opts.deviceType,
    monitorOnly: opts.monitorOnly,
    ...(opts.hdmiPort != null ? { hdmiPort: opts.hdmiPort } : {}),
    ...(opts.baseDevice != null ? { baseDevice: opts.baseDevice } : {}),
  });

  adapter.on('log', (m) => {
    if (m.level & logMask)
      console.log(`${fmtLogLevel(m.level)}[${m.time}]\t${m.message}`);
  });
  adapter.on('keyPress', (k) => {
    // duration 0 is the initial press; a follow-up with a duration is the release.
    if (k.duration === 0)
      console.log(`key pressed: ${cec.userControlKeyToString(k.keycode)} (${k.keycode})`);
  });
  adapter.on('command', (c) => {
    console.log(`>> ${fmtAddr(c.initiator)} -> ${fmtAddr(c.destination)}: ` +
                `${cec.opcodeToString(c.opcode)}` +
                (c.parameters.length ? ` [${c.parameters.map((b) => b.toString(16)).join(' ')}]` : ''));
  });
  adapter.on('sourceActivated', (address, activated) => {
    console.log(`source ${activated ? 'activated' : 'deactivated'}: ${fmtAddr(address)}`);
  });
  adapter.on('alert', (a) => console.log(`ALERT: ${a}`));
  adapter.on('configurationChanged', (config) => {
    if (config.logicalAddresses.length)
      console.log(`configuration changed; controlling ${config.logicalAddresses.map(fmtAddr).join(', ')}`);
  });

  // ---- connection-less commands -------------------------------------------

  const adapters = adapter.detectAdapters();

  if (opts.info) {
    console.log(adapter.getLibInfo());
    adapter.close();
    return;
  }

  if (opts.listDevices) {
    console.log(`found ${adapters.length} adapter(s):`);
    adapters.forEach((d, i) => {
      console.log(`device:              ${i + 1}`);
      console.log(`com port:            ${d.comName}`);
      console.log(`path:                ${d.path}`);
      console.log(`vendor id:           ${d.vendorId.toString(16)}`);
      console.log(`product id:          ${d.productId.toString(16)}`);
      console.log(`firmware version:    ${d.firmwareVersion}`);
      console.log(`type:                ${adapterTypeName(d.type)}\n`);
    });
    adapter.close();
    return;
  }

  // ---- open the adapter ----------------------------------------------------

  // open by the /dev node (strComName), as cec-client does — not the sysfs path.
  const port = opts.port || (adapters[0] && adapters[0].comName);
  if (!port) {
    console.error('no CEC adapter found; specify a port or connect one');
    adapter.close();
    process.exit(1);
  }

  console.log(`opening ${port} ...`);
  if (!adapter.open(port)) {
    console.error(`could not open a connection to ${port}`);
    adapter.close();
    process.exit(1);
  }
  console.log(adapter.getLibInfo());
  console.log("connection opened. type 'h' for help, 'q' to quit.");

  // ---- the REPL ------------------------------------------------------------

  const rl = readline.createInterface({ input: process.stdin, output: process.stdout, prompt: '' });
  rl.prompt();

  // Each command below is one focused example of the API. `addr()` reads a
  // logical address argument, defaulting to the TV.
  function addr(args, fallback = cec.CecLogicalAddress.TV) {
    return args.length ? parseInt(args[0], 10) : fallback;
  }

  function handle(line) {
    const parts = line.trim().split(/\s+/);
    const cmd = parts.shift();
    if (!cmd) return;

    switch (cmd) {
      case 'tx':
      case 'txn': {
        // Build a cec_command from raw bytes: the high/low nibble of the first
        // byte are the initiator/destination, the second byte is the opcode, the
        // rest are parameters. 'txn' asks not to wait for the ACK.
        const bytes = parseBytes(parts.join(' '));
        if (!bytes.length) { console.log('usage: tx <bytes>, e.g. tx 1F:82:10:00'); break; }
        const command = {
          initiator: (bytes[0] >> 4) & 0xf,
          destination: bytes[0] & 0xf,
          opcode: bytes.length > 1 ? bytes[1] : cec.CecOpcode.None,
          parameters: bytes.slice(2),
          transmitTimeout: cmd === 'txn' ? 0 : 1000,
        };
        console.log(adapter.transmit(command) ? 'transmit succeeded' : 'transmit failed');
        break;
      }

      case 'on':
        console.log(adapter.powerOnDevices(addr(parts, cec.CecLogicalAddress.Broadcast)) ? 'ok' : 'failed');
        break;

      case 'standby':
        console.log(adapter.standbyDevices(addr(parts, cec.CecLogicalAddress.Broadcast)) ? 'ok' : 'failed');
        break;

      case 'as':
        console.log(adapter.setActiveSource() ? 'now the active source' : 'failed');
        break;

      case 'is':
        console.log(adapter.setInactiveView() ? 'marked inactive' : 'failed');
        break;

      case 'sp': {
        // accept either "1.0.0.0" or a raw hex like "1000"
        const t = parts[0] || '';
        const pa = t.includes('.')
          ? t.split('.').reduce((acc, n) => (acc << 4) | (parseInt(n, 10) & 0xf), 0)
          : parseInt(t, 16);
        console.log(adapter.setStreamPathPhysical(pa) ? `stream path -> ${fmtPhysical(pa)}` : 'failed');
        break;
      }

      case 'spl':
        console.log(adapter.setStreamPathLogical(addr(parts)) ? 'ok' : 'failed');
        break;

      case 'osd': {
        const a = parseInt(parts.shift(), 10);
        const message = parts.join(' ');
        console.log(adapter.setOSDString(a, cec.CecDisplayControl.DisplayForDefaultTime, message) ? 'ok' : 'failed');
        break;
      }

      case 'ver':
        console.log(`CEC version ${fmtAddr(addr(parts))}: ${cec.cecVersionToString(adapter.getDeviceCecVersion(addr(parts)))}`);
        break;

      case 'ven':
        console.log(`vendor ${fmtAddr(addr(parts))}: ${cec.vendorIdToString(adapter.getDeviceVendorId(addr(parts)))}`);
        break;

      case 'pow':
        console.log(`power status ${fmtAddr(addr(parts))}: ${cec.powerStatusToString(adapter.getDevicePowerStatus(addr(parts)))}`);
        break;

      case 'name':
        console.log(`OSD name ${fmtAddr(addr(parts))}: ${adapter.getDeviceOSDName(addr(parts))}`);
        break;

      case 'poll':
        console.log(adapter.pollDevice(addr(parts)) ? 'device responded' : 'no response');
        break;

      case 'lad': {
        const active = adapter.getActiveDevices();
        console.log(`active devices: ${active.map(fmtAddr).join(', ') || '(none)'}`);
        break;
      }

      case 'scan':
        scanBus(adapter);
        break;

      case 'self': {
        const la = adapter.getLogicalAddresses();
        console.log(`controlled by libCEC: ${la.addresses.map(fmtAddr).join(', ') || '(none)'} ` +
                    `(primary ${fmtAddr(la.primary)})`);
        break;
      }

      case 'volup': console.log(`volume: ${adapter.volumeUp()}`); break;
      case 'voldown': console.log(`volume: ${adapter.volumeDown()}`); break;
      case 'mute': console.log(`audio status: ${adapter.muteAudio()}`); break;

      case 'mon': {
        const enable = parts[0] !== '0';
        console.log(adapter.switchMonitoring(enable) ? `monitoring ${enable ? 'on' : 'off'}` : 'failed');
        break;
      }

      case 'ping':
        console.log(adapter.pingAdapters() ? 'ping ok' : 'ping failed');
        break;

      case 'log':
        logMask = parseInt(parts[0], 10) || logMask;
        console.log(`log-level mask is now ${logMask}`);
        break;

      case 'r':
        console.log('reconnecting ...');
        adapter.close();
        // a fresh connection re-runs address allocation; re-open the same port.
        if (adapter.open(port)) console.log('reconnected'); else console.log('reconnect failed');
        break;

      case 'h': case 'help':
        printHelp();
        break;

      case 'q': case 'quit':
        rl.close();
        return;

      default:
        console.log(`unknown command '${cmd}'. type 'h' for help.`);
    }
  }

  rl.on('line', (line) => { try { handle(line); } catch (e) { console.error(`error: ${e.message}`); } rl.prompt(); });
  rl.on('close', () => { console.log('closing ...'); adapter.close(); process.exit(0); });
}

// A full bus scan, mirroring cec-client's `scan`: walk the active devices and
// query each attribute individually — a good end-to-end exercise of the query API.
function scanBus(adapter) {
  console.log('requesting CEC bus information ...');
  const devices = adapter.getActiveDevices();
  for (const address of devices) {
    const vendor = adapter.getDeviceVendorId(address);
    const physical = adapter.getDevicePhysicalAddress(address);
    const cecVersion = adapter.getDeviceCecVersion(address);
    const power = adapter.getDevicePowerStatus(address);
    const osdName = adapter.getDeviceOSDName(address);
    const active = adapter.isActiveSource(address);

    console.log(`device #${address}: ${cec.logicalAddressToString(address)}`);
    console.log(`address:       ${fmtPhysical(physical)}`);
    console.log(`active source: ${active ? 'yes' : 'no'}`);
    console.log(`vendor:        ${cec.vendorIdToString(vendor)}`);
    console.log(`osd string:    ${osdName}`);
    console.log(`CEC version:   ${cec.cecVersionToString(cecVersion)}`);
    console.log(`power status:  ${cec.powerStatusToString(power)}`);
    console.log('');
  }
  console.log(`scan complete: ${devices.length} device(s).`);
}

main();
