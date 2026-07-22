%module cec

%{
#include "SwigHelper.h"
%}

%include "stdint.i"
%include "cstring.i"
%include "std_string.i"
%include "std_vector.i"

%ignore *::operator=;

%typemap(out) CEC::cec_osd_name {
  $result = PyString_FromString($1.name);
}

/////// libcec_configuration::strDeviceLanguage ///////
// strDeviceLanguage is a raw, fixed-size char[3] that is NOT NUL-terminated (a
// 3-character ISO 639-2 code). SWIG's default char[ANY] setter reserves a byte
// for a terminator, so a valid 3-char code needs 4 bytes and overflows the
// buffer, raising a TypeError; only 2-char codes fit. Copy the raw bytes
// instead (accepting str or bytes), scoped to this member by name so the
// NUL-terminated strDeviceName keeps the default behaviour.
%typemap(in) char strDeviceLanguage[ANY] (char temp[$1_dim0]) {
  char *cptr = 0; size_t csize = 0; int alloc = SWIG_OLDOBJ;
  int res = SWIG_AsCharPtrAndSize($input, &cptr, &csize, &alloc);
  if (!SWIG_IsOK(res)) {
    %argument_fail(res, "char [$1_dim0]", $symname, $argnum);
  }
  {
    /* csize counts the trailing NUL; strDeviceLanguage stores no terminator */
    size_t slen = (csize > 0) ? csize - 1 : 0;
    if (slen > (size_t)($1_dim0)) {
      if (alloc == SWIG_NEWOBJ) %delete_array(cptr);
      %argument_fail(SWIG_ValueError, "char [$1_dim0]", $symname, $argnum);
    }
    memset(temp, 0, $1_dim0);
    if (slen) memcpy(temp, cptr, slen);
  }
  $1 = temp;
  if (alloc == SWIG_NEWOBJ) %delete_array(cptr);
}

%typemap(memberin) char strDeviceLanguage[ANY] {
  memcpy($1, $input, $1_dim0 * sizeof(char));
}

/////// replace operator[]() ///////

// CEC::cec_datapacket::operator[]()
%extend CEC::cec_datapacket {
 public:
  uint8_t __getitem__(uint8_t pos)
  {
    return (*($self))[pos];
  }
}

// CEC::cec_device_type::operator[]()
%extend CEC::cec_device_type_list {
 public:
  CEC::cec_device_type __getitem__(uint8_t pos)
  {
    return (*($self))[pos];
  }
}

// CEC::cec_logical_addresses::operator[]()
%extend CEC::cec_logical_addresses {
 public:
  bool __getitem__(uint8_t pos)
  {
    return (*($self))[pos];
  }
}
%ignore *::operator[];

/////// rename ToString() ///////
%rename("MenuStateToString") ToString(const cec_menu_state);
%rename("CecVersionToString") ToString(const cec_version);
%rename("PowerStatusToString") ToString(const cec_power_status);
%rename("LogicalAddressToString") ToString(const cec_logical_address);
%rename("DeckControlModeToString") ToString(const cec_deck_control_mode);
%rename("DeckInfoToString") ToString(const cec_deck_info);
%rename("OpcodeToString") ToString(const cec_opcode);
%rename("AudioStatusToString") ToString(const cec_audio_status);
%ignore *::ToString(const cec_vendor_id);
%rename("DeviceTypeToString") ToString(const cec_device_type);
%rename("UserControlCodeToString") ToString(const cec_user_control_code);
%rename("AdapterTypeToString") ToString(const cec_adapter_type);

/////// callbacks ///////
%extend CEC::libcec_configuration {
 public:
  virtual ~libcec_configuration(void)
  {
    _ClearCallbacks(self);
    self->Clear();
  }

  void SetLogCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_LOG_MESSAGE, pyfunc);
  }

  void SetKeyPressCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_KEY_PRESS, pyfunc);
  }

  void SetCommandCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_COMMAND, pyfunc);
  }

  void SetMenuStateCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_MENU_STATE, pyfunc);
  }

  void SetSourceActivatedCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_SOURCE_ACTIVATED, pyfunc);
  }

  void SetAlertCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_ALERT, pyfunc);
  }

  void SetConfigurationChangedCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_CONFIGURATION, pyfunc);
  }

  void SetCommandHandlerCallback(PyObject* pyfunc)
  {
    _SetCallback(self, CEC::PYTHON_CB_COMMAND_HANDLER, pyfunc);
  }

  void ClearCallbacks(void)
  {
    _ClearCallbacks(self);
  }
}

%ignore CEC::libcec_configuration::~libcec_configuration;
%ignore CEC::libcec_configuration::callbacks;
%ignore CEC::libcec_configuration::callbackParam;

namespace std {
  %template(AdapterVector) vector<CEC::AdapterDescriptor>;
}

/////// replace CECInitialise(), CECDestroy() and DetectAdapters() ///////

%extend CEC::ICECAdapter {
  public:
    virtual ~ICECAdapter(void)
    {
      CEC::libcec_configuration config;
      if (self->GetCurrentConfiguration(&config))
      {
        _ClearCallbacks(&config);
        %#if CEC_LIB_VERSION_MAJOR >= 5
        self->DisableCallbacks();
        %#else
        self->EnableCallbacks(NULL, NULL);
        %#endif
      }
    }

    static CEC::ICECAdapter* Create(CEC::libcec_configuration* configuration)
    {
      CEC::ICECAdapter* lib = CECInitialise(configuration);
      if (!!lib)
      {
        lib->InitVideoStandalone();
      }
      return lib;
    }

    std::vector<CEC::AdapterDescriptor> DetectAdapters(const char *strDevicePath = NULL, bool bQuickScan = false)
    {
      std::vector<CEC::AdapterDescriptor> retval;
      CEC::cec_adapter_descriptor devList[10];
      int nbAdapters = self->DetectAdapters(devList, 10, strDevicePath, bQuickScan);
      for (int adapter = 0; adapter < nbAdapters; ++adapter)
        retval.push_back(CEC::AdapterDescriptor(devList[adapter]));
      return retval;
    }
}

%ignore CEC::ICECAdapter::~ICECAdapter;
%ignore CEC::ICECAdapter::SetCallbacks;
%ignore CEC::ICECAdapter::EnableCallbacks;
%ignore CEC::ICECAdapter::CanPersistConfiguration;
%ignore CEC::ICECAdapter::PersistConfiguration;
%ignore CEC::ICECCallbacks;
%ignore CEC::DetectAdapters;
%ignore CEC::GetDeviceMenuLanguage;

%ignore CEC::cec_keypress;
%ignore CEC::cec_log_message;
%ignore CEC::cec_adapter_descriptor;
%ignore CEC::cec_adapter;
%ignore CECInitialise;
%ignore CECDestroy;

%include "cectypes.h"
%include "cec.h"

%ignore *::ToString;
