using System.Collections.Generic;

namespace CecConfigGui
{
  class CecButtonConfigItem
  {
    public CecButtonConfigItem(string name, CecSharp.CecKeypress key, string playerButton)
    {
      CecButtonName = name;
      Key = key;
      PlayerButton = playerButton;
      _enabled = false;
    }

    public string PlayerButton { get; set; }
    private bool _enabled;

    public bool Enabled
    {
      get
      {
        return _enabled || PlayerButton.Length > 0;
      }
      set
      {
        _enabled = value;
      }
    }
    public string CecButtonName { get; private set; }
    public CecSharp.CecKeypress Key { get; private set; }
  }

  class CecButtonConfig : List<CecButtonConfigItem>
  {
  }
}
