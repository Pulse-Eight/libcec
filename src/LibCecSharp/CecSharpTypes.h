#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "../lib/platform/threads/mutex.h"
#include <vcclr.h>
#include <msclr/marshal.h>
#include "../../include/cec.h"
#include <vector>

#using <System.dll>

namespace CecSharp
{
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

	public enum class CecAlert
	{
		ServiceDevice = 1
	};

	public enum class CecParameterType
	{
		ParameterTypeString = 1
	};

	public ref class CecParameter
	{
	public:
		CecParameter(CecParameterType type, System::String ^ strData)
		{
			Type = type;
			Data = strData;
		}

		property CecParameterType Type;
		property System::String ^ Data;
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
		SamsungReturn               = 0x91,
		Unknown
	};

	public enum class CecVendorId
	{
		Samsung   = 0x0000F0,
		LG        = 0x00E091,
		Panasonic = 0x008045,
		Pioneer   = 0x00E036,
		Onkyo     = 0x0009B0,
		Yamaha    = 0x00A0DE,
		Philips   = 0x00903E,
		Sony      = 0x080046,
		Toshiba   = 0x000039,
		Akai      = 0x0020C7,
		Benq      = 0x8065E9,
		Daewoo    = 0x009053,
		Grundig   = 0x00D0D5,
		Medion    = 0x000CB8,
		Sharp     = 0x08001F,
		Vizio     = 0x6B746D,
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

	public enum class CecClientVersion
	{
		VersionPre1_5 = 0,
		Version1_5_0  = 0x1500,
		Version1_5_1  = 0x1501,
		Version1_5_2  = 0x1502,
		Version1_5_3  = 0x1503,
		Version1_6_0  = 0x1600,
		Version1_6_1  = 0x1601,
		Version1_6_2  = 0x1602,
		Version1_6_3  = 0x1603,
		Version1_7_0  = 0x1700,
		Version1_7_1  = 0x1701,
		Version1_7_2  = 0x1702,
		Version1_8_0  = 0x1800,
		Version1_8_1  = 0x1801
	};

	public enum class CecServerVersion
	{
		VersionPre1_5 = 0,
		Version1_5_0  = 0x1500,
		Version1_5_1  = 0x1501,
		Version1_5_2  = 0x1502,
		Version1_5_3  = 0x1503,
		Version1_6_0  = 0x1600,
		Version1_6_1  = 0x1601,
		Version1_6_2  = 0x1602,
		Version1_6_3  = 0x1603,
		Version1_7_0  = 0x1700,
		Version1_7_1  = 0x1701,
		Version1_7_2  = 0x1702,
		Version1_8_0  = 0x1800,
		Version1_8_1  = 0x1801
	};

	public ref class CecAdapter
	{
	public:
		CecAdapter(System::String ^ strPath, System::String ^ strComPort)
		{
			Path = strPath;
			ComPort = strComPort;
		}

		property System::String ^ Path;
		property System::String ^ ComPort;
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
			Clear();
		}

		void Clear(void)
		{
			Primary = CecLogicalAddress::Unknown;
			for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
				Addresses[iPtr] = CecLogicalAddress::Unknown;
		}

		bool IsSet(CecLogicalAddress iAddress)
		{
			return Addresses[(unsigned int)iAddress] != CecLogicalAddress::Unknown;
		}

	  void Set(CecLogicalAddress iAddress)
		{
			Addresses[(unsigned int)iAddress] = iAddress;
			if (Primary == CecLogicalAddress::Unknown)
				Primary = iAddress;
		}

		property CecLogicalAddress          Primary;
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
		CecKeypress(CecUserControlCode iKeycode, unsigned int iDuration)
		{
			Keycode  = iKeycode;
			Duration = iDuration;
			Empty    = false;
		}

		CecKeypress(void)
		{
			Keycode  = CecUserControlCode::Unknown;
			Duration = 0;
			Empty    = true;
		}

		property bool               Empty;
		property CecUserControlCode Keycode;
		property unsigned int       Duration;
	};

	public ref class CecLogMessage
	{
	public:
		CecLogMessage(System::String ^ strMessage, CecLogLevel iLevel, int64_t iTime)
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

		property bool            Empty;
		property System::String ^Message;
		property CecLogLevel     Level;
		property int64_t         Time;
	};

	ref class CecCallbackMethods; //forward
	public ref class LibCECConfiguration
	{
	public:
		LibCECConfiguration(void)
		{
			DeviceName          = "";
			DeviceTypes         = gcnew CecDeviceTypeList();
			AutodetectAddress  = true;
			PhysicalAddress     = CEC_DEFAULT_PHYSICAL_ADDRESS;
			BaseDevice          = (CecLogicalAddress)CEC_DEFAULT_BASE_DEVICE;
			HDMIPort            = CEC_DEFAULT_HDMI_PORT;
			ClientVersion       = CecClientVersion::VersionPre1_5;
			ServerVersion       = CecServerVersion::VersionPre1_5;
			TvVendor            = CecVendorId::Unknown;

			GetSettingsFromROM  = false;
			UseTVMenuLanguage   = CEC_DEFAULT_SETTING_USE_TV_MENU_LANGUAGE == 1;
			ActivateSource      = CEC_DEFAULT_SETTING_ACTIVATE_SOURCE == 1;

			WakeDevices         = gcnew CecLogicalAddresses();
			if (CEC_DEFAULT_SETTING_ACTIVATE_SOURCE == 1)
				WakeDevices->Set(CecLogicalAddress::Tv);

			PowerOffDevices     = gcnew CecLogicalAddresses();
			if (CEC_DEFAULT_SETTING_POWER_OFF_SHUTDOWN == 1)
				PowerOffDevices->Set(CecLogicalAddress::Broadcast);

			PowerOffScreensaver = CEC_DEFAULT_SETTING_POWER_OFF_SCREENSAVER == 1;
			PowerOffOnStandby   = CEC_DEFAULT_SETTING_POWER_OFF_ON_STANDBY == 1;

			SendInactiveSource  = CEC_DEFAULT_SETTING_SEND_INACTIVE_SOURCE == 1;
			LogicalAddresses    = gcnew CecLogicalAddresses();
			FirmwareVersion     = 1;
			PowerOffDevicesOnStandby = CEC_DEFAULT_SETTING_POWER_OFF_DEVICES_STANDBY == 1;
			ShutdownOnStandby   = CEC_DEFAULT_SETTING_SHUTDOWN_ON_STANDBY == 1;
			DeviceLanguage      = "";
		}

		void SetCallbacks(CecCallbackMethods ^callbacks)
		{
			Callbacks = callbacks;
		}

		void Update(const CEC::libcec_configuration &config)
		{
			DeviceName = gcnew System::String(config.strDeviceName);

			for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
				DeviceTypes->Types[iPtr] = (CecDeviceType)config.deviceTypes.types[iPtr];

			AutodetectAddress  = config.bAutodetectAddress == 1;
			PhysicalAddress    = config.iPhysicalAddress;
			BaseDevice         = (CecLogicalAddress)config.baseDevice;
			HDMIPort           = config.iHDMIPort;
			ClientVersion      = (CecClientVersion)config.clientVersion;
			ServerVersion      = (CecServerVersion)config.serverVersion;
			TvVendor           = (CecVendorId)config.tvVendor;

			// player specific settings
			GetSettingsFromROM = config.bGetSettingsFromROM == 1;
			UseTVMenuLanguage = config.bUseTVMenuLanguage == 1;
			ActivateSource = config.bActivateSource == 1;

			WakeDevices->Clear();
			for (uint8_t iPtr = 0; iPtr <= 16; iPtr++)
				if (config.wakeDevices[iPtr])
					WakeDevices->Set((CecLogicalAddress)iPtr);

			PowerOffDevices->Clear();
			for (uint8_t iPtr = 0; iPtr <= 16; iPtr++)
				if (config.powerOffDevices[iPtr])
					PowerOffDevices->Set((CecLogicalAddress)iPtr);

			PowerOffScreensaver = config.bPowerOffScreensaver == 1;
			PowerOffOnStandby = config.bPowerOffOnStandby == 1;

			if (ServerVersion >= CecServerVersion::Version1_5_1)
				SendInactiveSource = config.bSendInactiveSource == 1;

			if (ServerVersion >= CecServerVersion::Version1_5_3)
			{
				LogicalAddresses->Clear();
				for (uint8_t iPtr = 0; iPtr <= 16; iPtr++)
					if (config.logicalAddresses[iPtr])
						LogicalAddresses->Set((CecLogicalAddress)iPtr);
			}

			if (ServerVersion >= CecServerVersion::Version1_6_0)
			{
				FirmwareVersion          = config.iFirmwareVersion;
				PowerOffDevicesOnStandby = config.bPowerOffDevicesOnStandby == 1;
				ShutdownOnStandby        = config.bShutdownOnStandby == 1;
			}

			if (ServerVersion >= CecServerVersion::Version1_6_2)
				DeviceLanguage = gcnew System::String(config.strDeviceLanguage);

			if (ServerVersion >= CecServerVersion::Version1_6_3)
			  MonitorOnlyClient = config.bMonitorOnly == 1;
		}

		property System::String ^     DeviceName;
		property CecDeviceTypeList ^  DeviceTypes;
		property bool                 AutodetectAddress;
		property uint16_t             PhysicalAddress;
		property CecLogicalAddress    BaseDevice;
		property uint8_t              HDMIPort;
		property CecClientVersion     ClientVersion;
		property CecServerVersion     ServerVersion;
		property CecVendorId          TvVendor;

		// player specific settings
		property bool                 GetSettingsFromROM;
		property bool                 UseTVMenuLanguage;
		property bool                 ActivateSource;
		property CecLogicalAddresses ^WakeDevices;
		property CecLogicalAddresses ^PowerOffDevices;
		property bool                 PowerOffScreensaver;
		property bool                 PowerOffOnStandby;
		property bool                 SendInactiveSource;
		property CecLogicalAddresses ^LogicalAddresses;
		property uint16_t             FirmwareVersion;
		property bool                 PowerOffDevicesOnStandby;
		property bool                 ShutdownOnStandby;
		property bool                 MonitorOnlyClient;
		property System::String ^     DeviceLanguage;
		property CecCallbackMethods ^ Callbacks;
	};

	// the callback methods are called by unmanaged code, so we need some delegates for this
	#pragma unmanaged
	// unmanaged callback methods
	typedef int (__stdcall *LOGCB)    (const CEC::cec_log_message &message);
	typedef int (__stdcall *KEYCB)    (const CEC::cec_keypress &key);
	typedef int (__stdcall *COMMANDCB)(const CEC::cec_command &command);
	typedef int (__stdcall *CONFIGCB) (const CEC::libcec_configuration &config);
	typedef int (__stdcall *ALERTCB)  (const CEC::libcec_alert, const CEC::libcec_parameter &data);
	typedef int (__stdcall *MENUCB)   (const CEC::cec_menu_state newVal);
	typedef void (__stdcall *ACTICB)  (const CEC::cec_logical_address logicalAddress, const uint8_t bActivated);

	typedef struct
	{
		LOGCB     logCB;
		KEYCB     keyCB;
		COMMANDCB commandCB;
		CONFIGCB  configCB;
		ALERTCB   alertCB;
		MENUCB    menuCB;
		ACTICB    sourceActivatedCB;
	} UnmanagedCecCallbacks;

	static PLATFORM::CMutex                   g_callbackMutex;
	static std::vector<UnmanagedCecCallbacks> g_unmanagedCallbacks;
  static CEC::ICECCallbacks                 g_cecCallbacks;

	int CecLogMessageCB(void *cbParam, const CEC::cec_log_message &message)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				return g_unmanagedCallbacks[iPtr].logCB(message);
		}
		return 0;
	}

	int CecKeyPressCB(void *cbParam, const CEC::cec_keypress &key)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				return g_unmanagedCallbacks[iPtr].keyCB(key);
		}
		return 0;
	}

	int CecCommandCB(void *cbParam, const CEC::cec_command &command)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				return g_unmanagedCallbacks[iPtr].commandCB(command);
		}
		return 0;
	}

  int CecConfigCB(void *cbParam, const CEC::libcec_configuration &config)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				return g_unmanagedCallbacks[iPtr].configCB(config);
		}
		return 0;
	}

	int CecAlertCB(void *cbParam, const CEC::libcec_alert alert, const CEC::libcec_parameter &data)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				return g_unmanagedCallbacks[iPtr].alertCB(alert, data);
		}
		return 0;
	}

	int CecMenuCB(void *cbParam, const CEC::cec_menu_state newVal)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				return g_unmanagedCallbacks[iPtr].menuCB(newVal);
		}
		return 0;
	}

	void CecSourceActivatedCB(void *cbParam, const CEC::cec_logical_address logicalAddress, const uint8_t bActivated)
	{
		if (cbParam)
		{
			size_t iPtr = (size_t)cbParam;
			PLATFORM::CLockObject lock(g_callbackMutex);
			if (iPtr >= 0 && iPtr < g_unmanagedCallbacks.size())
				g_unmanagedCallbacks[iPtr].sourceActivatedCB(logicalAddress, bActivated);
		}
	}

	#pragma managed
	// delegates for the unmanaged callback methods
	public delegate int  CecLogMessageManagedDelegate(const CEC::cec_log_message &);
	public delegate int  CecKeyPressManagedDelegate(const CEC::cec_keypress &);
	public delegate int  CecCommandManagedDelegate(const CEC::cec_command &);
	public delegate int  CecConfigManagedDelegate(const CEC::libcec_configuration &);
	public delegate int  CecAlertManagedDelegate(const CEC::libcec_alert, const CEC::libcec_parameter &);
	public delegate int  CecMenuManagedDelegate(const CEC::cec_menu_state newVal);
	public delegate void CecSourceActivatedManagedDelegate(const CEC::cec_logical_address logicalAddress, const uint8_t bActivated);

	void AssignCallbacks()
	{
		g_cecCallbacks.CBCecLogMessage           = CecLogMessageCB;
		g_cecCallbacks.CBCecKeyPress             = CecKeyPressCB;
		g_cecCallbacks.CBCecCommand              = CecCommandCB;
		g_cecCallbacks.CBCecConfigurationChanged = CecConfigCB;
		g_cecCallbacks.CBCecAlert                = CecAlertCB;
		g_cecCallbacks.CBCecMenuStateChanged     = CecMenuCB;
    g_cecCallbacks.CBCecSourceActivated      = CecSourceActivatedCB;
	}

	// callback method interface
	public ref class CecCallbackMethods
	{
	public:
    CecCallbackMethods(void)
    {
			m_iCallbackPtr = -1;
			AssignCallbacks();
      m_bHasCallbacks = false;
      m_bDelegatesCreated = false;
    }

	  ~CecCallbackMethods(void)
    {
      DestroyDelegates();
    }

		size_t GetCallbackPtr(void)
		{
			PLATFORM::CLockObject lock(g_callbackMutex);
			return m_iCallbackPtr;
		}

	protected:
   !CecCallbackMethods(void)
   {
     DestroyDelegates();
   }

	public:
		virtual void DisableCallbacks(void)
		{
			DestroyDelegates();
		}

		virtual bool EnableCallbacks(CecCallbackMethods ^ callbacks)
		{
      CreateDelegates();
			if (!m_bHasCallbacks)
			{
				m_bHasCallbacks = true;
				m_callbacks = callbacks;
				return true;
			}

			return false;
		}

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

		virtual int ConfigurationChanged(LibCECConfiguration ^ config)
		{
			return 0;
		}

		virtual int ReceiveAlert(CecAlert alert, CecParameter ^ data)
		{
			return 0;
		}

		virtual int ReceiveMenuStateChange(CecMenuState newVal)
		{
			return 0;
		}

		virtual void SourceActivated(CecLogicalAddress logicalAddress, bool bActivated)
		{
		}

	protected:
		// managed callback methods
		int CecLogMessageManaged(const CEC::cec_log_message &message)
		{
			int iReturn(0);
			if (m_bHasCallbacks)
				iReturn = m_callbacks->ReceiveLogMessage(gcnew CecLogMessage(gcnew System::String(message.message), (CecLogLevel)message.level, message.time));
			return iReturn;
		}

		int CecKeyPressManaged(const CEC::cec_keypress &key)
		{
			int iReturn(0);
			if (m_bHasCallbacks)
				iReturn = m_callbacks->ReceiveKeypress(gcnew CecKeypress((CecUserControlCode)key.keycode, key.duration));
			return iReturn;
		}

		int CecCommandManaged(const CEC::cec_command &command)
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

		int CecConfigManaged(const CEC::libcec_configuration &config)
		{
			int iReturn(0);
			if (m_bHasCallbacks)
			{
				LibCECConfiguration ^netConfig = gcnew LibCECConfiguration();
				netConfig->Update(config);
				iReturn = m_callbacks->ConfigurationChanged(netConfig);
			}
			return iReturn;
		}

		int CecAlertManaged(const CEC::libcec_alert alert, const CEC::libcec_parameter &data)
		{
			int iReturn(0);
			if (m_bHasCallbacks)
			{
				CecParameterType newType = (CecParameterType)data.paramType;
				if (newType == CecParameterType::ParameterTypeString)
				{
					System::String ^ newData = gcnew System::String((const char *)data.paramData, 0, 128);
					CecParameter ^ newParam = gcnew CecParameter(newType, newData);
				    iReturn = m_callbacks->ReceiveAlert((CecAlert)alert, newParam);
				}
			}
			return iReturn;
		}

		int CecMenuManaged(const CEC::cec_menu_state newVal)
		{
			int iReturn(0);
			if (m_bHasCallbacks)
			{
				iReturn = m_callbacks->ReceiveMenuStateChange((CecMenuState)newVal);
			}
			return iReturn;
		}

		void CecSourceActivatedManaged(const CEC::cec_logical_address logicalAddress, const uint8_t bActivated)
		{
			if (m_bHasCallbacks)
				m_callbacks->SourceActivated((CecLogicalAddress)logicalAddress, bActivated == 1);
		}

		void DestroyDelegates()
		{
      m_bHasCallbacks = false;
			if (m_bDelegatesCreated)
			{
        m_bDelegatesCreated = false;
				m_logMessageGCHandle.Free();
				m_keypressGCHandle.Free();
				m_commandGCHandle.Free();
				m_alertGCHandle.Free();
				m_menuGCHandle.Free();
				m_sourceActivatedGCHandle.Free();
			}
		}

    void CreateDelegates()
    {
      DestroyDelegates();

      if (!m_bDelegatesCreated)
      {
        msclr::interop::marshal_context ^ context = gcnew msclr::interop::marshal_context();

        // create the delegate method for the log message callback
        m_logMessageDelegate      = gcnew CecLogMessageManagedDelegate(this, &CecCallbackMethods::CecLogMessageManaged);
        m_logMessageGCHandle      = System::Runtime::InteropServices::GCHandle::Alloc(m_logMessageDelegate);
				m_logMessageCallback      = static_cast<LOGCB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_logMessageDelegate).ToPointer());

        // create the delegate method for the keypress callback
        m_keypressDelegate        = gcnew CecKeyPressManagedDelegate(this, &CecCallbackMethods::CecKeyPressManaged);
        m_keypressGCHandle        = System::Runtime::InteropServices::GCHandle::Alloc(m_keypressDelegate);
				m_keypressCallback        = static_cast<KEYCB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_keypressDelegate).ToPointer());

        // create the delegate method for the command callback
        m_commandDelegate         = gcnew CecCommandManagedDelegate(this, &CecCallbackMethods::CecCommandManaged);
        m_commandGCHandle         = System::Runtime::InteropServices::GCHandle::Alloc(m_commandDelegate);
				m_commandCallback         = static_cast<COMMANDCB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_commandDelegate).ToPointer());

        // create the delegate method for the configuration change callback
        m_configDelegate          = gcnew CecConfigManagedDelegate(this, &CecCallbackMethods::CecConfigManaged);
        m_configGCHandle          = System::Runtime::InteropServices::GCHandle::Alloc(m_configDelegate);
				m_configCallback          = static_cast<CONFIGCB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_configDelegate).ToPointer());

        // create the delegate method for the alert callback
        m_alertDelegate           = gcnew CecAlertManagedDelegate(this, &CecCallbackMethods::CecAlertManaged);
        m_alertGCHandle           = System::Runtime::InteropServices::GCHandle::Alloc(m_alertDelegate);
				m_alertCallback           = static_cast<ALERTCB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_alertDelegate).ToPointer());

        // create the delegate method for the menu callback
        m_menuDelegate            = gcnew CecMenuManagedDelegate(this, &CecCallbackMethods::CecMenuManaged);
        m_menuGCHandle            = System::Runtime::InteropServices::GCHandle::Alloc(m_menuDelegate);
				m_menuCallback            = static_cast<MENUCB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_menuDelegate).ToPointer());

			  // create the delegate method for the source activated callback
        m_sourceActivatedDelegate = gcnew CecSourceActivatedManagedDelegate(this, &CecCallbackMethods::CecSourceActivatedManaged);
        m_sourceActivatedGCHandle = System::Runtime::InteropServices::GCHandle::Alloc(m_sourceActivatedDelegate);
				m_sourceActivatedCallback = static_cast<ACTICB>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(m_sourceActivatedDelegate).ToPointer());

        delete context;

				UnmanagedCecCallbacks unmanagedCallbacks;
				unmanagedCallbacks.logCB             = m_logMessageCallback;
				unmanagedCallbacks.keyCB             = m_keypressCallback;
				unmanagedCallbacks.commandCB         = m_commandCallback;
				unmanagedCallbacks.configCB          = m_configCallback;
				unmanagedCallbacks.alertCB           = m_alertCallback;
				unmanagedCallbacks.menuCB            = m_menuCallback;
				unmanagedCallbacks.sourceActivatedCB = m_sourceActivatedCallback;

				PLATFORM::CLockObject lock(g_callbackMutex);
				g_unmanagedCallbacks.push_back(unmanagedCallbacks);
				m_iCallbackPtr = g_unmanagedCallbacks.size() - 1;
				m_bDelegatesCreated = true;
      }
    }

		CecLogMessageManagedDelegate ^                    m_logMessageDelegate;
		static System::Runtime::InteropServices::GCHandle m_logMessageGCHandle;
		LOGCB                                             m_logMessageCallback;

		CecKeyPressManagedDelegate ^                      m_keypressDelegate;
		static System::Runtime::InteropServices::GCHandle m_keypressGCHandle;
		KEYCB                                             m_keypressCallback;

		CecCommandManagedDelegate ^                       m_commandDelegate;
		static System::Runtime::InteropServices::GCHandle m_commandGCHandle;
		COMMANDCB                                         m_commandCallback;

	  CecConfigManagedDelegate ^                        m_configDelegate;
		static System::Runtime::InteropServices::GCHandle m_configGCHandle;
		CONFIGCB                                          m_configCallback;

		CecAlertManagedDelegate ^                         m_alertDelegate;
		static System::Runtime::InteropServices::GCHandle m_alertGCHandle;
		ALERTCB                                           m_alertCallback;

		CecMenuManagedDelegate ^                          m_menuDelegate;
		static System::Runtime::InteropServices::GCHandle m_menuGCHandle;
		MENUCB                                            m_menuCallback;

		CecSourceActivatedManagedDelegate ^               m_sourceActivatedDelegate;
		static System::Runtime::InteropServices::GCHandle m_sourceActivatedGCHandle;
		ACTICB                                            m_sourceActivatedCallback;

		CecCallbackMethods ^ m_callbacks;
	  bool                 m_bHasCallbacks;
    bool                 m_bDelegatesCreated;
		size_t               m_iCallbackPtr;
	};
}
