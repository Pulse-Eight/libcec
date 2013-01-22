/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2013 Pulse-Eight Limited.  All rights reserved.
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

using System;
using System.Windows.Forms;
using CecSharp;
using LibCECTray.controller.applications;

namespace LibCECTray.ui
{
  interface IAsyncControls
  {
    void SetControlEnabled(Control control, bool val);
    void SetControlText(Control control, string val);
    void SetToolStripMenuText(ToolStripMenuItem item, string val);
    void SetCheckboxChecked(CheckBox control, bool val);
    void SetCheckboxItemChecked(CheckedListBox control, int index, bool val);
    void SetProgressValue(ProgressBar control, int val);
    void SetComboBoxItems(ComboBox control, int selectedIndex, object[] val);
    void SetControlVisible(Control control, bool val);
    void DisplayDialog(Form control, bool modal);
    void SafeHide(bool val);
    void SetSelectedIndex(ComboBox control, int index);
    string GetSelectedTabName(TabControl container, TabControl.TabPageCollection tabPages);
    void SelectKeypressRow(Control container, DataGridView dgView, CecKeypress key);
  }

  /// <summary>
  /// Utility methods to change GUI content from another thread
  /// </summary>
  class AsyncControls
  {
    /// <summary>
    /// Enable or disable a control
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to enable, false to disable</param>
    public static void SetControlEnabled(Control container, Control control, bool val)
    {
      if (container == null || control == null) return;
      if (container.InvokeRequired)
      {
        SetControlEnabledCallback d = SetControlEnabled;
        try
        {
          container.Invoke(d, new object[] { container, control, val });
        }
        catch { }
      }
      else
      {
        control.Enabled = val;
      }
    }
    private delegate void SetControlEnabledCallback(Control container, Control control, bool val);

    /// <summary>
    /// Change the text label of a control
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="val">The new text</param>
    public static void SetControlText(Control container, Control control, string val)
    {
      if (container == null || control == null) return;
      if (container.InvokeRequired)
      {
        SetControlTextCallback d = SetControlText;
        try
        {
          container.Invoke(d, new object[] { container, control, val });
        }
        catch { }
      }
      else
      {
        control.Text = val;
      }
    }
    private delegate void SetControlTextCallback(Control container, Control control, string val);

    /// <summary>
    /// Change the checked status of a checkbox
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to change to checked, false to change to unchecked</param>
    public static void SetCheckboxChecked(Control container, CheckBox control, bool val)
    {
      if (container.InvokeRequired)
      {
        SetCheckboxCheckedCallback d = SetCheckboxChecked;
        try
        {
          container.Invoke(d, new object[] { container, control, val });
        }
        catch { }
      }
      else
      {
        control.Checked = val;
      }
    }
    private delegate void SetCheckboxCheckedCallback(Control container, CheckBox control, bool val);

    /// <summary>
    /// Change the checked status of an item in a CheckedListBox
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="index">The index of the checkbox in the list to change</param>
    /// <param name="val">True to change to checked, false to change to unchecked</param>
    public static void SetCheckboxItemChecked(Control container, CheckedListBox control, int index, bool val)
    {
      if (container.InvokeRequired)
      {
        SetCheckboxItemCheckedCallback d = SetCheckboxItemChecked;
        try
        {
          container.Invoke(d, new object[] { container, control, index, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.SetItemChecked(index, val);
      }
    }
    private delegate void SetCheckboxItemCheckedCallback(Control container, CheckedListBox control, int index, bool val);

    /// <summary>
    /// Changes the toolstrip menu text
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="item">The toolstrip menu item to change</param>
    /// <param name="val">The new value</param>
    public static void SetToolStripMenuText(Control container, ToolStripMenuItem item, string val)
    {
      if (container.InvokeRequired)
      {
        SetToolStripMenuTextCallback d = SetToolStripMenuText;
        try
        {
          container.Invoke(d, new object[] { container, item, val });
        }
        catch (Exception) { }
      }
      else
      {
        item.Text = val;
      }
    }
    private delegate void SetToolStripMenuTextCallback(Control container, ToolStripMenuItem item, string val);

    /// <summary>
    /// Changes the progress value of a progress bar
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="val">The new percentage</param>
    public static void SetProgressValue(Control container, ProgressBar control, int val)
    {
      if (container.InvokeRequired)
      {
        SetProgressValueCallback d = SetProgressValue;
        try
        {
          container.Invoke(d, new object[] { container, control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Value = val;
      }
    }
    private delegate void SetProgressValueCallback(Control container, ProgressBar control, int val);

    /// <summary>
    /// Replaces the items of a combobox
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="selectedIndex">The new selection index</param>
    /// <param name="val">The new content</param>
    public static void SetComboBoxItems(Control container, ComboBox control, int selectedIndex, object[] val)
    {
      if (container.InvokeRequired)
      {
        SetComboBoxItemsCallback d = SetComboBoxItems;
        try
        {
          container.Invoke(d, new object[] { container, control, selectedIndex, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Items.Clear();
        control.Items.AddRange(val);
        if (control.Items.Count > 0)
          control.SelectedIndex = selectedIndex;
      }
    }
    private delegate void SetComboBoxItemsCallback(Control container, ComboBox control, int selectedIndex, object[] val);

    /// <summary>
    /// Make a control visible or invisible
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to make it visible, false to make it invisible</param>
    public static void SetControlVisible(Control container, Control control, bool val)
    {
      if (container.InvokeRequired)
      {
        SetControlVisibleCallback d = SetControlVisible;
        try
        {
          container.Invoke(d, new object[] { container, control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Visible = val;
      }
    }
    private delegate void SetControlVisibleCallback(Control container, Control control, bool val);

    /// <summary>
    /// Display a new dialog
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to display</param>
    /// <param name="modal">True to make it a modal dialog</param>
    public static void DisplayDialog(Control container, Form control, bool modal)
    {
      if (container.InvokeRequired)
      {
        DisplayDialogCallback d = DisplayDialog;
        try
        {
          container.Invoke(d, new object[] { container, control, modal });
        }
        catch (Exception) { }
      }
      else
      {
        if (modal)
          control.ShowDialog(container);
        else
          control.Show(container);
      }
    }
    private delegate void DisplayDialogCallback(Control container, Form control, bool modal);

    /// <summary>
    /// Hides a control
    /// </summary>
    /// <param name="container">The control to hide</param>
    /// <param name="val">True to hide, false to show</param>
    public static void SafeHide(Control container, bool val)
    {
      if (container.InvokeRequired)
      {
        SafeHideCallback d = SafeHide;
        try
        {
          container.Invoke(d, new object[] { container, val });
        }
        catch (Exception) { }
      }
      else
      {
        if (val)
          container.Hide();
        else
          container.Show();
      }
    }
    private delegate void SafeHideCallback(Control container, bool val);

    /// <summary>
    /// Change the selected index
    /// </summary>
    /// <param name="container">The control that contains the control to change</param>
    /// <param name="control">The control to change</param>
    /// <param name="index">The new selected index</param>
    public static void SetSelectedIndex(Control container, ComboBox control, int index)
    {
      if (container.InvokeRequired)
      {
        SetSelectedIndexCallback d = SetSelectedIndex;
        try
        {
          container.Invoke(d, new object[] { container, control, index });
        }
        catch (Exception) { }
      }
      else
      {
        control.SelectedIndex = index;
      }
    }
    private delegate void SetSelectedIndexCallback(Control container, ComboBox control, int index);

    /// <summary>
    /// Get the name of the selected tab in a TabControl
    /// </summary>
    /// <param name="container">The tab container</param>
    /// <param name="tabPages">The tab pages</param>
    /// <returns>The name of the selected tab</returns>
    public static string GetSelectedTabName(TabControl container, TabControl.TabPageCollection tabPages)
    {
      if (container.InvokeRequired)
      {
        GetSelectedTabNameCallback d = GetSelectedTabName;
        try
        {
          return container.Invoke(d, new object[] { container, tabPages }) as string;
        }
        catch (Exception) { }
      }
      else
      {
        return tabPages[container.SelectedIndex].Name;
      }
      return string.Empty;
    }
    private delegate string GetSelectedTabNameCallback(TabControl container, TabControl.TabPageCollection tabPages);

    /// <summary>
    /// Selects the row with the given CecKeypress for a datagrid
    /// </summary>
    /// <param name="container">The datagrid container</param>
    /// <param name="dgView">The datagrid</param>
    /// <param name="key">The key to selected</param>
    public static void SelectKeypressRow(Control container, DataGridView dgView, CecKeypress key)
    {
      if (dgView.InvokeRequired)
      {
        SelectKeypressRowCallback d = SelectKeypressRow;
        try
        {
          container.Invoke(d, new object[] { container, dgView, key });
        }
        catch (Exception) { }
      }
      else
      {
        var rowIndex = -1;
        foreach (DataGridViewRow row in dgView.Rows)
        {
          CecButtonConfigItem item = row.DataBoundItem as CecButtonConfigItem;
          if (item != null && item.Key.Keycode == key.Keycode)
          {
            rowIndex = row.Index;
            row.Selected = true;
            item.Enabled = true;
          }
          else
          {
            row.Selected = false;
          }
        }
        if (rowIndex > -1)
          dgView.FirstDisplayedScrollingRowIndex = rowIndex;
      }
    }
    private delegate void SelectKeypressRowCallback(Control container, DataGridView dgView, CecKeypress key);
  }

  /// <summary>
  /// Form that implements IAsyncControls
  /// </summary>
  class AsyncForm : Form, IAsyncControls
  {
    /// <summary>
    /// Changes the ShowInTaskbar value
    /// </summary>
    /// <param name="val">True to show, false to hide</param>
    public void SetShowInTaskbar(bool val)
    {
      if (InvokeRequired)
      {
        SetShowInTaskbarCallback d = SetShowInTaskbar;
        try
        {
          Invoke(d, new object[] { val });
        }
        catch (Exception) { }
      }
      else
      {
        ShowInTaskbar = val;
      }
    }
    private delegate void SetShowInTaskbarCallback(bool val);

    /// <summary>
    /// Enable or disable a control
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to enable, false to disable</param>
    public void SetControlEnabled(Control control, bool val)
    {
      AsyncControls.SetControlEnabled(this, control, val);
    }

    /// <summary>
    /// Change the text label of a control
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">The new text</param>
    public void SetControlText(Control control, string val)
    {
      AsyncControls.SetControlText(this, control, val);
    }

    /// <summary>
    /// Changes the toolstrip menu text
    /// </summary>
    /// <param name="item">The toolstrip menu item to change</param>
    /// <param name="val">The new value</param>
    public void SetToolStripMenuText(ToolStripMenuItem item, string val)
    {
      AsyncControls.SetToolStripMenuText(this, item, val);
    }

    /// <summary>
    /// Change the checked status of a checkbox
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to change to checked, false to change to unchecked</param>
    public void SetCheckboxChecked(CheckBox control, bool val)
    {
      AsyncControls.SetCheckboxChecked(this, control, val);
    }

    /// <summary>
    /// Change the checked status of an item in a CheckedListBox
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="index">The index of the checkbox in the list to change</param>
    /// <param name="val">True to change to checked, false to change to unchecked</param>
    public void SetCheckboxItemChecked(CheckedListBox control, int index, bool val)
    {
      AsyncControls.SetCheckboxItemChecked(this, control, index, val);
    }

    /// <summary>
    /// Changes the progress value of a progress bar
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">The new percentage</param>
    public void SetProgressValue(ProgressBar control, int val)
    {
      AsyncControls.SetProgressValue(this, control, val);
    }

    /// <summary>
    /// Replaces the items of a combobox
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="selectedIndex">The new selection index</param>
    /// <param name="val">The new content</param>
    public void SetComboBoxItems(ComboBox control, int selectedIndex, object[] val)
    {
      AsyncControls.SetComboBoxItems(this, control, selectedIndex, val);
    }

    /// <summary>
    /// Make a control visible or invisible
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to make it visible, false to make it invisible</param>
    public void SetControlVisible(Control control, bool val)
    {
      AsyncControls.SetControlVisible(this, control, val);
    }

    /// <summary>
    /// Display a new dialog
    /// </summary>
    /// <param name="control">The control to display</param>
    /// <param name="modal">True to make it a modal dialog</param>
    public void DisplayDialog(Form control, bool modal)
    {
      AsyncControls.DisplayDialog(this, control, modal);
    }

    /// <summary>
    /// Hides a control
    /// </summary>
    /// <param name="val">True to hide, false to show</param>
    public void SafeHide(bool val)
    {
      AsyncControls.SafeHide(this, val);
    }

    /// <summary>
    /// Change the selected index
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="index">The new selected index</param>
    public void SetSelectedIndex(ComboBox control, int index)
    {
      AsyncControls.SetSelectedIndex(this, control, index);
    }

    /// <summary>
    /// Get the name of the selected tab in a TabControl
    /// </summary>
    /// <param name="container">The tab container</param>
    /// <param name="tabPages">The tab pages</param>
    /// <returns>The name of the selected tab</returns>
    public string GetSelectedTabName(TabControl container, TabControl.TabPageCollection tabPages)
    {
      return AsyncControls.GetSelectedTabName(container, tabPages);
    }

    /// <summary>
    /// Selects the row with the given CecKeypress for a datagrid
    /// </summary>
    /// <param name="container">The datagrid container</param>
    /// <param name="dgView">The datagrid</param>
    /// <param name="key">The key to selected</param>
    public void SelectKeypressRow(Control container, DataGridView dgView, CecKeypress key)
    {
      AsyncControls.SelectKeypressRow(container, dgView, key);
    }
  }

  /// <summary>
  /// TabPage that implements IAsyncControls
  /// </summary>
  class AsyncTabPage : TabPage, IAsyncControls
  {
    /// <summary>
    /// Enable or disable a control
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to enable, false to disable</param>
    public void SetControlEnabled(Control control, bool val)
    {
      AsyncControls.SetControlEnabled(this, control, val);
    }

    /// <summary>
    /// Change the text label of a control
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">The new text</param>
    public void SetControlText(Control control, string val)
    {
      AsyncControls.SetControlText(this, control, val);
    }

    /// <summary>
    /// Changes the toolstrip menu text
    /// </summary>
    /// <param name="item">The toolstrip menu item to change</param>
    /// <param name="val">The new value</param>
    public void SetToolStripMenuText(ToolStripMenuItem item, string val)
    {
      AsyncControls.SetToolStripMenuText(this, item, val);
    }

    /// <summary>
    /// Change the checked status of a checkbox
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to change to checked, false to change to unchecked</param>
    public void SetCheckboxChecked(CheckBox control, bool val)
    {
      AsyncControls.SetCheckboxChecked(this, control, val);
    }

    /// <summary>
    /// Change the checked status of an item in a CheckedListBox
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="index">The index of the checkbox in the list to change</param>
    /// <param name="val">True to change to checked, false to change to unchecked</param>
    public void SetCheckboxItemChecked(CheckedListBox control, int index, bool val)
    {
      AsyncControls.SetCheckboxItemChecked(this, control, index, val);
    }

    /// <summary>
    /// Changes the progress value of a progress bar
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">The new percentage</param>
    public void SetProgressValue(ProgressBar control, int val)
    {
      AsyncControls.SetProgressValue(this, control, val);
    }

    /// <summary>
    /// Replaces the items of a combobox
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="selectedIndex">The new selection index</param>
    /// <param name="val">The new content</param>
    public void SetComboBoxItems(ComboBox control, int selectedIndex, object[] val)
    {
      AsyncControls.SetComboBoxItems(this, control, selectedIndex, val);
    }

    /// <summary>
    /// Make a control visible or invisible
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="val">True to make it visible, false to make it invisible</param>
    public void SetControlVisible(Control control, bool val)
    {
      AsyncControls.SetControlVisible(this, control, val);
    }

    /// <summary>
    /// Display a new dialog
    /// </summary>
    /// <param name="control">The control to display</param>
    /// <param name="modal">True to make it a modal dialog</param>
    public void DisplayDialog(Form control, bool modal)
    {
      AsyncControls.DisplayDialog(this, control, modal);
    }

    /// <summary>
    /// Hides a control
    /// </summary>
    /// <param name="val">True to hide, false to show</param>
    public void SafeHide(bool val)
    {
      AsyncControls.SafeHide(this, val);
    }

    /// <summary>
    /// Change the selected index
    /// </summary>
    /// <param name="control">The control to change</param>
    /// <param name="index">The new selected index</param>
    public void SetSelectedIndex(ComboBox control, int index)
    {
      AsyncControls.SetSelectedIndex(this, control, index);
    }

    /// <summary>
    /// Get the name of the selected tab in a TabControl
    /// </summary>
    /// <param name="container">The tab container</param>
    /// <param name="tabPages">The tab pages</param>
    /// <returns>The name of the selected tab</returns>
    public string GetSelectedTabName(TabControl container, TabControl.TabPageCollection tabPages)
    {
      return AsyncControls.GetSelectedTabName(container, tabPages);
    }

    /// <summary>
    /// Selects the row with the given CecKeypress for a datagrid
    /// </summary>
    /// <param name="container">The datagrid container</param>
    /// <param name="dgView">The datagrid</param>
    /// <param name="key">The key to selected</param>
    public void SelectKeypressRow(Control container, DataGridView dgView, CecKeypress key)
    {
      AsyncControls.SelectKeypressRow(container, dgView, key);
    }
  }
}
