// Grab a Python function object as a Python object.
%typemap(python, in) PyObject *pyfunc
{
  if (!PyCallable_Check($source))
  {
    PyErr_SetString(PyExc_TypeError, "Need a callable object!");
    return NULL;
  }
  $target = $source;
}

%module cec

%include "stdint.i"
%include "cstring.i"

%{
#define SWIG_FILE_WITH_INIT
#include "cectypes.h"
#include "cecc.h"

enum {
  PYTHON_CB_LOG_MESSAGE,
  PYTHON_CB_KEY_PRESS,
  PYTHON_CB_COMMAND,
  PYTHON_CB_ALERT,
  PYTHON_CB_MENU_STATE,
  PYTHON_CB_SOURCE_ACTIVATED,
  NB_PYTHON_CB,
};

// TODO get rid of the globals
static ICECCallbacks g_callbacks;
static PyObject* g_python_callbacks[NB_PYTHON_CB] = { NULL, NULL, NULL, NULL, NULL, NULL };

%}

%include "cectypes.h"
%include "cecc.h"

%extend libcec_configuration {
  static int cb_cec_log_message(void* param, const cec_log_message message)
  {
    PyObject *func, *arglist;
    PyObject *result;

    func = ((PyObject**)param)[PYTHON_CB_LOG_MESSAGE];
    arglist = Py_BuildValue("IIs", message.level, message.time, message.message);
    if (func && arglist)
    {
      result = PyEval_CallObject(func, arglist);
      Py_DECREF(arglist);
      Py_XDECREF(result);
    }
    return 1;
  }

  static int cb_cec_key_press(void* param, const cec_keypress key)
  {
    PyObject *func, *arglist;
    PyObject *result;

    func = ((PyObject**)param)[PYTHON_CB_KEY_PRESS];
    arglist = Py_BuildValue("II", key.keycode, key.duration);
    if (func && arglist)
    {
      result = PyEval_CallObject(func, arglist);
      Py_DECREF(arglist);
      Py_XDECREF(result);
    }
    return 1;
  }

  static int cb_command(void* param, const cec_command command)
  {
    PyObject *func, *arglist;
    PyObject *result;

    func = ((PyObject**)param)[PYTHON_CB_COMMAND];
    arglist = Py_BuildValue("s", "TODO");
    if (func && arglist)
    {
      result = PyEval_CallObject(func, arglist);
      Py_DECREF(arglist);
      Py_XDECREF(result);
    }
    return 1;
  }

  static int cb_menu_state_changed(void* param, const cec_menu_state state)
  {
    PyObject *func, *arglist;
    PyObject *result;

    func = ((PyObject**)param)[PYTHON_CB_MENU_STATE];
    arglist = Py_BuildValue("i", state);
    if (func && arglist)
    {
      result = PyEval_CallObject(func, arglist);
      Py_DECREF(arglist);
      Py_XDECREF(result);
    }
    return 1;
  }

  static void cb_cec_source_activated(void* param, const cec_logical_address logicalAddress, const uint8_t activated)
  {
    PyObject *func, *arglist;
    PyObject *result;

    func = ((PyObject**)param)[PYTHON_CB_SOURCE_ACTIVATED];
    arglist = Py_BuildValue("Ii", logicalAddress, activated ? 1 : 0);
    if (func && arglist)
    {
      result = PyEval_CallObject(func, arglist);
      Py_DECREF(arglist);
      Py_XDECREF(result);
    }
  }

  // Set a Python function object as a callback function
  // Note : PyObject *pyfunc is remapped with a typempap
  void set_log_callback(PyObject* pyfunc)
  {
    self->callbacks = &g_callbacks;
    g_python_callbacks[PYTHON_CB_LOG_MESSAGE] = pyfunc;
    self->callbacks->CBCecLogMessage = libcec_configuration_cb_cec_log_message;
    self->callbackParam = (void*)g_python_callbacks;
    Py_INCREF(pyfunc);
  }

  void set_key_press_callback(PyObject* pyfunc)
  {
    self->callbacks = &g_callbacks;
    g_python_callbacks[PYTHON_CB_KEY_PRESS] = pyfunc;
    self->callbacks->CBCecKeyPress = libcec_configuration_cb_cec_key_press;
    self->callbackParam = (void*)g_python_callbacks;
    Py_INCREF(pyfunc);
  }

  void set_command_callback(PyObject* pyfunc)
  {
    self->callbacks = &g_callbacks;
    g_python_callbacks[PYTHON_CB_COMMAND] = pyfunc;
    self->callbacks->CBCecCommand = libcec_configuration_cb_command;
    self->callbackParam = (void*)g_python_callbacks;
    Py_INCREF(pyfunc);
  }

  void set_menu_state_callback(PyObject* pyfunc)
  {
    self->callbacks = &g_callbacks;
    g_python_callbacks[PYTHON_CB_MENU_STATE] = pyfunc;
    self->callbacks->CBCecMenuStateChanged = libcec_configuration_cb_menu_state_changed;
    self->callbackParam = (void*)g_python_callbacks;
    Py_INCREF(pyfunc);
  }

  void set_source_activated_callback(PyObject* pyfunc)
  {
    self->callbacks = &g_callbacks;
    g_python_callbacks[PYTHON_CB_SOURCE_ACTIVATED] = pyfunc;
    self->callbacks->CBCecSourceActivated = libcec_configuration_cb_cec_source_activated;
    self->callbackParam = (void*)g_python_callbacks;
    Py_INCREF(pyfunc);
  }
}
