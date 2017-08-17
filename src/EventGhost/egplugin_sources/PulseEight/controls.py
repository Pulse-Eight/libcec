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
import threading
import wx
from wx.lib.agw import ultimatelistctrl as ulc


class Text(eg.TranslatableStrings):
    name_lbl = 'Adapter Name'
    avr_lbl = 'AVR Volume Control'
    com_port_lbl = 'Adapter COM Port'
    hdmi_lbl = 'Device HDMI Port'
    adapter_lbl = 'Adapter:'
    device_lbl = 'Device:'
    poll_lbl = 'Polling Speed (ms)'


class AdapterListCtrl(ulc.UltimateListCtrl):

    @eg.LogIt
    def __init__(self, parent):
        self.lock = threading.Lock()

        ulc.UltimateListCtrl.__init__(
            self,
            parent,
            -1,
            size=(600, 200),
            agwStyle=(
                wx.LC_REPORT |
                wx.BORDER_SUNKEN |
                wx.LC_EDIT_LABELS |
                wx.LC_VRULES |
                wx.LC_HRULES |
                ulc.ULC_HAS_VARIABLE_ROW_HEIGHT |
                ulc.ULC_BORDER_SELECT
            )
        )
        self.InsertColumn(0, Text.name_lbl)
        self.InsertColumn(1, Text.com_port_lbl)
        self.InsertColumn(2, Text.hdmi_lbl)
        self.InsertColumn(3, Text.avr_lbl)
        self.InsertColumn(4, Text.poll_lbl)

        self.SetColumnWidth(0, 100)
        self.SetColumnWidth(1, 120)
        self.SetColumnWidth(2, 115)
        self.SetColumnWidth(3, 130)
        self.SetColumnWidth(4, 130)

        def get_value():
            res = ()

            for row in range(self.GetItemCount()):
                name_item = self.GetItem(row, 0)
                com_item = self.GetItem(row, 1)
                hdmi_item = self.GetItem(row, 2)
                avr_item = self.GetItem(row, 3)
                poll_item = self.GetItem(row, 4)

                adapter_name = str(name_item.GetText())
                com_port = str(com_item.GetText())
                hdmi_port = hdmi_item.GetWindow().GetValue()
                use_avr = avr_item.GetWindow().GetValue()
                poll_interval = poll_item.GetWindow().GetValue()

                if adapter_name and adapter_name != 'ENTER NAME':
                    res += ((
                        com_port,
                        adapter_name,
                        hdmi_port,
                        use_avr,
                        poll_interval
                    ),)
            return res

        self.GetValue = get_value

    def add_cec_item(
        self,
        com_port,
        adapter_name,
        hdmi_port,
        use_avr,
        poll_interval,
        _
        # scan_type
    ):
        self.lock.acquire()
        self.Freeze()

        index = self.InsertStringItem(self.GetItemCount(), adapter_name)
        self.SetStringItem(index, 1, com_port)
        self.SetStringItem(index, 2, '')
        self.SetStringItem(index, 3, '')
        self.SetStringItem(index, 4, '')

        com_item = self.GetItem(index, 0)
        name_item = self.GetItem(index, 1)
        hdmi_item = self.GetItem(index, 2)
        avr_item = self.GetItem(index, 3)
        poll_item = self.GetItem(index, 4)

        hdmi_port_ctrl = eg.SpinIntCtrl(self, -1, hdmi_port, min=1, max=99)
        hdmi_item.SetWindow(hdmi_port_ctrl)

        avr_ctrl = wx.CheckBox(self, -1, '')
        avr_ctrl.SetValue(use_avr)
        avr_item.SetWindow(avr_ctrl)

        poll_ctrl = eg.SpinNumCtrl(
            self,
            -1,
            poll_interval,
            min=0.1,
            max=5.0,
            increment=0.1
        )
        poll_item.SetWindow(poll_ctrl)

        # if scan_type is None:
        #     com_item.SetBackgroundColour((255, 255, 75))
        #     name_item.SetBackgroundColour((255, 255, 75))
        #     hdmi_port_ctrl.SetBackgroundColour((255, 255, 75))
        #     hdmi_item.SetBackgroundColour((255, 255, 75))
        #     avr_item.SetBackgroundColour((255, 255, 75))
        #
        # elif scan_type is False:
        #     com_item.SetBackgroundColour((255, 0, 0))
        #     name_item.SetBackgroundColour((255, 0, 0))
        #     hdmi_item.SetBackgroundColour((255, 0, 0))
        #     avr_item.SetBackgroundColour((255, 0, 0))

        self.SetItem(com_item)
        self.SetItem(name_item)
        self.SetItem(hdmi_item)
        self.SetItem(avr_item)
        self.SetItem(poll_item)

        self.Thaw()
        self.Update()
        self.lock.release()


class AdapterCtrl(wx.Panel):

    def __init__(self, parent, com_port, adapter_name, adapters):
        wx.Panel.__init__(self, parent, -1)

        choices = list(
            adapter.name + ' : ' + adapter.com_port
            for adapter in adapters
        )

        adapters_st = wx.StaticText(self, -1, Text.adapter_lbl)
        adapters_ctrl = eg.Choice(self, 0, choices=sorted(choices))

        adapters_ctrl.SetStringSelection(
            str(adapter_name) + ' : ' + str(com_port)
        )

        def get_value():
            value = adapters_ctrl.GetStringSelection()
            a_name, c_port = value.split(' : ')
            return c_port, a_name

        adapters_sizer = wx.BoxSizer(wx.HORIZONTAL)
        adapters_sizer.Add(adapters_st, 0, wx.EXPAND | wx.ALL, 5)
        adapters_sizer.Add(adapters_ctrl, 0, wx.EXPAND | wx.ALL, 5)

        self.GetValue = get_value
        self.SetSizer(adapters_sizer)


class DeviceCtrl(wx.Panel):

    def __init__(self, parent, dev):
        wx.Panel.__init__(self, parent, -1)

        device_st = wx.StaticText(self, -1, Text.device_lbl)
        device_ctrl = eg.Choice(self, 0, choices=[])
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(device_st, 0, wx.EXPAND | wx.ALL, 5)
        sizer.Add(device_ctrl, 0, wx.EXPAND | wx.ALL, 5)

        def on_choice(evt):
            global dev
            dev = device_ctrl.GetStringSelection()
            evt.Skip()

        device_ctrl.Bind(wx.EVT_CHOICE, on_choice)

        def update_devices(adapter):
            choices = list(d.name for d in adapter.devices)
            device_ctrl.SetItems(choices)

            if dev in choices:
                device_ctrl.SetStringSelection(dev)
            else:
                device_ctrl.SetSelection(0)

        def get_value():
            return dev

        self.GetValue = get_value
        self.UpdateDevices = update_devices
        self.SetSizer(sizer)
