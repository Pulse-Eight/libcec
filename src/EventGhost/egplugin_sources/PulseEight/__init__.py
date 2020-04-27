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

import eg


eg.RegisterPlugin(
    name='Pulse-Eight CEC adapter',
    author='Lars Op den Kamp, K',
    version='1.1b',
    kind='remote',
    # guid='{fd322eea-c897-470c-bef7-77bf15c52db4}',
    guid='{81AC5776-0220-4D2A-B561-DD91F052FF7B}',
    url='http://libcec.pulse-eight.com/',
    description=(
        '<rst>'
        'Integration with libCEC, which adds support for Pulse-Eight\'s ' 
        '`CEC adapters <http://www.pulse-eight.com/>`_.\n\n'
        '|\n\n'
        '.. image:: cec.png\n\n'
        '**Notice:** '
        'Make sure you select the correct HDMI port number on the device that '
        'the CEC adapter is connected to, '
        'or remote control input won\'t work.\n'
    ),
    createMacrosOnAdd=True,
    canMultiLoad=False,
    hardwareId="USB\\VID_2548&PID_1002",
)

from cec_classes import UserControlCodes, CECAdapter, AdapterError # NOQA
from controls import DeviceCtrl, AdapterCtrl, AdapterListCtrl # NOQA
from . import cec # NOQA
import threading # NOQA
import wx # NOQA


class Text(eg.TranslatableStrings):
    mute_group_lbl = 'Mute'
    volume_group_lbl = 'Volume'
    power_group_lbl = 'Power'
    remote_group_lbl = 'Remote Keys'
    volume_lbl = 'Volume:'
    command_lbl = 'Command:'
    key_lbl = 'Remote Key:'

    class RawCommand:
        name = 'Send command to an adapter'
        description = 'Send a raw CEC command to an adapter'

    class RestartAdapter:
        name = 'Restart Adapter'
        description = 'Restarts an adapter.'

    class VolumeUp:
        name = 'Volume Up'
        description = 'Turns up the volume by one point.'

    class VolumeDown:
        name = 'Volume Down'
        description = 'Turns down the volume by one point.'

    class GetVolume:
        name = 'Get Volume'
        description = 'Returns the current volume level.'

    class SetVolume:
        name = 'Set Volume'
        description = 'Sets the volume level.'

    class GetMute:
        name = 'Get Mute'
        description = 'Returns the mute state.'

    class ToggleMute:
        name = 'Toggle Mute'
        description = 'Toggles mute On and Off.'

    class MuteOn:
        name = 'Mute On'
        description = 'Turns mute on.'

    class MuteOff:
        name = 'Mute Off'
        description = 'Turns mute off.'

    class PowerOnAll:
        name = 'Power On All Devices'
        description = 'Powers on all devices on a specific adapter.'

    class StandbyAll:
        name = 'Standby All Devices'
        description = 'Powers off (standby) all devices in a specific adapter.'

    class StandbyDevice:
        name = 'Standby a Device'
        description = 'Powers off (standby) a single device.'

    class GetDevicePower:
        name = 'Get Device Power'
        description = 'Returns the power status of a device.'

    class PowerOnDevice:
        name = 'Power On a Device'
        description = 'Powers on a single device.'

    class GetDeviceVendor:
        name = 'Get Device Vendor'
        description = 'Returns the vendor of a device.'

    class GetDeviceMenuLanguage:
        name = 'Get Device Menu Language'
        description = 'Returns the menu language of a device.'

    class IsActiveSource:
        name = 'Is Device Active Source'
        description = 'Returns True/False if a device is the active source.'

    class IsDeviceActive:
        name = 'Is Device Active'
        description = 'Returns True/False if a device is active.'

    class GetDeviceOSDName:
        name = 'Get Device OSD Name'
        description = 'Returns the OSD text that is display for a device.'

    class SetDeviceActiveSource:
        name = 'Set Device as Active Source'
        description = 'Sets a device as the active source.'

    class SendRemoteKey:
        name = 'Send Remote Key'
        description = 'Send a Remote Keypress to a specific device.'


class PulseEight(eg.PluginBase):
    text = Text

    def __init__(self):
        self.adapters = []

        power_group = self.AddGroup(Text.power_group_lbl)
        power_group.AddAction(GetDevicePower)
        power_group.AddAction(PowerOnDevice)
        power_group.AddAction(StandbyDevice)
        power_group.AddAction(PowerOnAll)
        power_group.AddAction(StandbyAll)

        volume_group = self.AddGroup(Text.volume_group_lbl)
        volume_group.AddAction(GetVolume)
        volume_group.AddAction(VolumeUp)
        volume_group.AddAction(VolumeDown)
        volume_group.AddAction(SetVolume)

        mute_group = self.AddGroup(Text.mute_group_lbl)
        mute_group.AddAction(GetMute)
        mute_group.AddAction(MuteOn)
        mute_group.AddAction(MuteOff)
        mute_group.AddAction(ToggleMute)

        self.AddAction(SendRemoteKey)
        self.AddAction(SetDeviceActiveSource)
        self.AddAction(IsActiveSource)
        self.AddAction(IsDeviceActive)
        self.AddAction(GetDeviceVendor)
        self.AddAction(GetDeviceMenuLanguage)
        self.AddAction(GetDeviceOSDName)
        self.AddAction(RestartAdapter)
        self.AddAction(RawCommand)

        remote_group = self.AddGroup(Text.remote_group_lbl)
        remote_group.AddActionsFromList(REMOTE_ACTIONS)

    def __start__(self, *adapters):

        def start_connections(*adptrs):
            while self.adapters:
                pass

            cec_lib = cec.ICECAdapter.Create(cec.libcec_configuration())
            available_coms = list(
                a.strComName for a in cec_lib.DetectAdapters()
            )
            cec_lib.Close()

            for item in adptrs:
                com_port = item[0]

                if com_port in available_coms:
                    try:
                        self.adapters += [CECAdapter(*item)]
                    except AdapterError:
                        continue
                else:
                    eg.PrintError(
                        'CEC Error: adapter on %s is not found' % com_port
                    )

            if not self.adapters:
                eg.PrintError('CEC Error: no CEC adapters found')
                self.__stop__()

        for items in adapters:
            if not isinstance(items, tuple):
                eg.PrintError(
                    'You cannot upgrade to this version.\n'
                    'Delete the plugin from the plugins folder '
                    'and then install this one'
                )
                break
        else:
            threading.Thread(target=start_connections, args=adapters).start()

    @eg.LogIt
    def __stop__(self):
        for adapter in self.adapters:
            adapter.close()

        del self.adapters[:]

    def Configure(self, *adapters):
        panel = eg.ConfigPanel()

        loading_st = panel.StaticText(
            'Populating CEC Adapters, Please Wait.....'
        )
        list_ctrl = AdapterListCtrl(panel)
        desc_st = panel.StaticText(
            'Click on "ENTER NAME" and enter a name '
            'to register an adapter\n'
            'To remove an adapter registration delete the adapter name.'
        )

        ok_button = panel.dialog.buttonRow.okButton
        cancel_button = panel.dialog.buttonRow.cancelButton
        apply_button = panel.dialog.buttonRow.applyButton

        ok_button.Enable(False)
        cancel_button.Enable(False)
        apply_button.Enable(False)

        def populate():
            def on_close(_):
                pass

            panel.dialog.Bind(wx.EVT_CLOSE, on_close)

            cec_lib = cec.ICECAdapter.Create(cec.libcec_configuration())
            m_adapters = ()

            for adapter in cec_lib.DetectAdapters():
                com = adapter.strComName
                for settings in adapters:
                    com_port, adapter_name = settings[:2]
                    hdmi_port, use_avr, poll_interval = settings[2:]
                    if com_port == com:
                        m_adapters += ((
                            com_port,
                            adapter_name,
                            hdmi_port,
                            use_avr,
                            poll_interval,
                            True
                        ),)
                        wx.CallAfter(list_ctrl.add_cec_item, *m_adapters[-1])
                        break
                else:
                    wx.CallAfter(
                        list_ctrl.add_cec_item,
                        com,
                        'ENTER NAME',
                        1,
                        False,
                        0.5,
                        None
                    )

            for adapter in adapters:
                for m_adapter in m_adapters:
                    if m_adapter[:-1] == adapter:
                        break
                else:
                    m_adapters += (adapter + (False,),)
                    wx.CallAfter(list_ctrl.add_cec_item, *m_adapters[-1])

            cec_lib.Close()
            ok_button.Enable(True)
            cancel_button.Enable(True)
            apply_button.Enable(True)

            panel.dialog.Bind(wx.EVT_CLOSE, panel.dialog.OnCancel)
            loading_st.SetLabel('')

        loading_sizer = wx.BoxSizer(wx.HORIZONTAL)
        loading_sizer.AddStretchSpacer()
        loading_sizer.Add(loading_st, 0, wx.ALL | 5)
        loading_sizer.AddStretchSpacer()

        panel.sizer.Add(loading_sizer, 0, wx.EXPAND)
        panel.sizer.Add(list_ctrl, 1, wx.EXPAND)
        panel.sizer.Add(desc_st, 0, wx.EXPAND)

        threading.Thread(target=populate).start()

        while panel.Affirmed():
            panel.SetResult(*list_ctrl.GetValue())


class AdapterBase(eg.ActionBase):

    def GetLabel(self, com_port=None, adapter_name=None, *_):
        return '%s: %s on %s' % (self.name, adapter_name, com_port)

    def _find_adapter(self, com_port, adapter_name):
        if com_port is None and adapter_name is None:
            return None

        for adapter in self.plugin.adapters:
            if com_port == adapter.com_port and adapter_name == adapter.name:
                return adapter
            if com_port == adapter.com_port:
                return adapter
            if adapter_name == adapter.name:
                return adapter

    def __call__(self, *args):
        raise NotImplementedError

    def Configure(self, com_port='', adapter_name=''):
        panel = eg.ConfigPanel()

        adapter_ctrl = AdapterCtrl(
            panel,
            com_port,
            adapter_name,
            self.plugin.adapters
        )

        panel.sizer.Add(adapter_ctrl, 0, wx.EXPAND)

        while panel.Affirmed():
            panel.SetResult(*adapter_ctrl.GetValue())


class DeviceBase(AdapterBase):
    def _process_call(self, device):
        raise NotImplementedError

    def __call__(self, com_port=None, adapter_name=None, device='TV'):
        adapter = self._find_adapter(com_port, adapter_name)

        if adapter is None:
            eg.PrintNotice(
                'CEC: Adapter %s on com port %s not found' %
                (adapter_name, com_port)
            )
        else:
            d = getattr(adapter, device.lower().replace(' ', ''), None)
            if d is None:
                eg.PrintNotice(
                    'CEC: Device %s not found in adapter %s' %
                    (device, adpater.name)
                )
            else:
                return self._process_call(d)

    def Configure(self, com_port='', adapter_name='', device='TV'):
        panel = eg.ConfigPanel()

        adapter_ctrl = AdapterCtrl(
            panel,
            com_port,
            adapter_name,
            self.plugin.adapters
        )

        device_ctrl = DeviceCtrl(panel, device)
        if com_port and adapter_name:
            device_ctrl.UpdateDevices(
                self._find_adapter(com_port, adapter_name)
            )

        def on_choice(evt):
            device_ctrl.UpdateDevices(
                self._find_adapter(*adapter_ctrl.GetValue())
            )

            evt.Skip()

        device_ctrl.UpdateDevices(
            self._find_adapter(*adapter_ctrl.GetValue())
        )

        adapter_ctrl.Bind(wx.EVT_CHOICE, on_choice)
        panel.sizer.Add(adapter_ctrl, 0, wx.EXPAND)
        panel.sizer.Add(device_ctrl, 0, wx.EXPAND)

        while panel.Affirmed():
            com_port, adapter_name = adapter_ctrl.GetValue()
            panel.SetResult(
                com_port,
                adapter_name,
                device_ctrl.GetValue()
            )


class RestartAdapter(AdapterBase):

    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        self.plugin.adapters[self.plugin.adapters.index(adapter)] = (
            adapter.restart()
        )


class VolumeUp(AdapterBase):

    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        return adapter.volume_up()


class VolumeDown(AdapterBase):

    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        return adapter.volume_down()


class GetVolume(AdapterBase):

    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        return adapter.volume


class GetMute(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        return adapter.mute


class ToggleMute(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        return adapter.toggle_mute()


class MuteOn(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        adapter.mute = True
        return adapter.mute


class MuteOff(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        adapter.mute = False
        return adapter.mute


class PowerOnAll(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        for d in adapter.devices:
            d.power = True


class StandbyAll(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None):
        adapter = self._find_adapter(com_port, adapter_name)
        for d in adapter.devices:
            d.power = False


class StandbyDevice(DeviceBase):
    def _process_call(self, device):
        device.power = False
        return device.power


class GetDevicePower(DeviceBase):
    def _process_call(self, device):
        return device.power


class PowerOnDevice(DeviceBase):
    def _process_call(self, device):
        device.power = True
        return device.power


class GetDeviceVendor(DeviceBase):
    def _process_call(self, device):
        return device.vendor


class GetDeviceMenuLanguage(DeviceBase):
    def _process_call(self, device):
        return device.menu_language


class IsActiveSource(DeviceBase):
    def _process_call(self, device):
        return device.active_source


class IsDeviceActive(DeviceBase):
    def _process_call(self, device):
        return device.active_device


class GetDeviceOSDName(DeviceBase):
    def _process_call(self, device):
        return device.osd_name


class SetDeviceActiveSource(DeviceBase):
    def _process_call(self, device):
        device.active_source = True
        return device.active_source


class RawCommand(AdapterBase):
    def __call__(self, com_port=None, adapter_name=None, command=""):
        adapter = self._find_adapter(com_port, adapter_name)
        return adapter.transmit_command(command)

    def Configure(self, com_port='', adapter_name='', command=''):
        panel = eg.ConfigPanel()

        adapter_ctrl = AdapterCtrl(
            panel,
            com_port,
            adapter_name,
            self.plugin.adapters
        )

        command_st = panel.StaticText(Text.command_lbl)
        command_ctrl = panel.TextCtrl(command)

        command_sizer = wx.BoxSizer(wx.HORIZONTAL)
        command_sizer.Add(command_st, 0, wx.EXPAND | wx.ALL, 5)
        command_sizer.Add(command_ctrl, 0, wx.EXPAND | wx.ALL, 5)

        panel.sizer.Add(adapter_ctrl, 0, wx.EXPAND)
        panel.sizer.Add(command_sizer, 0, wx.EXPAND)

        while panel.Affirmed():
            com_port, adapter_name = adapter_ctrl.GetValue()
            panel.SetResult(com_port, adapter_name, command_ctrl.GetValue())


class SetVolume(AdapterBase):

    def __call__(self, com_port=None, adapter_name=None, volume=0):
        adapter = self._find_adapter(com_port, adapter_name)
        adapter.volume = volume
        return adapter.volume

    def Configure(self, com_port='', adapter_name='', volume=0):
        panel = eg.ConfigPanel()

        adapter_ctrl = AdapterCtrl(
            panel,
            com_port,
            adapter_name,
            self.plugin.adapters
        )
        volume_st = panel.StaticText(Text.volume_lbl)
        volume_ctrl = panel.SpinIntCtrl(volume, min=0, max=100)
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(volume_st, 0, wx.EXPAND | wx.ALL, 5)
        sizer.Add(volume_ctrl, 0, wx.EXPAND | wx.ALL, 5)

        panel.sizer.Add(adapter_ctrl, 0, wx.EXPAND)
        panel.sizer.Add(sizer, 0, wx.EXPAND)

        while panel.Affirmed():
            com_port, adapter_name = adapter_ctrl.GetValue()
            panel.SetResult(com_port, adapter_name, volume_ctrl.GetValue())


class SendRemoteKey(AdapterBase):

    def __call__(
        self,
        com_port=None,
        adapter_name=None,
        device='TV',
        key=None
    ):
        if key is None:
            key = getattr(self, 'value', None)
            if key is None or (com_port is None and adapter_name is None):
                eg.PrintNotice(
                    'CEC: This action needs to be configured before use.'
                )
                return

        adapter = self._find_adapter(com_port, adapter_name)

        if adapter is None:
            eg.PrintNotice(
                'CEC: Adapter %s on com port %s not found' %
                (adapter_name, com_port)
            )
        else:
            d = getattr(adapter, device.lower().replace(' ', ''), None)
            if d is None:
                eg.PrintNotice(
                    'CEC: Device %s not found in adapter %s' %
                    (device, adpater.name)
                )
            else:
                remote = getattr(d, key, None)
                if remote is None:
                    eg.PrintError(
                        'CEC: Key %s not found for device %s on adapter %s' %
                        (key, device, adpater.name)
                    )
                else:
                    import time
                    remote.send_key_press()
                    time.sleep(0.1)
                    remote.send_key_release()

    def Configure(self, com_port='', adapter_name='', device='TV', key=None):

        panel = eg.ConfigPanel()

        adapter_ctrl = AdapterCtrl(
            panel,
            com_port,
            adapter_name,
            self.plugin.adapters
        )

        device_ctrl = DeviceCtrl(panel, device)

        device_ctrl.UpdateDevices(
            self._find_adapter(*adapter_ctrl.GetValue())
        )

        def on_choice(evt):
            device_ctrl.UpdateDevices(
                self._find_adapter(*adapter_ctrl.GetValue())
            )

            evt.Skip()

        adapter_ctrl.Bind(wx.EVT_CHOICE, on_choice)
        panel.sizer.Add(adapter_ctrl, 0, wx.EXPAND)
        panel.sizer.Add(device_ctrl, 0, wx.EXPAND)

        if key is None and not hasattr(self, 'value'):
            key = ''
            key_st = panel.StaticText(Text.key_lbl)
            key_ctrl = panel.Choice(
                0,
                choices=list(key_name for key_name in UserControlCodes)
            )

            key_ctrl.SetStringSelection(key)

            key_sizer = wx.BoxSizer(wx.HORIZONTAL)
            key_sizer.Add(key_st, 0, wx.EXPAND | wx.ALL, 5)
            key_sizer.Add(key_ctrl, 0, wx.EXPAND | wx.ALL, 5)
            panel.sizer.Add(key_sizer, 0, wx.EXPAND)
        else:
            key_ctrl = None

        while panel.Affirmed():
            com_port, adapter_name = adapter_ctrl.GetValue()
            panel.SetResult(
                com_port,
                adapter_name,
                device_ctrl.GetValue(),
                None if key_ctrl is None else key_ctrl.GetStringSelection()
            )

REMOTE_ACTIONS = ()

for remote_key in UserControlCodes:
    key_func = remote_key
    for rep in ('Samsung', 'Blue', 'Red', 'Green', 'Yellow'):
        key_func = key_func.replace(' (%s)' % rep, '')
    key_func = key_func.replace('.', 'DOT').replace('+', '_').replace(' ', '_')

    REMOTE_ACTIONS += ((
        SendRemoteKey,
        'fn' + key_func.upper(),
        'Remote Key: ' + remote_key,
        'Remote Key ' + remote_key,
        remote_key
    ),)
