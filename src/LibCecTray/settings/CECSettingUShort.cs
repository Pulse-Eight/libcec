using System.Globalization;
using System.Windows.Forms;

namespace LibCECTray.settings
{
  /// <summary>
  /// A setting of type ushort that can be persisted in the registry
  /// </summary>
  class CECSettingUShort : CECSettingString
  {
    public CECSettingUShort(string keyName, string friendlyName, ushort defaultValue, SettingChangedHandler changedHandler) :
      base(CECSettingType.UShort, keyName, friendlyName, string.Format("{0,4:X}", defaultValue), changedHandler)
    {
    }

    public new ushort Value
    {
      get
      {
        ushort iValue;
        return base.Value != null && ushort.TryParse(base.Value, NumberStyles.AllowHexSpecifier, null, out iValue) ? iValue : ushort.MinValue;
      }
      set
      {
        base.Value = string.Format("{0,4:X}", value);
        if (Form != null)
          Form.SetControlText(ValueControl, base.Value);
      }
    }

    public new ushort DefaultValue
    {
      get
      {
        ushort iValue;
        return base.DefaultValue != null && ushort.TryParse(base.DefaultValue, NumberStyles.AllowHexSpecifier, null, out iValue) ? iValue : ushort.MinValue;
      }
      set { base.DefaultValue = string.Format("{0,4:X}", value); }
    }

    public new Control ValueControl
    {
      get
      {
        if (BaseValueControl == null)
        {
          TextBox control = new TextBox
                              {
                                MaxLength = 4,
                                Size = new System.Drawing.Size(100, 20),
                                Enabled = InitialEnabledValue,
                                Text = string.Format("{0,4:X}", Value)
                              };
          control.TextChanged += delegate
                                   {
                                     ushort iValue;
                                     if (
                                       !ushort.TryParse(control.Text, NumberStyles.AllowHexSpecifier, null,
                                                        out iValue))
                                       iValue = DefaultValue;
                                     Value = iValue;
                                   };
          BaseValueControl = control;
        }
        return BaseValueControl;
      }
    }
  }
}
