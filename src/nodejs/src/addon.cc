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

// Native Node.js (N-API) binding for libCEC. It wraps the C API in
// include/cecc.h the same way src/dotnetlib binds it for .NET: one thin object
// (CecAdapter) owns a libcec_connection_t and forwards the synchronous calls.
//
// The interesting part is the callbacks. libCEC invokes the ICECCallbacks table
// from its own worker thread, where touching a JS value would crash. Each C
// trampoline therefore copies its payload and hands it to a
// Napi::ThreadSafeFunction, which re-enters JavaScript on the event loop.

#include <napi.h>
// cec.h first: compiled as C++, cecc.h typedefs libcec_connection_t as
// CEC::ICECAdapter*, which cec.h declares (and it pulls in cectypes.h).
#include <libcec/cec.h>
#include <libcec/cecc.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace CEC;

namespace {

// ---- payloads copied off the CEC worker thread ------------------------------

struct LogData    { std::string message; int level; int64_t time; };
struct KeyData    { int keycode; uint32_t duration; };
struct CommandData{ cec_command cmd; };
struct SourceData { int logicalAddress; bool activated; };
struct AlertData  { int alert; };
struct MenuData   { int state; };
struct ConfigData { libcec_configuration cfg; };

// Build a JS object from a (heap) cec_command copy.
Napi::Object CommandToJs(Napi::Env env, const cec_command& c) {
  Napi::Object o = Napi::Object::New(env);
  o.Set("initiator", Napi::Number::New(env, c.initiator));
  o.Set("destination", Napi::Number::New(env, c.destination));
  o.Set("ack", Napi::Boolean::New(env, c.ack != 0));
  o.Set("eom", Napi::Boolean::New(env, c.eom != 0));
  o.Set("opcode", Napi::Number::New(env, c.opcode));
  o.Set("opcodeSet", Napi::Boolean::New(env, c.opcode_set != 0));
  o.Set("transmitTimeout", Napi::Number::New(env, c.transmit_timeout));
  Napi::Array params = Napi::Array::New(env, c.parameters.size);
  for (uint8_t i = 0; i < c.parameters.size; ++i)
    params.Set(i, Napi::Number::New(env, c.parameters.data[i]));
  o.Set("parameters", params);
  return o;
}

// Read a fixed char buffer that may not be NUL-terminated (stop at NUL or cap).
std::string ReadFixed(const char* src, size_t cap) {
  size_t n = 0;
  while (n < cap && src[n] != '\0') ++n;
  return std::string(src, n);
}

// Build a JS object from a (heap) libcec_configuration copy. A useful subset of
// the struct; mirrors the field names of the .NET LibCECConfiguration surface.
Napi::Object ConfigToJs(Napi::Env env, const libcec_configuration& c) {
  Napi::Object o = Napi::Object::New(env);
  o.Set("deviceName", Napi::String::New(env, ReadFixed(c.strDeviceName, LIBCEC_OSD_NAME_SIZE)));
  Napi::Array types = Napi::Array::New(env);
  uint32_t ti = 0;
  for (int i = 0; i < 5; ++i)
    if (c.deviceTypes.types[i] != CEC_DEVICE_TYPE_RESERVED || i == 0)
      types.Set(ti++, Napi::Number::New(env, c.deviceTypes.types[i]));
  o.Set("deviceTypes", types);
  o.Set("physicalAddress", Napi::Number::New(env, c.iPhysicalAddress));
  o.Set("baseDevice", Napi::Number::New(env, c.baseDevice));
  o.Set("hdmiPort", Napi::Number::New(env, c.iHDMIPort));
  o.Set("tvVendor", Napi::Number::New(env, c.tvVendor));
  o.Set("clientVersion", Napi::Number::New(env, c.clientVersion));
  o.Set("serverVersion", Napi::Number::New(env, c.serverVersion));
  o.Set("firmwareVersion", Napi::Number::New(env, c.iFirmwareVersion));
  o.Set("firmwareBuildDate", Napi::Number::New(env, c.iFirmwareBuildDate));
  o.Set("cecVersion", Napi::Number::New(env, c.cecVersion));
  o.Set("adapterType", Napi::Number::New(env, c.adapterType));
  o.Set("deviceLanguage", Napi::String::New(env, ReadFixed(c.strDeviceLanguage, 3)));
  o.Set("autodetectAddress", Napi::Boolean::New(env, c.bAutodetectAddress != 0));
  o.Set("activateSource", Napi::Boolean::New(env, c.bActivateSource != 0));
  o.Set("monitorOnly", Napi::Boolean::New(env, c.bMonitorOnly != 0));
  Napi::Array la = Napi::Array::New(env);
  uint32_t li = 0;
  for (int i = 0; i < 16; ++i)
    if (c.logicalAddresses.addresses[i])
      la.Set(li++, Napi::Number::New(env, i));
  o.Set("logicalAddresses", la);
  return o;
}

// Copy a JS string into a fixed char buffer, always NUL-terminated.
void CopyString(char* dst, size_t cap, const std::string& src) {
  std::memset(dst, 0, cap);
  if (cap == 0) return;
  size_t n = src.size() < cap - 1 ? src.size() : cap - 1;
  std::memcpy(dst, src.data(), n);
}

}  // namespace

class CecAdapter : public Napi::ObjectWrap<CecAdapter> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  explicit CecAdapter(const Napi::CallbackInfo& info);
  ~CecAdapter() override;

 private:
  libcec_connection_t connection_ = nullptr;
  ICECCallbacks       callbacks_;   // address handed to libCEC; must outlive the connection

  Napi::ThreadSafeFunction tsfnLog_;
  Napi::ThreadSafeFunction tsfnKey_;
  Napi::ThreadSafeFunction tsfnCommand_;
  Napi::ThreadSafeFunction tsfnSource_;
  Napi::ThreadSafeFunction tsfnAlert_;
  Napi::ThreadSafeFunction tsfnConfig_;
  Napi::ThreadSafeFunction tsfnMenu_;
  Napi::ThreadSafeFunction tsfnCmdHandler_;

  void DoClose();

  // lifecycle
  Napi::Value Open(const Napi::CallbackInfo& info);
  void        Close(const Napi::CallbackInfo& info);

  // control
  Napi::Value Transmit(const Napi::CallbackInfo& info);
  Napi::Value PowerOnDevices(const Napi::CallbackInfo& info);
  Napi::Value StandbyDevices(const Napi::CallbackInfo& info);
  Napi::Value SetActiveSource(const Napi::CallbackInfo& info);
  Napi::Value SetInactiveView(const Napi::CallbackInfo& info);
  Napi::Value VolumeUp(const Napi::CallbackInfo& info);
  Napi::Value VolumeDown(const Napi::CallbackInfo& info);
  Napi::Value MuteAudio(const Napi::CallbackInfo& info);
  Napi::Value SendKeypress(const Napi::CallbackInfo& info);
  Napi::Value SendKeyRelease(const Napi::CallbackInfo& info);
  Napi::Value SetOSDString(const Napi::CallbackInfo& info);

  // queries
  Napi::Value GetActiveSource(const Napi::CallbackInfo& info);
  Napi::Value IsActiveSource(const Napi::CallbackInfo& info);
  Napi::Value GetDevicePowerStatus(const Napi::CallbackInfo& info);
  Napi::Value GetDeviceVendorId(const Napi::CallbackInfo& info);
  Napi::Value GetDevicePhysicalAddress(const Napi::CallbackInfo& info);
  Napi::Value GetDeviceCecVersion(const Napi::CallbackInfo& info);
  Napi::Value GetDeviceOSDName(const Napi::CallbackInfo& info);
  Napi::Value GetActiveDevices(const Napi::CallbackInfo& info);
  Napi::Value GetLogicalAddresses(const Napi::CallbackInfo& info);
  Napi::Value SwitchMonitoring(const Napi::CallbackInfo& info);
  Napi::Value SetStreamPathPhysical(const Napi::CallbackInfo& info);
  Napi::Value SetStreamPathLogical(const Napi::CallbackInfo& info);
  Napi::Value PollDevice(const Napi::CallbackInfo& info);
  Napi::Value RescanDevices(const Napi::CallbackInfo& info);
  Napi::Value PingAdapters(const Napi::CallbackInfo& info);
  Napi::Value DetectAdapters(const Napi::CallbackInfo& info);
  Napi::Value GetLibInfo(const Napi::CallbackInfo& info);

  // trampolines invoked on libCEC's callback thread
  static void CB_Log(void* p, const cec_log_message* m);
  static void CB_Key(void* p, const cec_keypress* k);
  static void CB_Command(void* p, const cec_command* c);
  static void CB_Source(void* p, const cec_logical_address a, const uint8_t activated);
  static void CB_Alert(void* p, const libcec_alert type, const libcec_parameter param);
  static void CB_Config(void* p, const libcec_configuration* c);
  // These return an int (1 = "handled, suppress default processing"). We only
  // observe, so they post to the event loop and return 0 synchronously; libCEC
  // then keeps its default handling. Blocking its callback thread on the event
  // loop to honour a JS return would race the 1000ms upstream timeout.
  static int  CB_MenuState(void* p, const cec_menu_state state);
  static int  CB_CommandHandler(void* p, const cec_command* c);
};

// -----------------------------------------------------------------------------
// trampolines (CEC worker thread -> event loop)
// -----------------------------------------------------------------------------

void CecAdapter::CB_Log(void* p, const cec_log_message* m) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnLog_) return;
  auto* d = new LogData{ m->message ? std::string(m->message) : std::string(),
                         static_cast<int>(m->level), static_cast<int64_t>(m->time) };
  auto st = self->tsfnLog_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, LogData* d) {
    Napi::Object o = Napi::Object::New(env);
    o.Set("message", Napi::String::New(env, d->message));
    o.Set("level", Napi::Number::New(env, d->level));
    o.Set("time", Napi::Number::New(env, static_cast<double>(d->time)));
    cb.Call({ o });
    delete d;
  });
  if (st != napi_ok) delete d;
}

void CecAdapter::CB_Key(void* p, const cec_keypress* k) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnKey_) return;
  auto* d = new KeyData{ static_cast<int>(k->keycode), k->duration };
  auto st = self->tsfnKey_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, KeyData* d) {
    Napi::Object o = Napi::Object::New(env);
    o.Set("keycode", Napi::Number::New(env, d->keycode));
    o.Set("duration", Napi::Number::New(env, d->duration));
    cb.Call({ o });
    delete d;
  });
  if (st != napi_ok) delete d;
}

void CecAdapter::CB_Command(void* p, const cec_command* c) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnCommand_) return;
  auto* d = new CommandData{ *c };
  auto st = self->tsfnCommand_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, CommandData* d) {
    cb.Call({ CommandToJs(env, d->cmd) });
    delete d;
  });
  if (st != napi_ok) delete d;
}

void CecAdapter::CB_Source(void* p, const cec_logical_address a, const uint8_t activated) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnSource_) return;
  auto* d = new SourceData{ static_cast<int>(a), activated != 0 };
  auto st = self->tsfnSource_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, SourceData* d) {
    cb.Call({ Napi::Number::New(env, d->logicalAddress), Napi::Boolean::New(env, d->activated) });
    delete d;
  });
  if (st != napi_ok) delete d;
}

void CecAdapter::CB_Alert(void* p, const libcec_alert type, const libcec_parameter /*param*/) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnAlert_) return;
  auto* d = new AlertData{ static_cast<int>(type) };
  auto st = self->tsfnAlert_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, AlertData* d) {
    cb.Call({ Napi::Number::New(env, d->alert) });
    delete d;
  });
  if (st != napi_ok) delete d;
}

void CecAdapter::CB_Config(void* p, const libcec_configuration* c) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnConfig_) return;
  auto* d = new ConfigData{ *c };
  auto st = self->tsfnConfig_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, ConfigData* d) {
    cb.Call({ ConfigToJs(env, d->cfg) });
    delete d;
  });
  if (st != napi_ok) delete d;
}

int CecAdapter::CB_MenuState(void* p, const cec_menu_state state) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnMenu_) return 0;
  auto* d = new MenuData{ static_cast<int>(state) };
  auto st = self->tsfnMenu_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, MenuData* d) {
    cb.Call({ Napi::Number::New(env, d->state) });
    delete d;
  });
  if (st != napi_ok) delete d;
  return 0;  // not handled; libCEC keeps its default menu handling
}

int CecAdapter::CB_CommandHandler(void* p, const cec_command* c) {
  auto* self = static_cast<CecAdapter*>(p);
  if (!self || !self->tsfnCmdHandler_) return 0;
  auto* d = new CommandData{ *c };
  auto st = self->tsfnCmdHandler_.NonBlockingCall(d, [](Napi::Env env, Napi::Function cb, CommandData* d) {
    cb.Call({ CommandToJs(env, d->cmd) });
    delete d;
  });
  if (st != napi_ok) delete d;
  return 0;  // not handled; libCEC keeps its default command processing
}

// -----------------------------------------------------------------------------
// construction / teardown
// -----------------------------------------------------------------------------

CecAdapter::CecAdapter(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<CecAdapter>(info) {
  Napi::Env env = info.Env();
  Napi::Object opts = (info.Length() > 0 && info[0].IsObject())
                          ? info[0].As<Napi::Object>()
                          : Napi::Object::New(env);

  libcec_configuration config;
  libcec_clear_configuration(&config);   // seeds clientVersion + read-only defaults

  std::string name = opts.Has("deviceName") ? opts.Get("deviceName").ToString().Utf8Value()
                                             : std::string("CECNode");
  CopyString(config.strDeviceName, LIBCEC_OSD_NAME_SIZE, name);

  int deviceType = opts.Has("deviceType") ? opts.Get("deviceType").ToNumber().Int32Value()
                                          : CEC_DEVICE_TYPE_RECORDING_DEVICE;
  config.deviceTypes.types[0] = static_cast<cec_device_type>(deviceType);

  if (opts.Has("physicalAddress"))
    config.iPhysicalAddress = static_cast<uint16_t>(opts.Get("physicalAddress").ToNumber().Uint32Value());
  if (opts.Has("baseDevice"))
    config.baseDevice = static_cast<cec_logical_address>(opts.Get("baseDevice").ToNumber().Int32Value());
  if (opts.Has("hdmiPort"))
    config.iHDMIPort = static_cast<uint8_t>(opts.Get("hdmiPort").ToNumber().Uint32Value());
  if (opts.Has("activateSource"))
    config.bActivateSource = opts.Get("activateSource").ToBoolean().Value() ? 1 : 0;
  if (opts.Has("monitorOnly"))
    config.bMonitorOnly = opts.Get("monitorOnly").ToBoolean().Value() ? 1 : 0;
  if (opts.Has("cecVersion"))
    config.cecVersion = static_cast<cec_version>(opts.Get("cecVersion").ToNumber().Int32Value());

  // Wire only the callbacks the caller supplied, so libCEC never invokes an
  // unimplemented slot. Each becomes a ThreadSafeFunction keyed to that JS fn.
  std::memset(&callbacks_, 0, sizeof(callbacks_));
  auto maybeTsfn = [&](const char* key, const char* resource,
                       Napi::ThreadSafeFunction& out, void* trampoline) {
    if (opts.Has(key) && opts.Get(key).IsFunction()) {
      out = Napi::ThreadSafeFunction::New(env, opts.Get(key).As<Napi::Function>(),
                                          resource, 0 /*unlimited*/, 1 /*threads*/);
      return trampoline;
    }
    return static_cast<void*>(nullptr);
  };

  if (void* fn = maybeTsfn("onLogMessage", "cecLog", tsfnLog_, (void*)&CecAdapter::CB_Log))
    callbacks_.logMessage = reinterpret_cast<decltype(callbacks_.logMessage)>(fn);
  if (void* fn = maybeTsfn("onKeyPress", "cecKey", tsfnKey_, (void*)&CecAdapter::CB_Key))
    callbacks_.keyPress = reinterpret_cast<decltype(callbacks_.keyPress)>(fn);
  if (void* fn = maybeTsfn("onCommand", "cecCommand", tsfnCommand_, (void*)&CecAdapter::CB_Command))
    callbacks_.commandReceived = reinterpret_cast<decltype(callbacks_.commandReceived)>(fn);
  if (void* fn = maybeTsfn("onSourceActivated", "cecSource", tsfnSource_, (void*)&CecAdapter::CB_Source))
    callbacks_.sourceActivated = reinterpret_cast<decltype(callbacks_.sourceActivated)>(fn);
  if (void* fn = maybeTsfn("onAlert", "cecAlert", tsfnAlert_, (void*)&CecAdapter::CB_Alert))
    callbacks_.alert = reinterpret_cast<decltype(callbacks_.alert)>(fn);
  if (void* fn = maybeTsfn("onConfigurationChanged", "cecConfig", tsfnConfig_, (void*)&CecAdapter::CB_Config))
    callbacks_.configurationChanged = reinterpret_cast<decltype(callbacks_.configurationChanged)>(fn);
  if (void* fn = maybeTsfn("onMenuStateChanged", "cecMenu", tsfnMenu_, (void*)&CecAdapter::CB_MenuState))
    callbacks_.menuStateChanged = reinterpret_cast<decltype(callbacks_.menuStateChanged)>(fn);
  if (void* fn = maybeTsfn("onCommandHandler", "cecCmdHandler", tsfnCmdHandler_, (void*)&CecAdapter::CB_CommandHandler))
    callbacks_.commandHandler = reinterpret_cast<decltype(callbacks_.commandHandler)>(fn);

  config.callbackParam = this;
  config.callbacks = &callbacks_;

  connection_ = libcec_initialise(&config);
  if (!connection_) {
    // Nothing to release: TSFNs will finalize when this object is collected.
    Napi::Error::New(env, "libcec_initialise failed").ThrowAsJavaScriptException();
    return;
  }
  libcec_init_video_standalone(connection_);
}

CecAdapter::~CecAdapter() {
  DoClose();
}

void CecAdapter::DoClose() {
  if (connection_) {
    libcec_close(connection_);
    libcec_destroy(connection_);   // stops the worker thread; no callbacks after this
    connection_ = nullptr;
  }
  // Release lets the event loop stop once queued calls have drained.
  if (tsfnLog_)        { tsfnLog_.Release();        tsfnLog_ = Napi::ThreadSafeFunction(); }
  if (tsfnKey_)        { tsfnKey_.Release();        tsfnKey_ = Napi::ThreadSafeFunction(); }
  if (tsfnCommand_)    { tsfnCommand_.Release();    tsfnCommand_ = Napi::ThreadSafeFunction(); }
  if (tsfnSource_)     { tsfnSource_.Release();     tsfnSource_ = Napi::ThreadSafeFunction(); }
  if (tsfnAlert_)      { tsfnAlert_.Release();      tsfnAlert_ = Napi::ThreadSafeFunction(); }
  if (tsfnConfig_)     { tsfnConfig_.Release();     tsfnConfig_ = Napi::ThreadSafeFunction(); }
  if (tsfnMenu_)       { tsfnMenu_.Release();       tsfnMenu_ = Napi::ThreadSafeFunction(); }
  if (tsfnCmdHandler_) { tsfnCmdHandler_.Release(); tsfnCmdHandler_ = Napi::ThreadSafeFunction(); }
}

// -----------------------------------------------------------------------------
// method helpers
// -----------------------------------------------------------------------------

#define REQUIRE_CONN(env)                                                    \
  if (!connection_) {                                                        \
    Napi::Error::New((env), "adapter is closed").ThrowAsJavaScriptException();\
    return (env).Undefined();                                               \
  }

static int ArgInt(const Napi::CallbackInfo& info, size_t i, int fallback) {
  return (info.Length() > i && info[i].IsNumber()) ? info[i].As<Napi::Number>().Int32Value() : fallback;
}
static bool ArgBool(const Napi::CallbackInfo& info, size_t i, bool fallback) {
  return (info.Length() > i && info[i].IsBoolean()) ? info[i].As<Napi::Boolean>().Value() : fallback;
}

// -----------------------------------------------------------------------------
// lifecycle
// -----------------------------------------------------------------------------

Napi::Value CecAdapter::Open(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  std::string port = (info.Length() > 0 && info[0].IsString()) ? info[0].As<Napi::String>().Utf8Value()
                                                               : std::string();
  uint32_t timeout = (info.Length() > 1 && info[1].IsNumber())
                         ? info[1].As<Napi::Number>().Uint32Value() : 10000;
  int ok = libcec_open(connection_, port.empty() ? nullptr : port.c_str(), timeout);
  return Napi::Boolean::New(env, ok != 0);
}

void CecAdapter::Close(const Napi::CallbackInfo& /*info*/) {
  DoClose();
}

// -----------------------------------------------------------------------------
// control
// -----------------------------------------------------------------------------

Napi::Value CecAdapter::Transmit(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  if (info.Length() < 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "transmit(command) expects an object").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  Napi::Object o = info[0].As<Napi::Object>();
  cec_command cmd;
  std::memset(&cmd, 0, sizeof(cmd));
  cmd.initiator   = static_cast<cec_logical_address>(o.Has("initiator") ? o.Get("initiator").ToNumber().Int32Value() : CECDEVICE_UNKNOWN);
  cmd.destination = static_cast<cec_logical_address>(o.Has("destination") ? o.Get("destination").ToNumber().Int32Value() : CECDEVICE_BROADCAST);
  cmd.opcode      = static_cast<cec_opcode>(o.Get("opcode").ToNumber().Int32Value());
  cmd.opcode_set  = 1;
  cmd.eom         = 1;
  cmd.transmit_timeout = o.Has("transmitTimeout") ? o.Get("transmitTimeout").ToNumber().Int32Value() : 1000;
  if (o.Has("parameters") && o.Get("parameters").IsArray()) {
    Napi::Array params = o.Get("parameters").As<Napi::Array>();
    uint32_t n = params.Length();
    if (n > 64) n = 64;
    for (uint32_t i = 0; i < n; ++i)
      cmd.parameters.data[i] = static_cast<uint8_t>(params.Get(i).ToNumber().Uint32Value());
    cmd.parameters.size = static_cast<uint8_t>(n);
  }
  return Napi::Boolean::New(env, libcec_transmit(connection_, &cmd) != 0);
}

Napi::Value CecAdapter::PowerOnDevices(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_BROADCAST);
  return Napi::Boolean::New(env, libcec_power_on_devices(connection_, static_cast<cec_logical_address>(addr)) != 0);
}

Napi::Value CecAdapter::StandbyDevices(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_BROADCAST);
  return Napi::Boolean::New(env, libcec_standby_devices(connection_, static_cast<cec_logical_address>(addr)) != 0);
}

Napi::Value CecAdapter::SetActiveSource(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int type = ArgInt(info, 0, CEC_DEVICE_TYPE_RESERVED);
  return Napi::Boolean::New(env, libcec_set_active_source(connection_, static_cast<cec_device_type>(type)) != 0);
}

Napi::Value CecAdapter::SetInactiveView(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Boolean::New(env, libcec_set_inactive_view(connection_) != 0);
}

Napi::Value CecAdapter::VolumeUp(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Number::New(env, libcec_volume_up(connection_, ArgBool(info, 0, true) ? 1 : 0));
}

Napi::Value CecAdapter::VolumeDown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Number::New(env, libcec_volume_down(connection_, ArgBool(info, 0, true) ? 1 : 0));
}

Napi::Value CecAdapter::MuteAudio(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Number::New(env, libcec_mute_audio(connection_, ArgBool(info, 0, true) ? 1 : 0));
}

Napi::Value CecAdapter::SendKeypress(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int dest = ArgInt(info, 0, CECDEVICE_TV);
  int key  = ArgInt(info, 1, 0);
  bool wait = ArgBool(info, 2, true);
  return Napi::Boolean::New(env, libcec_send_keypress(connection_, static_cast<cec_logical_address>(dest),
                                                      static_cast<cec_user_control_code>(key), wait ? 1 : 0) != 0);
}

Napi::Value CecAdapter::SendKeyRelease(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int dest = ArgInt(info, 0, CECDEVICE_TV);
  bool wait = ArgBool(info, 1, true);
  return Napi::Boolean::New(env, libcec_send_key_release(connection_, static_cast<cec_logical_address>(dest), wait ? 1 : 0) != 0);
}

Napi::Value CecAdapter::SetOSDString(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int dest = ArgInt(info, 0, CECDEVICE_TV);
  int duration = ArgInt(info, 1, CEC_DISPLAY_CONTROL_DISPLAY_FOR_DEFAULT_TIME);
  std::string msg = (info.Length() > 2 && info[2].IsString()) ? info[2].As<Napi::String>().Utf8Value() : std::string();
  return Napi::Boolean::New(env, libcec_set_osd_string(connection_, static_cast<cec_logical_address>(dest),
                                                       static_cast<cec_display_control>(duration), msg.c_str()) != 0);
}

// -----------------------------------------------------------------------------
// queries
// -----------------------------------------------------------------------------

Napi::Value CecAdapter::GetActiveSource(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Number::New(env, libcec_get_active_source(connection_));
}

Napi::Value CecAdapter::IsActiveSource(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_UNKNOWN);
  return Napi::Boolean::New(env, libcec_is_active_source(connection_, static_cast<cec_logical_address>(addr)) != 0);
}

Napi::Value CecAdapter::GetDevicePowerStatus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  return Napi::Number::New(env, libcec_get_device_power_status(connection_, static_cast<cec_logical_address>(addr)));
}

Napi::Value CecAdapter::GetDeviceVendorId(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  return Napi::Number::New(env, libcec_get_device_vendor_id(connection_, static_cast<cec_logical_address>(addr)));
}

Napi::Value CecAdapter::GetDevicePhysicalAddress(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  return Napi::Number::New(env, libcec_get_device_physical_address(connection_, static_cast<cec_logical_address>(addr)));
}

Napi::Value CecAdapter::GetDeviceCecVersion(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  return Napi::Number::New(env, libcec_get_device_cec_version(connection_, static_cast<cec_logical_address>(addr)));
}

Napi::Value CecAdapter::GetDeviceOSDName(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  cec_osd_name name;
  std::memset(name, 0, sizeof(name));
  libcec_get_device_osd_name(connection_, static_cast<cec_logical_address>(addr), name);
  return Napi::String::New(env, name);
}

Napi::Value CecAdapter::GetActiveDevices(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  cec_logical_addresses addrs = libcec_get_active_devices(connection_);
  Napi::Array out = Napi::Array::New(env);
  uint32_t idx = 0;
  for (int i = 0; i < 16; ++i)
    if (addrs.addresses[i])
      out.Set(idx++, Napi::Number::New(env, i));
  return out;
}

Napi::Value CecAdapter::GetLogicalAddresses(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  cec_logical_addresses addrs = libcec_get_logical_addresses(connection_);
  Napi::Object o = Napi::Object::New(env);
  o.Set("primary", Napi::Number::New(env, addrs.primary));
  Napi::Array arr = Napi::Array::New(env);
  uint32_t idx = 0;
  for (int i = 0; i < 16; ++i)
    if (addrs.addresses[i])
      arr.Set(idx++, Napi::Number::New(env, i));
  o.Set("addresses", arr);
  return o;
}

Napi::Value CecAdapter::SwitchMonitoring(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Boolean::New(env, libcec_switch_monitoring(connection_, ArgBool(info, 0, true) ? 1 : 0) != 0);
}

Napi::Value CecAdapter::SetStreamPathPhysical(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  uint16_t addr = static_cast<uint16_t>(ArgInt(info, 0, 0));
  return Napi::Boolean::New(env, libcec_set_stream_path_physical(connection_, addr) != 0);
}

Napi::Value CecAdapter::SetStreamPathLogical(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  return Napi::Boolean::New(env, libcec_set_stream_path_logical(connection_, static_cast<cec_logical_address>(addr)) != 0);
}

Napi::Value CecAdapter::PollDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  int addr = ArgInt(info, 0, CECDEVICE_TV);
  return Napi::Boolean::New(env, libcec_poll_device(connection_, static_cast<cec_logical_address>(addr)) != 0);
}

Napi::Value CecAdapter::RescanDevices(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  libcec_rescan_devices(connection_);
  return env.Undefined();
}

Napi::Value CecAdapter::PingAdapters(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  return Napi::Boolean::New(env, libcec_ping_adapters(connection_) != 0);
}

Napi::Value CecAdapter::DetectAdapters(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  cec_adapter_descriptor list[16];
  std::memset(list, 0, sizeof(list));
  int8_t n = libcec_detect_adapters(connection_, list, 16, nullptr, 0 /*quickScan off*/);
  Napi::Array out = Napi::Array::New(env);
  for (int8_t i = 0; i < n; ++i) {
    Napi::Object o = Napi::Object::New(env);
    o.Set("path", Napi::String::New(env, list[i].strComPath));
    o.Set("comName", Napi::String::New(env, list[i].strComName));
    o.Set("vendorId", Napi::Number::New(env, list[i].iVendorId));
    o.Set("productId", Napi::Number::New(env, list[i].iProductId));
    o.Set("firmwareVersion", Napi::Number::New(env, list[i].iFirmwareVersion));
    o.Set("physicalAddress", Napi::Number::New(env, list[i].iPhysicalAddress));
    o.Set("type", Napi::Number::New(env, list[i].adapterType));
    out.Set(static_cast<uint32_t>(i), o);
  }
  return out;
}

Napi::Value CecAdapter::GetLibInfo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  REQUIRE_CONN(env);
  const char* s = libcec_get_lib_info(connection_);
  return Napi::String::New(env, s ? s : "");
}

// -----------------------------------------------------------------------------
// module-level enum-to-string helpers (no live connection needed)
// -----------------------------------------------------------------------------

namespace {

using ToStringFn = void (*)(int, char*, size_t);

Napi::String CallToString(const Napi::CallbackInfo& info, ToStringFn fn) {
  Napi::Env env = info.Env();
  int value = info.Length() > 0 ? info[0].ToNumber().Int32Value() : 0;
  char buf[64];
  std::memset(buf, 0, sizeof(buf));
  fn(value, buf, sizeof(buf));
  return Napi::String::New(env, buf);
}

Napi::String CecVersionToString(const Napi::CallbackInfo& info) {
  return CallToString(info, [](int v, char* b, size_t n) { libcec_cec_version_to_string(static_cast<cec_version>(v), b, n); });
}
Napi::String PowerStatusToString(const Napi::CallbackInfo& info) {
  return CallToString(info, [](int v, char* b, size_t n) { libcec_power_status_to_string(static_cast<cec_power_status>(v), b, n); });
}
Napi::String LogicalAddressToString(const Napi::CallbackInfo& info) {
  return CallToString(info, [](int v, char* b, size_t n) { libcec_logical_address_to_string(static_cast<cec_logical_address>(v), b, n); });
}
Napi::String VendorIdToString(const Napi::CallbackInfo& info) {
  return CallToString(info, [](int v, char* b, size_t n) { libcec_vendor_id_to_string(static_cast<cec_vendor_id>(v), b, n); });
}
Napi::String OpcodeToString(const Napi::CallbackInfo& info) {
  return CallToString(info, [](int v, char* b, size_t n) { libcec_opcode_to_string(static_cast<cec_opcode>(v), b, n); });
}
Napi::String UserControlKeyToString(const Napi::CallbackInfo& info) {
  return CallToString(info, [](int v, char* b, size_t n) { libcec_user_control_key_to_string(static_cast<cec_user_control_code>(v), b, n); });
}

}  // namespace

// -----------------------------------------------------------------------------
// registration
// -----------------------------------------------------------------------------

Napi::Object CecAdapter::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function ctor = DefineClass(env, "CecAdapter", {
    InstanceMethod("open", &CecAdapter::Open),
    InstanceMethod("close", &CecAdapter::Close),
    InstanceMethod("transmit", &CecAdapter::Transmit),
    InstanceMethod("powerOnDevices", &CecAdapter::PowerOnDevices),
    InstanceMethod("standbyDevices", &CecAdapter::StandbyDevices),
    InstanceMethod("setActiveSource", &CecAdapter::SetActiveSource),
    InstanceMethod("setInactiveView", &CecAdapter::SetInactiveView),
    InstanceMethod("volumeUp", &CecAdapter::VolumeUp),
    InstanceMethod("volumeDown", &CecAdapter::VolumeDown),
    InstanceMethod("muteAudio", &CecAdapter::MuteAudio),
    InstanceMethod("sendKeypress", &CecAdapter::SendKeypress),
    InstanceMethod("sendKeyRelease", &CecAdapter::SendKeyRelease),
    InstanceMethod("setOSDString", &CecAdapter::SetOSDString),
    InstanceMethod("getActiveSource", &CecAdapter::GetActiveSource),
    InstanceMethod("isActiveSource", &CecAdapter::IsActiveSource),
    InstanceMethod("getDevicePowerStatus", &CecAdapter::GetDevicePowerStatus),
    InstanceMethod("getDeviceVendorId", &CecAdapter::GetDeviceVendorId),
    InstanceMethod("getDevicePhysicalAddress", &CecAdapter::GetDevicePhysicalAddress),
    InstanceMethod("getDeviceCecVersion", &CecAdapter::GetDeviceCecVersion),
    InstanceMethod("getDeviceOSDName", &CecAdapter::GetDeviceOSDName),
    InstanceMethod("getActiveDevices", &CecAdapter::GetActiveDevices),
    InstanceMethod("getLogicalAddresses", &CecAdapter::GetLogicalAddresses),
    InstanceMethod("switchMonitoring", &CecAdapter::SwitchMonitoring),
    InstanceMethod("setStreamPathPhysical", &CecAdapter::SetStreamPathPhysical),
    InstanceMethod("setStreamPathLogical", &CecAdapter::SetStreamPathLogical),
    InstanceMethod("pollDevice", &CecAdapter::PollDevice),
    InstanceMethod("rescanDevices", &CecAdapter::RescanDevices),
    InstanceMethod("pingAdapters", &CecAdapter::PingAdapters),
    InstanceMethod("detectAdapters", &CecAdapter::DetectAdapters),
    InstanceMethod("getLibInfo", &CecAdapter::GetLibInfo),
  });

  exports.Set("CecAdapter", ctor);
  exports.Set("cecVersionToString", Napi::Function::New(env, CecVersionToString));
  exports.Set("powerStatusToString", Napi::Function::New(env, PowerStatusToString));
  exports.Set("logicalAddressToString", Napi::Function::New(env, LogicalAddressToString));
  exports.Set("vendorIdToString", Napi::Function::New(env, VendorIdToString));
  exports.Set("opcodeToString", Napi::Function::New(env, OpcodeToString));
  exports.Set("userControlKeyToString", Napi::Function::New(env, UserControlKeyToString));
  return exports;
}

static Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return CecAdapter::Init(env, exports);
}

NODE_API_MODULE(cec_native, InitAll)
