#pragma once

#include "platform/threads/mutex.h"

enum libcecSwigCallback {
  PYTHON_CB_LOG_MESSAGE,
  PYTHON_CB_KEY_PRESS,
  PYTHON_CB_COMMAND,
  PYTHON_CB_ALERT,
  PYTHON_CB_MENU_STATE,
  PYTHON_CB_SOURCE_ACTIVATED,
  NB_PYTHON_CB,
};

class CCecPythonCallbacks
{
public:
  CCecPythonCallbacks(void)
  {
    memset(callbacks, 0, sizeof(callbacks));
  }

  ~CCecPythonCallbacks(void)
  {
    for (size_t ptr = 0; ptr < NB_PYTHON_CB; ++ptr)
      if (callbacks[ptr])
        Py_XDECREF(static_cast<PyObject*>(callbacks[ptr]));
  }

  int CallPythonCallback(enum libcecSwigCallback callback, PyObject* arglist)
  {
    PLATFORM::CLockObject lock(mutex);
    if (!callbacks[callback])
      return 0;
    PyObject* func = static_cast<PyObject*>(callbacks[callback]);
    PyObject* result = NULL;
    if (func && arglist)
    {
      result = PyEval_CallObject(func, arglist);
      Py_DECREF(arglist);
      if (result)
        Py_XDECREF(result);
    }
    return 1;
  }

  void* callbacks[NB_PYTHON_CB];

private:
  PLATFORM::CMutex mutex;
};

static int _callPythonCallback(void* param, enum libcecSwigCallback callback, PyObject* arglist)
{
  CCecPythonCallbacks* pCallbacks = static_cast<CCecPythonCallbacks*>(param);
  return pCallbacks ?
    pCallbacks->CallPythonCallback(callback, arglist) :
    0;
}

static int _cb_cec_log_message(void* param, const CEC::cec_log_message message)
{
  return _callPythonCallback(param, PYTHON_CB_LOG_MESSAGE, Py_BuildValue("Iks", message.level, message.time, message.message));
}

static int _cb_cec_key_press(void* param, const CEC::cec_keypress key)
{
  return _callPythonCallback(param, PYTHON_CB_KEY_PRESS, Py_BuildValue("II", key.keycode, key.duration));
}

static int _cb_command(void* param, const CEC::cec_command command)
{
  //TODO
  (void)command;
  return _callPythonCallback(param, PYTHON_CB_COMMAND, Py_BuildValue("s", "TODO"));
}

static int _cb_menu_state_changed(void* param, const CEC::cec_menu_state state)
{
  return _callPythonCallback(param, PYTHON_CB_MENU_STATE, Py_BuildValue("i", state));
}

static void _cb_cec_source_activated(void* param, const CEC::cec_logical_address logicalAddress, const uint8_t activated)
{
  _callPythonCallback(param, PYTHON_CB_SOURCE_ACTIVATED, Py_BuildValue("Ii", logicalAddress, activated ? 1 : 0));
}

static void _SetCallback(CEC::libcec_configuration* self, size_t cb, PyObject* pyfunc)
{
  if (!PyCallable_Check(pyfunc))
    return;
  if (!self->callbacks)
  {
    self->callbacks = new CEC::ICECCallbacks;
    if (!self->callbacks) return;

    self->callbacks->CBCecLogMessage       = _cb_cec_log_message;
    self->callbacks->CBCecKeyPress         = _cb_cec_key_press;
    self->callbacks->CBCecCommand          = _cb_command;
    self->callbacks->CBCecMenuStateChanged = _cb_menu_state_changed;
    self->callbacks->CBCecSourceActivated  = _cb_cec_source_activated;
  }

  if (!self->callbackParam)
  {
    self->callbackParam = new CCecPythonCallbacks;
    if (!self->callbackParam)
    {
      delete self->callbacks;
      self->callbacks = NULL;
      return;
    }
  }

  CCecPythonCallbacks* pCallbacks = static_cast<CCecPythonCallbacks*>(self->callbackParam);
  pCallbacks->callbacks[cb] = (void*)pyfunc;
  Py_XINCREF(pyfunc);
}

void _ClearCallbacks(CEC::libcec_configuration* self)
{
  if (self->callbacks)
    delete self->callbacks;
  self->callbacks = NULL;
  CCecPythonCallbacks* pCallbacks = static_cast<CCecPythonCallbacks*>(self->callbackParam);
  if (pCallbacks)
    delete pCallbacks;
  self->callbackParam = NULL;
  self->Clear();
}

