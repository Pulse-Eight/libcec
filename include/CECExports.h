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

#include <string>
#include <stdint.h>
#include <vector>

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
  #define CEC_MIN_VERSION      1
  #define CEC_LIB_VERSION      1
  #define CEC_SETTLE_DOWN_TIME 1000

  typedef std::vector<uint8_t> cec_frame;

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

  typedef enum cec_log_level
  {
    CEC_LOG_DEBUG = 0,
    CEC_LOG_NOTICE,
    CEC_LOG_WARNING,
    CEC_LOG_ERROR
  } cec_log_level;

  typedef struct cec_log_message
  {
    std::string   message;
    cec_log_level level;
  } cec_log_message;

  typedef struct cec_keypress
  {
    cec_user_control_code keycode;
    unsigned int          duration;
  } cec_keypress;

  typedef struct cec_device
  {
    std::string path;
    std::string comm;
  } cec_device;
};

#ifdef __cplusplus
#include "CECExportsCpp.h"
#include "CECExportsC.h"
};
#else
#include "CECExportsC.h"
#endif

#endif /* CECEXPORTS_H_ */
