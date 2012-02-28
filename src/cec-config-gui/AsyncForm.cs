using System;
using System.Windows.Forms;

namespace CecConfigGui
{
  public class AsyncForm : Form
  {
    delegate void SetControlEnabledCallback(Control control, bool val);
    public void SetControlEnabled(Control control, bool val)
    {
      if (control.InvokeRequired)
      {
        SetControlEnabledCallback d = new SetControlEnabledCallback(SetControlEnabled);
        try
        {
          Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Enabled = val;
      }
    }

    delegate void SetControlTextCallback(Control control, string val);
    public void SetControlText(Control control, string val)
    {
      if (control.InvokeRequired)
      {
        SetControlTextCallback d = new SetControlTextCallback(SetControlText);
        try
        {
          Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Text = val;
      }
    }

    delegate void SetCheckboxCheckedCallback(CheckBox control, bool val);
    public void SetCheckboxChecked(CheckBox control, bool val)
    {
      if (control.InvokeRequired)
      {
        SetCheckboxCheckedCallback d = new SetCheckboxCheckedCallback(SetCheckboxChecked);
        try
        {
          Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Checked = val;
      }
    }

    delegate void SetCheckboxItemCheckedCallback(CheckedListBox control, int index, bool val);
    public void SetCheckboxItemChecked(CheckedListBox control, int index, bool val)
    {
      if (control.InvokeRequired)
      {
        SetCheckboxItemCheckedCallback d = new SetCheckboxItemCheckedCallback(SetCheckboxItemChecked);
        try
        {
          Invoke(d, new object[] { control, index, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.SetItemChecked(index, val);
      }
    }

    delegate void SetProgressValueCallback(ProgressBar control, int val);
    public void SetProgressValue(ProgressBar control, int val)
    {
      if (control.InvokeRequired)
      {
        SetProgressValueCallback d = new SetProgressValueCallback(SetProgressValue);
        try
        {
          Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Value = val;
      }
    }

    delegate void SetComboBoxItemsCallback(ComboBox control, string selectedText, object[] val);
    public void SetComboBoxItems(ComboBox control, string selectedText, object[] val)
    {
      if (control.InvokeRequired)
      {
        SetComboBoxItemsCallback d = new SetComboBoxItemsCallback(SetComboBoxItems);
        try
        {
          Invoke(d, new object[] { control, selectedText, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Items.Clear();
        control.Items.AddRange(val);
        control.Text = selectedText;
      }
    }

    delegate void SetControlVisibleCallback(Control control, bool val);
    public void SetControlVisible(Control control, bool val)
    {
      if (control.InvokeRequired)
      {
        SetControlVisibleCallback d = new SetControlVisibleCallback(SetControlVisible);
        try
        {
          Invoke(d, new object[] { control, val });
        }
        catch (Exception) { }
      }
      else
      {
        control.Visible = val;
      }
    }

    delegate void DisplayDialogCallback(Form control, bool modal);
    public void DisplayDialog(Form control, bool modal)
    {
      if (InvokeRequired)
      {
        DisplayDialogCallback d = new DisplayDialogCallback(DisplayDialog);
        try
        {
          Invoke(d, new object[] { control, modal });
        }
        catch (Exception) { }
      }
      else
      {
        if (modal)
          control.ShowDialog(this);
        else
          control.Show(this);
      }
    }
  }
}
