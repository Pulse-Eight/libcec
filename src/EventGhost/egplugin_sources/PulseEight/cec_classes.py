# -*- coding: utf-8 -*-
# This file is part of the libCEC(R) library.
#
# libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.
# All rights reserved.
# libCEC(R) is an original work, containing original code.
#
# libCEC(R) is a trademark of Pulse-Eight Limited.
#
# This program is dual-licensed; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#
#
# Alternatively, you can license this library under a commercial license,
# please contact Pulse-Eight Licensing for more information.
#
# For more information contact:
# Pulse-Eight Licensing       <license@pulse-eight.com>
#     http://www.pulse-eight.com/
#     http://www.pulse-eight.net/
#
#
# The code contained within this file also falls under the GNU license of
# EventGhost
#
# Copyright © 2005-2016 EventGhost Project <http://www.eventghost.org/>
#
# EventGhost is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 2 of the License, or (at your option)
# any later version.
#
# EventGhost is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with EventGhost. If not, see <http://www.gnu.org/licenses/>.

from . import cec
import threading
import eg

CEC_LOG_CONSTANTS = {
    cec.CEC_LOG_ERROR: "ERROR:   ",
    cec.CEC_LOG_WARNING: "WARNING: ",
    cec.CEC_LOG_NOTICE: "NOTICE:  ",
    cec.CEC_LOG_TRAFFIC: "TRAFFIC: ",
    cec.CEC_LOG_DEBUG: "DEBUG:   ",
    cec.CEC_LOG_ALL: "ALL:   "
}

CEC_POWER_CONSTANTS = {
    cec.CEC_POWER_STATUS_ON: True,
    cec.CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY: False,
    cec.CEC_POWER_STATUS_STANDBY: False,
    cec.CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON: True,
    cec.CEC_POWER_STATUS_UNKNOWN: None
}

_CONTROL_CODES = [
    cec.CEC_USER_CONTROL_CODE_SELECT,
    cec.CEC_USER_CONTROL_CODE_UP,
    cec.CEC_USER_CONTROL_CODE_DOWN,
    cec.CEC_USER_CONTROL_CODE_LEFT,
    cec.CEC_USER_CONTROL_CODE_RIGHT,
    cec.CEC_USER_CONTROL_CODE_RIGHT_UP,
    cec.CEC_USER_CONTROL_CODE_RIGHT_DOWN,
    cec.CEC_USER_CONTROL_CODE_LEFT_UP,
    cec.CEC_USER_CONTROL_CODE_LEFT_DOWN,
    cec.CEC_USER_CONTROL_CODE_ROOT_MENU,
    cec.CEC_USER_CONTROL_CODE_SETUP_MENU,
    cec.CEC_USER_CONTROL_CODE_CONTENTS_MENU,
    cec.CEC_USER_CONTROL_CODE_FAVORITE_MENU,
    cec.CEC_USER_CONTROL_CODE_EXIT,
    cec.CEC_USER_CONTROL_CODE_TOP_MENU,
    cec.CEC_USER_CONTROL_CODE_DVD_MENU,
    cec.CEC_USER_CONTROL_CODE_NUMBER_ENTRY_MODE,
    cec.CEC_USER_CONTROL_CODE_NUMBER11,
    cec.CEC_USER_CONTROL_CODE_NUMBER12,
    cec.CEC_USER_CONTROL_CODE_NUMBER0,
    cec.CEC_USER_CONTROL_CODE_NUMBER1,
    cec.CEC_USER_CONTROL_CODE_NUMBER2,
    cec.CEC_USER_CONTROL_CODE_NUMBER3,
    cec.CEC_USER_CONTROL_CODE_NUMBER4,
    cec.CEC_USER_CONTROL_CODE_NUMBER5,
    cec.CEC_USER_CONTROL_CODE_NUMBER6,
    cec.CEC_USER_CONTROL_CODE_NUMBER7,
    cec.CEC_USER_CONTROL_CODE_NUMBER8,
    cec.CEC_USER_CONTROL_CODE_NUMBER9,
    cec.CEC_USER_CONTROL_CODE_DOT,
    cec.CEC_USER_CONTROL_CODE_ENTER,
    cec.CEC_USER_CONTROL_CODE_CLEAR,
    cec.CEC_USER_CONTROL_CODE_NEXT_FAVORITE,
    cec.CEC_USER_CONTROL_CODE_CHANNEL_UP,
    cec.CEC_USER_CONTROL_CODE_CHANNEL_DOWN,
    cec.CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL,
    cec.CEC_USER_CONTROL_CODE_SOUND_SELECT,
    cec.CEC_USER_CONTROL_CODE_INPUT_SELECT,
    cec.CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION,
    cec.CEC_USER_CONTROL_CODE_HELP,
    cec.CEC_USER_CONTROL_CODE_PAGE_UP,
    cec.CEC_USER_CONTROL_CODE_PAGE_DOWN,
    cec.CEC_USER_CONTROL_CODE_POWER,
    cec.CEC_USER_CONTROL_CODE_VOLUME_UP,
    cec.CEC_USER_CONTROL_CODE_VOLUME_DOWN,
    cec.CEC_USER_CONTROL_CODE_MUTE,
    cec.CEC_USER_CONTROL_CODE_PLAY,
    cec.CEC_USER_CONTROL_CODE_STOP,
    cec.CEC_USER_CONTROL_CODE_PAUSE,
    cec.CEC_USER_CONTROL_CODE_RECORD,
    cec.CEC_USER_CONTROL_CODE_REWIND,
    cec.CEC_USER_CONTROL_CODE_FAST_FORWARD,
    cec.CEC_USER_CONTROL_CODE_EJECT,
    cec.CEC_USER_CONTROL_CODE_FORWARD,
    cec.CEC_USER_CONTROL_CODE_BACKWARD,
    cec.CEC_USER_CONTROL_CODE_STOP_RECORD,
    cec.CEC_USER_CONTROL_CODE_PAUSE_RECORD,
    cec.CEC_USER_CONTROL_CODE_ANGLE,
    cec.CEC_USER_CONTROL_CODE_SUB_PICTURE,
    cec.CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND,
    cec.CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE,
    cec.CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING,
    cec.CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION,
    cec.CEC_USER_CONTROL_CODE_SELECT_BROADCAST_TYPE,
    cec.CEC_USER_CONTROL_CODE_SELECT_SOUND_PRESENTATION,
    cec.CEC_USER_CONTROL_CODE_PLAY_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_RECORD_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_STOP_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_MUTE_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_TUNE_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION,
    cec.CEC_USER_CONTROL_CODE_F1_BLUE,
    cec.CEC_USER_CONTROL_CODE_F2_RED,
    cec.CEC_USER_CONTROL_CODE_F3_GREEN,
    cec.CEC_USER_CONTROL_CODE_F4_YELLOW,
    cec.CEC_USER_CONTROL_CODE_F5,
    cec.CEC_USER_CONTROL_CODE_DATA,
    cec.CEC_USER_CONTROL_CODE_AN_RETURN,
    cec.CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST,
    cec.CEC_USER_CONTROL_CODE_MAX,
    cec.CEC_USER_CONTROL_CODE_UNKNOWN,
]


class _UserControlCodes(object):
    _control_codes = {}

    def __init__(self):
        cec_lib = cec.ICECAdapter.Create(cec.libcec_configuration())

        for code in _CONTROL_CODES:
            code_name = cec_lib.UserControlCodeToString(code).title()
            self._control_codes[code_name.replace(' (Function)', '')] = code
        cec_lib.Close()

    def __iter__(self):
        for key in sorted(self._control_codes.keys()):
            yield key

    def __contains__(self, item):
        return item in self._control_codes

    def __getattr__(self, item):
        if item in self.__dict__:
            return self.__dict__[item]

        if item in self._control_codes:
            return self._control_codes[item]

        for key in self._control_codes:
            if '(%s)' % item in key:
                return self._control_codes[key]

        raise AttributeError


UserControlCodes = _UserControlCodes()


class CECDevice(object):
    def __init__(self, adapter, name, device_const):
        self.adapter = adapter
        self.sla = adapter.LogicalAddressToString(device_const)
        self.pa = adapter.GetDevicePhysicalAddress(device_const)
        self.la = device_const
        self._osd_event = threading.Event()
        self._osd_thread = None
        self._osd_string = None
        self.name = name

    @property
    def osd_name(self):
        return self.adapter.GetDeviceOSDName(self.la)

    @property
    def osd_string(self):
        return self._osd_string

    @osd_string.setter
    def osd_string(self, (msg, duration)):
        if self._osd_thread is not None:
            self._osd_event.set()
            self._osd_thread.join(1.0)

        self._osd_event.clear()

        def clear_osd():
            self._osd_event.wait(duration)
            self._osd_string = None
            self._osd_thread = None

        self._osd_string = msg
        self._osd_thread = threading.Thread(target=clear_osd())
        self.adapter.SetOSDString(self.la, duration, msg)
        self._osd_thread.start()

    @property
    def menu_language(self):
        return self.adapter.GetDeviceMenuLanguage(self.la)

    @property
    def cec_version(self):
        cec_version = self.adapter.GetDeviceCecVersion(self.la)
        return self.adapter.CecVersionToString(cec_version)

    @property
    def vendor(self):
        vendor_id = self.adapter.GetDeviceVendorId(self.la)
        return self.adapter.VendorIdToString(vendor_id)

    @property
    def power(self):
        return CEC_POWER_CONSTANTS[self.adapter.GetDevicePowerStatus(self.la)]

    @power.setter
    def power(self, flag):
        if flag:
            self.adapter.PowerOnDevices(self.la)
        else:
            self.adapter.StandbyDevices(self.la)

    @property
    def active_device(self):
        return self.adapter.IsActiveDevice(self.la)

    @property
    def active_source(self):
        return self.adapter.IsActiveSource(self.la)

    @active_source.setter
    def active_source(self, flag=True):
        if flag:
            self.adapter.SetActiveSource(self.la)

    def __getattr__(self, item):
        if item in self.__dict__:
            return self.__dict__[item]

        if item in UserControlCodes:
            adapter = self.adapter

            class Wrapper:
                def __init__(self):
                    pass

                @staticmethod
                def send_key_press():
                    code = getattr(UserControlCodes, item)
                    print code
                    adapter.SendKeypress(
                        self.la,
                        code
                    )

                @staticmethod
                def send_key_release():
                    adapter.SendKeyRelease(self.la)
            return Wrapper
        return None


class AdapterError(Exception):
    pass


class CECAdapter(object):
    @eg.LogIt
    def __init__(self, com_port, adapter_name, hdmi_port, use_avr, poll_interval):
        self.name = adapter_name
        self.com_port = com_port
        self._log_level = None
        self._menu_state = False
        self._key_event = None
        self._last_key = 255
        self._restart_params = (com_port, adapter_name, hdmi_port, use_avr)
        self._poll_event = threading.Event()
        self._poll_interval = poll_interval
        self._poll_thread = threading.Thread(
            name='PulseEightCEC-' + adapter_name,
            target=self._run_poll
        )

        self.cec_config = cec_config = cec.libcec_configuration()
        cec_config.clientVersion = cec.LIBCEC_VERSION_CURRENT
        cec_config.deviceTypes.Add(
            cec.CEC_DEVICE_TYPE_RECORDING_DEVICE
        )
        cec_config.SetLogCallback(self._log_callback)
        cec_config.SetKeyPressCallback(self._key_callback)
        cec_config.iHDMIPort = hdmi_port
        cec_config.strDeviceName = str(adapter_name)
        cec_config.bActivateSource = 0

        if use_avr:
            cec_config.baseDevice = cec.CECDEVICE_AUDIOSYSTEM
        else:
            cec_config.baseDevice = cec.CECDEVICE_TV

        self.adapter = adapter = cec.ICECAdapter.Create(cec_config)

        if adapter.Open(com_port):
            eg.Print('CEC: connection opened on ' + com_port)
        else:
            eg.PrintError(
                'CEC Error: connection failed on ' + com_port
            )
            raise AdapterError

        self.tv = CECDevice(adapter, 'TV', cec.CECDEVICE_TV)
        self.tuner1 = CECDevice(adapter, 'Tuner 1', cec.CECDEVICE_TUNER1)
        self.tuner2 = CECDevice(adapter, 'Tuner 2', cec.CECDEVICE_TUNER2)
        self.tuner3 = CECDevice(adapter, 'Tuner 3', cec.CECDEVICE_TUNER3)
        self.tuner4 = CECDevice(adapter, 'Tuner 4', cec.CECDEVICE_TUNER4)
        self.audiosystem = CECDevice(adapter, 'AVR', cec.CECDEVICE_AUDIOSYSTEM)
        self.freeuse = CECDevice(adapter, 'Free Use', cec.CECDEVICE_FREEUSE)
        self.unknown = CECDevice(adapter, 'Unknown', cec.CECDEVICE_UNKNOWN)
        self.broadcast = CECDevice(
            adapter,
            'Broadcast',
            cec.CECDEVICE_BROADCAST
        )
        self.reserved1 = CECDevice(
            adapter,
            'Reserved 1',
            cec.CECDEVICE_RESERVED1
        )
        self.reserved2 = CECDevice(
            adapter,
            'Reserved 2',
            cec.CECDEVICE_RESERVED2
        )
        self.recordingdevice1 = CECDevice(
            adapter,
            'Recording Device 1',
            cec.CECDEVICE_RECORDINGDEVICE1
        )
        self.playbackdevice1 = CECDevice(
            adapter,
            'Playback Device 1',
            cec.CECDEVICE_PLAYBACKDEVICE1
        )
        self.recordingdevice2 = CECDevice(
            adapter,
            'Recording Device 2',
            cec.CECDEVICE_RECORDINGDEVICE2
        )
        self.playbackdevice2 = CECDevice(
            adapter,
            'Playback Device 2',
            cec.CECDEVICE_PLAYBACKDEVICE2
        )
        self.recordingdevice3 = CECDevice(
            adapter,
            'Recording Device 3',
            cec.CECDEVICE_RECORDINGDEVICE3
        )
        self.playbackdevice3 = CECDevice(
            adapter,
            'Playback Device 3',
            cec.CECDEVICE_PLAYBACKDEVICE3
        )

        self.devices = [
            self.tv,
            self.audiosystem,
            self.tuner1,
            self.tuner2,
            self.tuner3,
            self.tuner4,
            self.recordingdevice1,
            self.recordingdevice2,
            self.recordingdevice3,
            self.playbackdevice1,
            self.playbackdevice2,
            self.playbackdevice3,
            self.reserved1,
            self.reserved2,
            self.freeuse,
            self.broadcast,
            self.unknown,
        ]
        self._poll_thread.start()

    def _run_poll(self):
        devices = []

        volume = self.volume
        mute = self.mute
        menu = self.menu

        for device in self.devices:
            try:
                devices.append([
                    device.active_device,
                    device.active_source,
                    device.power,
                    device.menu_language
                ])
            except:
                devices.append([None] * 4)

        while not self._poll_event.isSet():
            new_volume = self.volume
            new_mute = self.mute
            new_menu = self.menu

            if volume != new_volume:
                volume = new_volume
                if volume is not None:
                    eg.TriggerEvent(
                        prefix=self.name,
                        suffix='Volume.' + str(volume)
                    )

            if mute != new_mute:
                mute = new_mute
                if mute is not None:
                    if mute:
                        suffix = 'On'
                    else:
                        suffix = 'Off'

                    eg.TriggerEvent(
                        prefix=self.name,
                        suffix='Mute.' + suffix
                    )

            if menu != new_menu:
                menu = new_menu
                if menu is not None:
                    if menu:
                        suffix = 'Opened'
                    else:
                        suffix = 'Closed'

                    eg.TriggerEvent(
                        prefix=self.name,
                        suffix='Menu.' + suffix
                    )

            for i, device in enumerate(self.devices):
                active, source, power, language = devices[i]

                new_active = device.active_device
                new_source = device.active_source
                new_power = device.power
                new_language = device.menu_language

                if active != new_active:
                    active = new_active
                    if active:
                        suffix = 'Active'
                    else:
                        suffix = 'Inactive'

                    eg.TriggerEvent(
                        prefix=self.name,
                        suffix=device.name + '.' + suffix
                    )

                if source != new_source:
                    source = new_source
                    if source:
                        eg.TriggerEvent(
                            prefix=self.name,
                            suffix='Source.' + device.name
                        )

                if power != new_power:
                    if power is None:
                        eg.TriggerEvent(
                            prefix=self.name,
                            suffix=device.name + '.Connected'
                        )
                    power = new_power
                    if power is None:
                        eg.TriggerEvent(
                            prefix=self.name,
                            suffix=device.name + '.Disconnected'
                        )
                    else:
                        if power:
                            suffix = 'On'
                        else:
                            suffix = 'Off'
                        eg.TriggerEvent(
                            prefix=self.name,
                            suffix=device.name + '.Power.' + suffix
                        )

                if language != new_language:
                    language = new_language
                    eg.TriggerEvent(
                        prefix=self.name,
                        suffix=device.name + '.MenuLanguage.' + str(language)
                    )

                devices[i] = [active, source, power, language]
            self._poll_event.wait(self._poll_interval)

    def transmit_command(self, command):
        return self.adapter.Transmit(self.adapter.CommandFromString(command))

    def _log_callback(self, level, time, message):
        if (
            self._log_level is not None and
            level <= self._log_level and
            level in CEC_LOG_CONSTANTS
        ):
            level_str = CEC_LOG_CONSTANTS[level]
            eg.PrintDebugNotice(
                "CEC %s: %s [%s]    %s" %
                (self.name, level_str, str(time), message)
            )
        return 0

    def _key_callback(self, key, duration):
        str_key = lib.UserControlCodeToString(key).title()
        if duration == 0 and self._last_key != key:
            self._last_key = key
            self._key_event = eg.TriggerEnduringEvent(
                prefix=self._name,
                suffix='KeyPressed.' + str_key
            )
        elif duration > 0 and self._last_key == key:
            self._last_key = 255
            self._key_event.SetShouldEnd()
            self._key_event = None
        elif self._last_key != key:
            self._last_key = 255
            eg.TriggerEvent(
                prefix=self._name,
                suffix='KeyPressed.' + str_key
            )
        return 0

    @property
    def log_level(self):
        return self._log_level

    @log_level.setter
    def log_level(self, level):
        if level is not None and level not in CEC_LOG_CONSTANTS:
            return
        self._log_level = level

    @property
    def vendor(self):
        vendor_id = self.adapter.GetAdapterVendorId()
        return self.adapter.VendorIdToString(vendor_id)

    @property
    def menu(self):
        return self._menu_state

    @menu.setter
    def menu(self, state):
        self._menu_state = state
        self.adapter.SetMenuState(state)

    def set_interactive_view(self):
        self.adapter.SetInactiveView()

    @property
    def volume(self):
        res = self.adapter.AudioStatus() ^ cec.CEC_AUDIO_MUTE_STATUS_MASK
        if res == 255:
            return None
        return res

    @volume.setter
    def volume(self, volume):
        if volume < self.volume:
            while volume < self.volume:
                self.volume_down()

        elif volume > self.volume:
            while volume > self.volume:
                self.volume_up()

    def volume_up(self):
        self.adapter.VolumeUp()
        return self.volume

    def volume_down(self):
        self.adapter.VolumeDown()
        return self.volume

    @property
    def mute(self):
        return (
            self.adapter.AudioStatus() & cec.CEC_AUDIO_MUTE_STATUS_MASK ==
            cec.CEC_AUDIO_MUTE_STATUS_MASK
        )

    @mute.setter
    def mute(self, flag):
        if flag and not self.mute:
            self.adapter.AudioMute()
        elif not flag and self.mute:
            self.adapter.AudioUnmute()

    def toggle_mute(self):
        self.adapter.AudioToggleMute()

    def restart(self):
        self.close()
        return CECAdapter(*self._restart_params)

    def close(self):
        self._poll_event.set()
        self._poll_thread.join(3)
        self.adapter.Close()
        eg.Print('CEC: connection closed on ' + self.com_port)
