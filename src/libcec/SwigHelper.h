#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2015 Pulse-Eight Limited.  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
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

#define SWIG_FILE_WITH_INIT
#define SWIG_PYTHON_THREADS
#define SWIG_PYTHON_USE_GIL
#define LIBCEC_SWIG_EXPORTS

#include "cectypes.h"
#include "cec.h"
#include "CECTypeUtils.h"
#include "p8-platform/threads/mutex.h"
/** XXX only to keep the IDE happy, using the actual Python.h with the correct system version when building */
#ifndef Py_PYTHON_H
#include <python2.7/Python.h>
#include <assert.h>
#endif

namespace CEC
{
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
    /**
     * Create a new python callbacks instance, and set callbacks in the configuration
     * @param config    the configuration to set the callbacks in
     */
    CCecPythonCallbacks(libcec_configuration* config) :
      m_configuration(config)
    {
      assert(m_configuration);

      config->callbacks = new ICECCallbacks;
      if (!config->callbacks)
        throw std::bad_alloc();

      for (size_t ptr = 0; ptr < NB_PYTHON_CB; ++ptr)
        m_callbacks[ptr] = NULL;

      m_configuration->callbacks->logMessage       = CBCecLogMessage;
      m_configuration->callbacks->keyPress         = CBCecKeyPress;
      m_configuration->callbacks->commandReceived  = CBCecCommand;
      m_configuration->callbacks->menuStateChanged = CBCecMenuStateChanged;
      m_configuration->callbacks->sourceActivated  = CBCecSourceActivated;
    }

    /**
     * Unreferences all python callbacks, and deletes the callbacks
     */
    virtual ~CCecPythonCallbacks(void)
    {
      for (size_t ptr = 0; ptr < NB_PYTHON_CB; ++ptr)
        if (m_callbacks[ptr])
          Py_XDECREF(m_callbacks[ptr]);
      delete m_configuration->callbacks;
      m_configuration->callbacks = nullptr;
    }

    /**
     * Call a python callback (if set)
     * @param callback  the callback to call
     * @param arglist   the arguments to pass to the callback
     * @return 0 if the callback failed, the result returned by python otherwise
     */
    int CallPythonCallback(enum libcecSwigCallback callback, PyObject* arglist)
    {
      int retval = 0;

      if (callback >= NB_PYTHON_CB || !m_callbacks[callback])
        return retval;

      PyObject* result = NULL;
      if (m_callbacks[callback])
      {
        /** call the callback */
        result = PyEval_CallObject(m_callbacks[callback], arglist);

        /** unref the argument and result */
        if (!!arglist)
          Py_DECREF(arglist);
        if (!!result)
        {
          if (PyInt_Check(result))
            retval = (int)PyInt_AsLong(result);
          Py_XDECREF(result);
        }
      }

      return retval;
    }

    /**
     * Set a python callable as callback
     * @param cb      the callback to set
     * @param pyfunc  the python callable to call
     */
    void SetCallback(size_t cb, PyObject* pyfunc)
    {
      assert(cb < NB_PYTHON_CB);
      assert(PyCallable_Check(pyfunc));

      /** unref previous callback (if any) */
      if (m_callbacks[cb])
        Py_XDECREF(m_callbacks[cb]);

      /** set the new callback */
      m_callbacks[cb] = pyfunc;
      Py_XINCREF(pyfunc);
    }

  private:
    static inline int CallPythonCallback(void* param, enum libcecSwigCallback callback, PyObject* arglist)
    {
      CCecPythonCallbacks* pCallbacks = static_cast<CCecPythonCallbacks*>(param);
      return pCallbacks ?
        pCallbacks->CallPythonCallback(callback, arglist) :
        0;
    }

    static void CBCecLogMessage(void* param, const CEC::cec_log_message* message)
    {
      PyGILState_STATE gstate = PyGILState_Ensure();
      PyObject* arglist = Py_BuildValue("(I,I,s)", message->level, (long)message->time, message->message);
      CallPythonCallback(param, PYTHON_CB_LOG_MESSAGE, arglist);
      PyGILState_Release(gstate);
    }

    static void CBCecKeyPress(void* param, const CEC::cec_keypress* key)
    {
      PyGILState_STATE gstate = PyGILState_Ensure();
      CallPythonCallback(param, PYTHON_CB_KEY_PRESS,
                                      Py_BuildValue("(I,I)", (long)key->keycode, (long)key->duration));
      PyGILState_Release(gstate);
    }

    static void CBCecCommand(void* param, const CEC::cec_command* command)
    {
      PyGILState_STATE gstate = PyGILState_Ensure();
      CallPythonCallback(param, PYTHON_CB_COMMAND,
                                      Py_BuildValue("(s)", CEC::CCECTypeUtils::ToString(*command).c_str()));
      PyGILState_Release(gstate);
    }

    static int CBCecMenuStateChanged(void* param, const CEC::cec_menu_state state)
    {
      PyGILState_STATE gstate = PyGILState_Ensure();
      int retval = CallPythonCallback(param, PYTHON_CB_MENU_STATE,
                                      Py_BuildValue("(I)", state));
      PyGILState_Release(gstate);
      return retval;
    }

    static void CBCecSourceActivated(void* param, const CEC::cec_logical_address logicalAddress, const uint8_t activated)
    {
      PyGILState_STATE gstate = PyGILState_Ensure();
      CallPythonCallback(param, PYTHON_CB_SOURCE_ACTIVATED,
                         Py_BuildValue("(I,I)", logicalAddress, activated));
      PyGILState_Release(gstate);
    }

    PyObject*             m_callbacks[NB_PYTHON_CB];
    libcec_configuration* m_configuration;
  };

  static CCecPythonCallbacks* _GetCallbacks(CEC::libcec_configuration* self)
  {
    /** allocate callbacks struct and python callbacks if needed */
    if (!self->callbackParam)
      self->callbackParam = new CCecPythonCallbacks(self);

    return static_cast<CCecPythonCallbacks*>(self->callbackParam);
  }
}

static void _SetCallback(CEC::libcec_configuration* self, size_t cb, PyObject* pyfunc)
{
  assert(self);
  CEC::CCecPythonCallbacks* pCallbacks = CEC::_GetCallbacks(self);
  if (pCallbacks)
    pCallbacks->SetCallback(cb, pyfunc);
  else
    printf("ERROR: cannot set callback to %p: out of memory\n", pyfunc);
}

void _ClearCallbacks(CEC::libcec_configuration* self)
{
  CEC::CCecPythonCallbacks* pCallbacks = static_cast<CEC::CCecPythonCallbacks*>(self->callbackParam);
  if (pCallbacks)
    delete pCallbacks;
  self->callbackParam = NULL;
}

