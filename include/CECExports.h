#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011 Pulse-Eight Limited.  All rights reserved.
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

#ifndef CECEXPORTS_H_
#define CECEXPORTS_H_

#include <stdint.h>
#include <string.h>

#if !defined(DECLSPEC)
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#if defined DLL_EXPORT
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport)
#endif
#else
#define DECLSPEC
#endif
#endif

#ifdef __cplusplus
extern "C" {
namespace CEC {
#endif
  #define CEC_MIN_VERSION      5
  #define CEC_LIB_VERSION      5
  #define CEC_SETTLE_DOWN_TIME 1000
  #define CEC_BUTTON_TIMEOUT   500

  typedef enum cec_user_control_code
  {
    CEC_USER_CONTROL_CODE_SELECT = 0x00,
    CEC_USER_CONTROL_CODE_UP = 0x01,
    CEC_USER_CONTROL_CODE_DOWN = 0x02,
    CEC_USER_CONTROL_CODE_LEFT = 0x03,
    CEC_USER_CONTROL_CODE_RIGHT = 0x04,
    CEC_USER_CONTROL_CODE_RIGHT_UP = 0x05,
    CEC_USER_CONTROL_CODE_RIGHT_DOWN = 0x06,
    CEC_USER_CONTROL_CODE_LEFT_UP = 0x07,
    CEC_USER_CONTROL_CODE_LEFT_DOWN = 0x08,
    CEC_USER_CONTROL_CODE_ROOT_MENU = 0x09,
    CEC_USER_CONTROL_CODE_SETUP_MENU = 0x0A,
    CEC_USER_CONTROL_CODE_CONTENTS_MENU = 0x0B,
    CEC_USER_CONTROL_CODE_FAVORITE_MENU = 0x0C,
    CEC_USER_CONTROL_CODE_EXIT = 0x0D,
    CEC_USER_CONTROL_CODE_NUMBER0 = 0x20,
    CEC_USER_CONTROL_CODE_NUMBER1 = 0x21,
    CEC_USER_CONTROL_CODE_NUMBER2 = 0x22,
    CEC_USER_CONTROL_CODE_NUMBER3 = 0x23,
    CEC_USER_CONTROL_CODE_NUMBER4 = 0x24,
    CEC_USER_CONTROL_CODE_NUMBER5 = 0x25,
    CEC_USER_CONTROL_CODE_NUMBER6 = 0x26,
    CEC_USER_CONTROL_CODE_NUMBER7 = 0x27,
    CEC_USER_CONTROL_CODE_NUMBER8 = 0x28,
    CEC_USER_CONTROL_CODE_NUMBER9 = 0x29,
    CEC_USER_CONTROL_CODE_DOT = 0x2A,
    CEC_USER_CONTROL_CODE_ENTER = 0x2B,
    CEC_USER_CONTROL_CODE_CLEAR = 0x2C,
    CEC_USER_CONTROL_CODE_NEXT_FAVORITE = 0x2F,
    CEC_USER_CONTROL_CODE_CHANNEL_UP = 0x30,
    CEC_USER_CONTROL_CODE_CHANNEL_DOWN = 0x31,
    CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL = 0x32,
    CEC_USER_CONTROL_CODE_SOUND_SELECT = 0x33,
    CEC_USER_CONTROL_CODE_INPUT_SELECT = 0x34,
    CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION = 0x35,
    CEC_USER_CONTROL_CODE_HELP = 0x36,
    CEC_USER_CONTROL_CODE_PAGE_UP = 0x37,
    CEC_USER_CONTROL_CODE_PAGE_DOWN = 0x38,
    CEC_USER_CONTROL_CODE_POWER = 0x40,
    CEC_USER_CONTROL_CODE_VOLUME_UP = 0x41,
    CEC_USER_CONTROL_CODE_VOLUME_DOWN = 0x42,
    CEC_USER_CONTROL_CODE_MUTE = 0x43,
    CEC_USER_CONTROL_CODE_PLAY = 0x44,
    CEC_USER_CONTROL_CODE_STOP = 0x45,
    CEC_USER_CONTROL_CODE_PAUSE = 0x46,
    CEC_USER_CONTROL_CODE_RECORD = 0x47,
    CEC_USER_CONTROL_CODE_REWIND = 0x48,
    CEC_USER_CONTROL_CODE_FAST_FORWARD = 0x49,
    CEC_USER_CONTROL_CODE_EJECT = 0x4A,
    CEC_USER_CONTROL_CODE_FORWARD = 0x4B,
    CEC_USER_CONTROL_CODE_BACKWARD = 0x4C,
    CEC_USER_CONTROL_CODE_STOP_RECORD = 0x4D,
    CEC_USER_CONTROL_CODE_PAUSE_RECORD = 0x4E,
    CEC_USER_CONTROL_CODE_ANGLE = 0x50,
    CEC_USER_CONTROL_CODE_SUB_PICTURE = 0x51,
    CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND = 0x52,
    CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE = 0x53,
    CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING = 0x54,
    CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION = 0x55,
    CEC_USER_CONTROL_CODE_PLAY_FUNCTION = 0x60,
    CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION = 0x61,
    CEC_USER_CONTROL_CODE_RECORD_FUNCTION = 0x62,
    CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION = 0x63,
    CEC_USER_CONTROL_CODE_STOP_FUNCTION = 0x64,
    CEC_USER_CONTROL_CODE_MUTE_FUNCTION = 0x65,
    CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION = 0x66,
    CEC_USER_CONTROL_CODE_TUNE_FUNCTION = 0x67,
    CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION = 0x68,
    CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION = 0x69,
    CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION = 0x6A,
    CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION = 0x6B,
    CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION = 0x6C,
    CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION = 0x6D,
    CEC_USER_CONTROL_CODE_F1_BLUE = 0x71,
    CEC_USER_CONTROL_CODE_F2_RED = 0X72,
    CEC_USER_CONTROL_CODE_F3_GREEN = 0x73,
    CEC_USER_CONTROL_CODE_F4_YELLOW = 0x74,
    CEC_USER_CONTROL_CODE_F5 = 0x75,
    CEC_USER_CONTROL_CODE_DATA = 0x76,
    CEC_USER_CONTROL_CODE_MAX = 0x76,
    CEC_USER_CONTROL_CODE_UNKNOWN
  } cec_user_control_code;

  typedef enum cec_logical_address
  {
    CECDEVICE_UNKNOWN = -1, //not a valid logical address
    CECDEVICE_TV,
    CECDEVICE_RECORDINGDEVICE1,
    CECDEVICE_RECORDINGDEVICE2,
    CECDEVICE_TUNER1,
    CECDEVICE_PLAYBACKDEVICE1,
    CECDEVICE_AUDIOSYSTEM,
    CECDEVICE_TUNER2,
    CECDEVICE_TUNER3,
    CECDEVICE_PLAYBACKDEVICE2,
    CECDEVICE_RECORDINGDEVICE3,
    CECDEVICE_TUNER4,
    CECDEVICE_PLAYBACKDEVICE3,
    CECDEVICE_RESERVED1,
    CECDEVICE_RESERVED2,
    CECDEVICE_FREEUSE,
    CECDEVICE_UNREGISTERED = 15,
    CECDEVICE_BROADCAST = 15
  } cec_logical_address;

  typedef enum cec_opcode
  {
    CEC_OPCODE_ACTIVE_SOURCE = 0x82,
    CEC_OPCODE_IMAGE_VIEW_ON = 0x04,
    CEC_OPCODE_TEXT_VIEW_ON = 0x0D,
    CEC_OPCODE_INACTIVE_SOURCE = 0x9D,
    CEC_OPCODE_REQUEST_ACTIVE_SOURCE = 0x85,
    CEC_OPCODE_ROUTING_CHANGE = 0x80,
    CEC_OPCODE_ROUTING_INFORMATION = 0x81,
    CEC_OPCODE_SET_STREAM_PATH = 0x86,
    CEC_OPCODE_STANDBY = 0x36,
    CEC_OPCODE_RECORD_OFF = 0x0B,
    CEC_OPCODE_RECORD_ON = 0x09,
    CEC_OPCODE_RECORD_STATUS = 0x0A,
    CEC_OPCODE_RECORD_TV_SCREEN = 0x0F,
    CEC_OPCODE_CLEAR_ANALOGUE_TIMER = 0x33,
    CEC_OPCODE_CLEAR_DIGITAL_TIMER = 0x99,
    CEC_OPCODE_CLEAR_EXTERNAL_TIMER = 0xA1,
    CEC_OPCODE_SET_ANALOGUE_TIMER = 0x34,
    CEC_OPCODE_SET_DIGITAL_TIMER = 0x97,
    CEC_OPCODE_SET_EXTERNAL_TIMER = 0xA2,
    CEC_OPCODE_SET_TIMER_PROGRAM_TITLE = 0x67,
    CEC_OPCODE_TIMER_CLEARED_STATUS = 0x43,
    CEC_OPCODE_TIMER_STATUS = 0x35,
    CEC_OPCODE_CEC_VERSION = 0x9E,
    CEC_OPCODE_GET_CEC_VERSION = 0x9F,
    CEC_OPCODE_GIVE_PHYSICAL_ADDRESS = 0x83,
    CEC_OPCODE_GET_MENU_LANGUAGE = 0x91,
    CEC_OPCODE_REPORT_PHYSICAL_ADDRESS = 0x84,
    CEC_OPCODE_SET_MENU_LANGUAGE = 0x32,
    CEC_OPCODE_DECK_CONTROL = 0x42,
    CEC_OPCODE_DECK_STATUS = 0x1B,
    CEC_OPCODE_GIVE_DECK_STATUS = 0x1A,
    CEC_OPCODE_PLAY = 0x41,
    CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS = 0x08,
    CEC_OPCODE_SELECT_ANALOGUE_SERVICE = 0x92,
    CEC_OPCODE_SELECT_DIGITAL_SERVICE = 0x93,
    CEC_OPCODE_TUNER_DEVICE_STATUS = 0x07,
    CEC_OPCODE_TUNER_STEP_DECREMENT = 0x06,
    CEC_OPCODE_TUNER_STEP_INCREMENT = 0x05,
    CEC_OPCODE_DEVICE_VENDOR_ID = 0x87,
    CEC_OPCODE_GIVE_DEVICE_VENDOR_ID = 0x8C,
    CEC_OPCODE_VENDOR_COMMAND = 0x89,
    CEC_OPCODE_VENDOR_COMMAND_WITH_ID = 0xA0,
    CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN = 0x8A,
    CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP = 0x8B,
    CEC_OPCODE_SET_OSD_STRING = 0x64,
    CEC_OPCODE_GIVE_OSD_NAME = 0x46,
    CEC_OPCODE_SET_OSD_NAME = 0x47,
    CEC_OPCODE_MENU_REQUEST = 0x8D,
    CEC_OPCODE_MENU_STATUS = 0x8E,
    CEC_OPCODE_USER_CONTROL_PRESSED = 0x44,
    CEC_OPCODE_USER_CONTROL_RELEASE = 0x45,
    CEC_OPCODE_GIVE_DEVICE_POWER_STATUS = 0x8F,
    CEC_OPCODE_REPORT_POWER_STATUS = 0x90,
    CEC_OPCODE_FEATURE_ABORT = 0x00,
    CEC_OPCODE_ABORT = 0xFF,
    CEC_OPCODE_GIVE_AUDIO_STATUS = 0x71,
    CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS = 0x7D,
    CEC_OPCODE_REPORT_AUDIO_STATUS = 0x7A,
    CEC_OPCODE_SET_SYSTEM_AUDIO_MODE = 0x72,
    CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST = 0x70,
    CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS = 0x7E,
    CEC_OPCODE_SET_AUDIO_RATE = 0x9A
  } cec_opcode;

  typedef enum cec_log_level
  {
    CEC_LOG_DEBUG = 0,
    CEC_LOG_NOTICE,
    CEC_LOG_WARNING,
    CEC_LOG_ERROR
  } cec_log_level;

  typedef struct cec_log_message
  {
    char          message[1024];
    cec_log_level level;
    int64_t       time;
  } cec_log_message;

  typedef struct cec_keypress
  {
    cec_user_control_code keycode;
    unsigned int          duration;
  } cec_keypress;

  typedef struct cec_adapter
  {
    char path[1024];
    char comm[1024];
  } cec_adapter;

  typedef struct cec_frame
  {
    uint8_t data[20];
    uint8_t size;

    void shift(uint8_t iShiftBy)
    {
      if (iShiftBy >= size)
      {
        clear();
      }
      else
      {
        for (uint8_t iPtr = 0; iPtr < size; iPtr++)
          data[iPtr] = (iPtr + iShiftBy < size) ? data[iPtr + iShiftBy] : 0;
        size = (uint8_t) (size - iShiftBy);
      }
    }

    void push_back(uint8_t add)
    {
      if (size < 20)
        data[size++] = add;
    }

    void clear(void)
    {
      memset(data, 0, sizeof(data));
      size = 0;
    }
  } cec_frame;

  typedef struct cec_command
  {
    cec_logical_address source;
    cec_logical_address destination;
    cec_opcode          opcode;
    cec_frame           parameters;

    void clear(void)
    {
      source      = CECDEVICE_UNKNOWN;
      destination = CECDEVICE_UNKNOWN;
      opcode      = CEC_OPCODE_FEATURE_ABORT;
      parameters.clear();
    };
  } cec_command;

  typedef enum cec_vendor_id
  {
    CEC_VENDOR_SAMSUNG = 240,
    CEC_VENDOR_UNKNOWN = 0
  } vendor_id;

  static const char *CECVendorIdToString(const uint64_t iVendorId)
  {
    switch (iVendorId)
    {
    case CEC_VENDOR_SAMSUNG:
      return "Samsung";
    default:
      return "Unknown";
    }
  }

  //default physical address 1.0.0.0
  #define CEC_DEFAULT_PHYSICAL_ADDRESS 0x1000

#ifdef __cplusplus
};

#include "CECExportsCpp.h"
#include "CECExportsC.h"
};
#else
#include "CECExportsC.h"
#endif

#endif /* CECEXPORTS_H_ */
