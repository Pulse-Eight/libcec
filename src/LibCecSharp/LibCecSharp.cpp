/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
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

#include <windows.h>
#include <vcclr.h>
#include <msclr/marshal.h>
#include <cec.h>
#using <System.dll>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace CEC;
using namespace msclr::interop;

public enum class CecDeviceType
{
  Tv              = 0,
  RecordingDevice = 1,
  Reserved        = 2,
  Tuner           = 3,
  PlaybackDevice  = 4,
  AudioSystem     = 5
};

public enum class CecLogLevel
{
  None    = 0,
  Error   = 1,
  Warning = 2,
  Notice  = 4,
  Traffic = 8,
  Debug   = 16,
  All     = 31
};

public enum class CecLogicalAddress
{
  Unknown          = -1, //not a valid logical address
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
};

public enum class CecPowerStatus
{
  On                      = 0x00,
  Standby                 = 0x01,
  InTransitionStandbyToOn = 0x02,
  InTransitionOnToStandby = 0x03,
  Unknown                 = 0x99
};

public enum class CecVersion
{
  Unknown = 0x00,
  V1_2    = 0x01,
  V1_2A   = 0x02,
  V1_3    = 0x03,
  V1_3A   = 0x04,
  V1_4    = 0x05
};

public enum class CecDisplayControl
{
  DisplayForDefaultTime = 0x00,
  DisplayUntilCleared   = 0x40,
  ClearPreviousMessage  = 0x80,
  ReservedForFutureUse  = 0xC0
};

public enum class CecMenuState
{
  Activated   = 0,
  Deactivated = 1
};

public enum class CecDeckControlMode
{
  SkipForwardWind   = 1,
  SkipReverseRewind = 2,
  Stop              = 3,
  Eject             = 4
};

public enum class CecDeckInfo
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
};

public enum class CecUserControlCode
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
  Unknown
};

public enum class CecVendorId
{
  Samsung   = 0x00F0,
  LG        = 0xE091,
  Panasonic = 0x8045,
  Pioneer   = 0xE036,
  Onkyo     = 0x09B0,
  Yamaha    = 0xA0DE,
  Philips   = 0x903E,
  Unknown   = 0
};

public enum class CecAudioStatus
{
  MuteStatusMask      = 0x80,
  VolumeStatusMask    = 0x7F,
  VolumeMin           = 0x00,
  VolumeMax           = 0x64,
  VolumeStatusUnknown = 0x7F
};

public enum class CecOpcode
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
  /* when this opcode is set, no opcode will be sent to the device. this is one of the reserved numbers */
  None                          = 0xFD
};

public enum class CecSystemAudioStatus
{
  Off = 0,
  On  = 1
};

public ref class CecAdapter
{
public:
  CecAdapter(String ^ strPath, String ^ strComPort)
  {
    Path = strPath;
    ComPort = strComPort;
  }

  property String ^ Path;
  property String ^ ComPort;
};

public ref class CecDeviceTypeList
{
public:
  CecDeviceTypeList(void)
  {
    Types = gcnew array<CecDeviceType>(5);
    for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
      Types[iPtr] = CecDeviceType::Reserved;
  }

  property array<CecDeviceType> ^ Types;
};

public ref class CecLogicalAddresses
{
public:
  CecLogicalAddresses(void)
  {
    Addresses = gcnew array<CecLogicalAddress>(16);
    for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
      Addresses[iPtr] = CecLogicalAddress::Unregistered;
  }

  bool IsSet(CecLogicalAddress iAddress)
  {
    return Addresses[(unsigned int)iAddress] != CecLogicalAddress::Unregistered;
  }

  property array<CecLogicalAddress> ^ Addresses;
};

public ref class CecDatapacket
{
public:
  CecDatapacket(void)
  {
    Data = gcnew array<uint8_t>(100);
    Size = 0;
  }

  void PushBack(uint8_t data)
  {
    if (Size < 100)
    {
      Data[Size] = data;
      Size++;
    }
  }

  property array<uint8_t> ^ Data;
  property uint8_t          Size;
};

public ref class CecCommand
{
public:
  CecCommand(CecLogicalAddress iInitiator, CecLogicalAddress iDestination, bool bAck, bool bEom, CecOpcode iOpcode, int32_t iTransmitTimeout)
  {
    Initiator       = iInitiator;
    Destination     = iDestination;
    Ack             = bAck;
    Eom             = bEom;
    Opcode          = iOpcode;
    OpcodeSet       = true;
    TransmitTimeout = iTransmitTimeout;
    Parameters      = gcnew CecDatapacket;
    Empty           = false;
  }

  CecCommand(void)
  {
    Initiator       = CecLogicalAddress::Unknown;
    Destination     = CecLogicalAddress::Unknown;
    Ack             = false;
    Eom             = false;
    Opcode          = CecOpcode::None;
    OpcodeSet       = false;
    TransmitTimeout = 0;
    Parameters      = gcnew CecDatapacket;
    Empty           = true;
  }

  void PushBack(uint8_t data)
  {
    if (Initiator == CecLogicalAddress::Unknown && Destination == CecLogicalAddress::Unknown)
    {
      Initiator   = (CecLogicalAddress) (data >> 4);
      Destination = (CecLogicalAddress) (data & 0xF);
    }
    else if (!OpcodeSet)
    {
      OpcodeSet = true;
      Opcode    = (CecOpcode)data;
    }
    else
    {
      Parameters->PushBack(data);
    }
  }

  property bool               Empty;
  property CecLogicalAddress  Initiator;
  property CecLogicalAddress  Destination;
  property bool               Ack;
  property bool               Eom;
  property CecOpcode          Opcode;
  property CecDatapacket ^    Parameters;
  property bool               OpcodeSet;
  property int32_t            TransmitTimeout;
};

public ref class CecKeypress
{
public:
  CecKeypress(int iKeycode, unsigned int iDuration)
  {
    Keycode  = iKeycode;
    Duration = iDuration;
    Empty    = false;
  }

  CecKeypress(void)
  {
    Keycode  = 0;
    Duration = 0;
    Empty    = true;
  }

  property bool         Empty;
  property int          Keycode;
  property unsigned int Duration;
};

public ref class CecLogMessage
{
public:
  CecLogMessage(String ^ strMessage, CecLogLevel iLevel, int64_t iTime)
  {
    Message = strMessage;
    Level   = iLevel;
    Time    = iTime;
    Empty   = false;
  }

  CecLogMessage(void)
  {
    Message = "";
    Level   = CecLogLevel::None;
    Time    = 0;
    Empty   = true;
  }

  property bool        Empty;
  property String ^    Message;
  property CecLogLevel Level;
  property int64_t     Time;
};

public ref class CecCallbackMethods
{
public:
  virtual int ReceiveLogMessage(CecLogMessage ^ message)
  {
    return 0;
  }

  virtual int ReceiveKeypress(CecKeypress ^ key)
  {
    return 0;
  }

  virtual int ReceiveCommand(CecCommand ^ command)
  {
    return 0;
  }
};

#pragma unmanaged
// unmanaged callback methods
typedef int (__stdcall *LOGCB)    (const cec_log_message &message);
typedef int (__stdcall *KEYCB)    (const cec_keypress &key);
typedef int (__stdcall *COMMANDCB)(const cec_command &command);

static LOGCB         g_logCB;
static KEYCB         g_keyCB;
static COMMANDCB     g_commandCB;
static ICECCallbacks g_cecCallbacks;

int CecLogMessageCB(void *cbParam, const cec_log_message &message)
{
  if (g_logCB)
    return g_logCB(message);
  return 0;
}

int CecKeyPressCB(void *cbParam, const cec_keypress &key)
{
  if (g_keyCB)
    return g_keyCB(key);
  return 0;
}

int CecCommandCB(void *cbParam, const cec_command &command)
{
  if (g_commandCB)
    return g_commandCB(command);
  return 0;
}

#pragma managed
// delegates for the unmanaged callback methods
public delegate int CecLogMessageManagedDelegate(const cec_log_message &);
public delegate int CecKeyPressManagedDelegate(const cec_keypress &);
public delegate int CecCommandManagedDelegate(const cec_command &);

public ref class LibCecSharp
{
public:
  LibCecSharp(String ^ strDeviceName, CecDeviceTypeList ^ deviceTypes)
  {
    marshal_context ^ context = gcnew marshal_context();
    m_bHasCallbacks = false;
    const char* strDeviceNameC = context->marshal_as<const char*>(strDeviceName);

    cec_device_type_list types;
    for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
      types.types[iPtr] = (cec_device_type)deviceTypes->Types[iPtr];
    m_libCec = (ICECAdapter *) CECInit(strDeviceNameC, types);

    // create the delegate method for the log message callback
    m_logMessageDelegate           = gcnew CecLogMessageManagedDelegate(this, &LibCecSharp::CecLogMessageManaged);
    m_logMessageGCHandle           = GCHandle::Alloc(m_logMessageDelegate);
    g_logCB                        = static_cast<LOGCB>(Marshal::GetFunctionPointerForDelegate(m_logMessageDelegate).ToPointer());
    g_cecCallbacks.CBCecLogMessage = CecLogMessageCB;

    // create the delegate method for the keypress callback
    m_keypressDelegate           = gcnew CecKeyPressManagedDelegate(this, &LibCecSharp::CecKeyPressManaged);
    m_keypressGCHandle           = GCHandle::Alloc(m_keypressDelegate);
    g_keyCB                      = static_cast<KEYCB>(Marshal::GetFunctionPointerForDelegate(m_keypressDelegate).ToPointer());
    g_cecCallbacks.CBCecKeyPress = CecKeyPressCB;

    // create the delegate method for the command callback
    m_commandDelegate           = gcnew CecCommandManagedDelegate(this, &LibCecSharp::CecCommandManaged);
    m_commandGCHandle           = GCHandle::Alloc(m_commandDelegate);
    g_commandCB                 = static_cast<COMMANDCB>(Marshal::GetFunctionPointerForDelegate(m_commandDelegate).ToPointer());
    g_cecCallbacks.CBCecCommand = CecCommandCB;

    delete context;
  }
   
   ~LibCecSharp(void)
   {
     CECDestroy(m_libCec);
     DestroyDelegates();
     m_libCec = NULL;
   }

protected:
   !LibCecSharp(void)
   {
     CECDestroy(m_libCec);
     DestroyDelegates();
     m_libCec = NULL;
   }

public:
  array<CecAdapter ^> ^ FindAdapters(String ^ path)
  {
    cec_adapter *devices = new cec_adapter[10];

    marshal_context ^ context = gcnew marshal_context();
    const char* strPathC = path->Length > 0 ? context->marshal_as<const char*>(path) : NULL;

    uint8_t iDevicesFound = m_libCec->FindAdapters(devices, 10, NULL);

    array<CecAdapter ^> ^ adapters = gcnew array<CecAdapter ^>(iDevicesFound);
    for (unsigned int iPtr = 0; iPtr < iDevicesFound; iPtr++)
      adapters[iPtr] = gcnew CecAdapter(gcnew String(devices[iPtr].path), gcnew String(devices[iPtr].comm));

    delete devices;
    delete context;
    return adapters;
  }

  bool Open(String ^ strPort, int iTimeoutMs)
  {
    marshal_context ^ context = gcnew marshal_context();
    const char* strPortC = context->marshal_as<const char*>(strPort);
    bool bReturn = m_libCec->Open(strPortC, iTimeoutMs);
    delete context;
    return bReturn;
  }

  void Close(void)
  {
    m_libCec->Close();
  }

  bool EnableCallbacks(CecCallbackMethods ^ callbacks)
  {
    if (m_libCec && !m_bHasCallbacks)
    {
      m_bHasCallbacks = true;
      m_callbacks = callbacks;
      return m_libCec->EnableCallbacks(NULL, &g_cecCallbacks);
    }

    return false;
  }

  bool PingAdapter(void)
  {
    return m_libCec->PingAdapter();
  }

  bool StartBootloader(void)
  {
    return m_libCec->StartBootloader();
  }

  int GetMinLibVersion(void)
  {
    return m_libCec->GetMinLibVersion();
  }

  int GetLibVersionMajor(void)
  {
    return m_libCec->GetLibVersionMajor();
  }

  int GetLibVersionMinor(void)
  {
    return m_libCec->GetLibVersionMinor();
  }

  CecLogMessage ^ GetNextLogMessage(void)
  {
    cec_log_message msg;
    if (m_libCec->GetNextLogMessage(&msg))
    {
      return gcnew CecLogMessage(gcnew String(msg.message), (CecLogLevel)msg.level, msg.time);
    }

    return gcnew CecLogMessage();
  }

  CecKeypress ^ GetNextKeypress(void)
  {
    cec_keypress key;
    if (m_libCec->GetNextKeypress(&key))
    {
      return gcnew CecKeypress(key.keycode, key.duration);
    }

    return gcnew CecKeypress();
  }

  CecCommand ^ GetNextCommand(void)
  {
    cec_command command;
    if (m_libCec->GetNextCommand(&command))
    {
      CecCommand ^ retVal = gcnew CecCommand((CecLogicalAddress)command.initiator, (CecLogicalAddress)command.destination, command.ack == 1 ? true : false, command.eom == 1 ? true : false, (CecOpcode)command.opcode, command.transmit_timeout);
      for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
        retVal->Parameters->PushBack(command.parameters[iPtr]);
      return retVal;
    }

    return gcnew CecCommand();
  }

  bool Transmit(CecCommand ^ command)
  {
    cec_command ccommand;
    cec_command::Format(ccommand, (cec_logical_address)command->Initiator, (cec_logical_address)command->Destination, (cec_opcode)command->Opcode);
    ccommand.transmit_timeout = command->TransmitTimeout;
    ccommand.eom              = command->Eom;
    ccommand.ack              = command->Ack;
    for (unsigned int iPtr = 0; iPtr < command->Parameters->Size; iPtr++)
      ccommand.parameters.PushBack(command->Parameters->Data[iPtr]);

    return m_libCec->Transmit(ccommand);
  }

  bool SetLogicalAddress(CecLogicalAddress logicalAddress)
  {
    return m_libCec->SetLogicalAddress((cec_logical_address) logicalAddress);
  }

  bool SetPhysicalAddress(int16_t physicalAddress)
  {
    return m_libCec->SetPhysicalAddress(physicalAddress);
  }

  bool PowerOnDevices(CecLogicalAddress logicalAddress)
  {
    return m_libCec->PowerOnDevices((cec_logical_address) logicalAddress);
  }

  bool StandbyDevices(CecLogicalAddress logicalAddress)
  {
    return m_libCec->StandbyDevices((cec_logical_address) logicalAddress);
  }

  bool PollDevice(CecLogicalAddress logicalAddress)
  {
    return m_libCec->PollDevice((cec_logical_address) logicalAddress);
  }

  bool SetActiveSource(CecDeviceType type)
  {
    return m_libCec->SetActiveSource((cec_device_type) type);
  }

  bool SetDeckControlMode(CecDeckControlMode mode, bool sendUpdate)
  {
    return m_libCec->SetDeckControlMode((cec_deck_control_mode) mode, sendUpdate);
  }

  bool SetDeckInfo(CecDeckInfo info, bool sendUpdate)
  {
    return m_libCec->SetDeckInfo((cec_deck_info) info, sendUpdate);
  }

  bool SetInactiveView(void)
  {
    return m_libCec->SetInactiveView();
  }

  bool SetMenuState(CecMenuState state, bool sendUpdate)
  {
    return m_libCec->SetMenuState((cec_menu_state) state, sendUpdate);
  }

  bool SetOSDString(CecLogicalAddress logicalAddress, CecDisplayControl duration, String ^ message)
  {
    marshal_context ^ context = gcnew marshal_context();
    const char* strMessageC = context->marshal_as<const char*>(message);

    bool bReturn = m_libCec->SetOSDString((cec_logical_address) logicalAddress, (cec_display_control) duration, strMessageC);

    delete context;
    return bReturn;
  }

  bool SwitchMonitoring(bool enable)
  {
    return m_libCec->SwitchMonitoring(enable);
  }

  CecVersion GetDeviceCecVersion(CecLogicalAddress logicalAddress)
  {
    return (CecVersion) m_libCec->GetDeviceCecVersion((cec_logical_address) logicalAddress);
  }

  String ^ GetDeviceMenuLanguage(CecLogicalAddress logicalAddress)
  {
    cec_menu_language lang;
    if (m_libCec->GetDeviceMenuLanguage((cec_logical_address) logicalAddress, &lang))
    {
      return gcnew String(lang.language);
    }

    return gcnew String("");
  }

  CecVendorId GetDeviceVendorId(CecLogicalAddress logicalAddress)
  {
    return (CecVendorId)m_libCec->GetDeviceVendorId((cec_logical_address) logicalAddress);
  }

  CecPowerStatus GetDevicePowerStatus(CecLogicalAddress logicalAddress)
  {
    return (CecPowerStatus) m_libCec->GetDevicePowerStatus((cec_logical_address) logicalAddress);
  }

  CecLogicalAddresses ^ GetActiveDevices(void)
  {
    CecLogicalAddresses ^ retVal = gcnew CecLogicalAddresses();
    unsigned int iDevices = 0;

    cec_logical_addresses activeDevices = m_libCec->GetActiveDevices();

    for (uint8_t iPtr = 0; iPtr < 16; iPtr++)
      if (activeDevices[iPtr])
        retVal->Addresses[iDevices++] = (CecLogicalAddress)iPtr;

    return retVal;
  }

  bool IsActiveDevice(CecLogicalAddress logicalAddress)
  {
    return m_libCec->IsActiveDevice((cec_logical_address)logicalAddress);
  }

  bool IsActiveDeviceType(CecDeviceType type)
  {
    return m_libCec->IsActiveDeviceType((cec_device_type)type);
  }

  bool SetHDMIPort(CecLogicalAddress address, uint8_t port)
  {
    return m_libCec->SetHDMIPort((cec_logical_address)address, port);
  }

  uint8_t VolumeUp(bool wait)
  {
    return m_libCec->VolumeUp(wait);
  }

  uint8_t VolumeDown(bool wait)
  {
    return m_libCec->VolumeDown(wait);
  }

  uint8_t MuteAudio(bool wait)
  {
    return m_libCec->MuteAudio(wait);
  }

  bool SendKeypress(CecLogicalAddress destination, CecUserControlCode key, bool wait)
  {
    return m_libCec->SendKeypress((cec_logical_address)destination, (cec_user_control_code)key, wait);
  }

  bool SendKeyRelease(CecLogicalAddress destination, bool wait)
  {
    return m_libCec->SendKeyRelease((cec_logical_address)destination, wait);
  }

  String ^ GetDeviceOSDName(CecLogicalAddress logicalAddress)
  {
    cec_osd_name osd = m_libCec->GetDeviceOSDName((cec_logical_address) logicalAddress);
    return gcnew String(osd.name);
  }

  CecLogicalAddress GetActiveSource()
  {
    return (CecLogicalAddress)m_libCec->GetActiveSource();
  }

  bool IsActiveSource(CecLogicalAddress logicalAddress)
  {
    return m_libCec->IsActiveSource((cec_logical_address)logicalAddress);
  }

  uint16_t GetDevicePhysicalAddress(CecLogicalAddress iAddress)
  {
    return m_libCec->GetDevicePhysicalAddress((cec_logical_address)iAddress);
  }

  String ^ ToString(CecLogicalAddress iAddress)
  {
    const char *retVal = m_libCec->ToString((cec_logical_address)iAddress);
    return gcnew String(retVal);
  }

  String ^ ToString(CecVendorId iVendorId)
  {
    const char *retVal = m_libCec->ToString((cec_vendor_id)iVendorId);
    return gcnew String(retVal);
  }
  
  String ^ ToString(CecVersion iVersion)
  {
    const char *retVal = m_libCec->ToString((cec_version)iVersion);
    return gcnew String(retVal);
  }
  
  String ^ ToString(CecPowerStatus iState)
  {
    const char *retVal = m_libCec->ToString((cec_power_status)iState);
    return gcnew String(retVal);
  }

  String ^ ToString(CecMenuState iState)
  {
    const char *retVal = m_libCec->ToString((cec_menu_state)iState);
    return gcnew String(retVal);
  }

  String ^ ToString(CecDeckControlMode iMode)
  {
    const char *retVal = m_libCec->ToString((cec_deck_control_mode)iMode);
    return gcnew String(retVal);
  }

  String ^ ToString(CecDeckInfo status)
  {
    const char *retVal = m_libCec->ToString((cec_deck_info)status);
    return gcnew String(retVal);
  }

  String ^ ToString(CecOpcode opcode)
  {
    const char *retVal = m_libCec->ToString((cec_opcode)opcode);
    return gcnew String(retVal);
  }

  String ^ ToString(CecSystemAudioStatus mode)
  {
    const char *retVal = m_libCec->ToString((cec_system_audio_status)mode);
    return gcnew String(retVal);
  }

  String ^ ToString(CecAudioStatus status)
  {
    const char *retVal = m_libCec->ToString((cec_audio_status)status);
    return gcnew String(retVal);
  }

private:
  void DestroyDelegates()
  {
    m_logMessageGCHandle.Free();
    m_keypressGCHandle.Free();
    m_commandGCHandle.Free();
  }

  // managed callback methods
  int CecLogMessageManaged(const cec_log_message &message)
  {
    int iReturn(0);
    if (m_bHasCallbacks)
      iReturn = m_callbacks->ReceiveLogMessage(gcnew CecLogMessage(gcnew String(message.message), (CecLogLevel)message.level, message.time));
    return iReturn;
  }

  int CecKeyPressManaged(const cec_keypress &key)
  {
    int iReturn(0);
    if (m_bHasCallbacks)
      iReturn = m_callbacks->ReceiveKeypress(gcnew CecKeypress(key.keycode, key.duration));
    return iReturn;
  }

  int CecCommandManaged(const cec_command &command)
  {
    int iReturn(0);
    if (m_bHasCallbacks)
    {
      CecCommand ^ newCommand = gcnew CecCommand((CecLogicalAddress)command.initiator, (CecLogicalAddress)command.destination, command.ack == 1 ? true : false, command.eom == 1 ? true : false, (CecOpcode)command.opcode, command.transmit_timeout);
      for (uint8_t iPtr = 0; iPtr < command.parameters.size; iPtr++)
        newCommand->Parameters->PushBack(command.parameters[iPtr]);
      iReturn = m_callbacks->ReceiveCommand(newCommand);
    }
    return iReturn;
  }

  ICECAdapter *        m_libCec;
  CecCallbackMethods ^ m_callbacks;
  bool                 m_bHasCallbacks;

  CecLogMessageManagedDelegate ^ m_logMessageDelegate;
  static GCHandle                m_logMessageGCHandle;
  LOGCB                          m_logMessageCallback;

  CecKeyPressManagedDelegate ^   m_keypressDelegate;
  static GCHandle                m_keypressGCHandle;
  KEYCB                          m_keypressCallback;

  CecCommandManagedDelegate ^    m_commandDelegate;
  static GCHandle                m_commandGCHandle;
  COMMANDCB                      m_commandCallback;
};
