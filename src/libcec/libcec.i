%module cec

%{
#define SWIG_FILE_WITH_INIT

#include "cectypes.h"
#include "cec.h"
#include "SwigHelper.h"
%}

%include "stdint.i"
%include "cstring.i"
%include "std_string.i"

%ignore *::operator=;
%ignore *::operator[];

%include "cectypes.h"

// CEC::cec_datapacket::operator[]()
%extend CEC::cec_datapacket {
  uint8_t __getitem__(uint8_t pos)
  {
    return (*($self))[pos];
  }
}

// CEC::cec_device_type::operator[]()
%extend CEC::cec_device_type_list {
  CEC::cec_device_type __getitem__(uint8_t pos)
  {
    return (*($self))[pos];
  }
}

// CEC::cec_logical_addresses::operator[]()
%extend CEC::cec_logical_addresses {
  bool __getitem__(uint8_t pos)
  {
    return (*($self))[pos];
  }
}

// rename ToString() methods
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

%include "cec.h"

// callbacks
%extend CEC::libcec_configuration {
 public:
  void SetLogCallback(PyObject* pyfunc)
  {
    _SetCallback(self, PYTHON_CB_LOG_MESSAGE, pyfunc);
  }

  void SetKeyPressCallback(PyObject* pyfunc)
  {
    _SetCallback(self, PYTHON_CB_KEY_PRESS, pyfunc);
  }

  void SetCommandCallback(PyObject* pyfunc)
  {
    _SetCallback(self, PYTHON_CB_COMMAND, pyfunc);
  }

  void SetMenuStateCallback(PyObject* pyfunc)
  {
    _SetCallback(self, PYTHON_CB_MENU_STATE, pyfunc);
  }

  void SetSourceActivatedCallback(PyObject* pyfunc)
  {
    _SetCallback(self, PYTHON_CB_SOURCE_ACTIVATED, pyfunc);
  }

  void ClearCallbacks(void)
  {
    _ClearCallbacks(self);
  }
}
