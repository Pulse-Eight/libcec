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

#ifndef CECTYPES_H_
#define CECTYPES_H_

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

//default physical address 1.0.0.0, HDMI port 1
#define CEC_DEFAULT_PHYSICAL_ADDRESS 0x1000
#define CEC_DEFAULT_HDMI_PORT        1
#define CEC_DEFAULT_BASE_DEVICE      0
#define MSGSTART                     0xFF
#define MSGEND                       0xFE
#define MSGESC                       0xFD
#define ESCOFFSET                    3
#define CEC_BUTTON_TIMEOUT           500

#define CEC_DEFAULT_TRANSMIT_TIMEOUT 1000
#define CEC_DEFAULT_TRANSMIT_WAIT    2000
#define CEC_DEFAULT_TRANSMIT_RETRIES 1

#define CEC_MIN_LIB_VERSION          1
#define CEC_LIB_VERSION_MAJOR        1
#define CEC_LIB_VERSION_MINOR        2

typedef enum cec_abort_reason
{
  CEC_ABORT_REASON_UNRECOGNIZED_OPCODE            = 0,
  CEC_ABORT_REASON_NOT_IN_CORRECT_MODE_TO_RESPOND = 1,
  CEC_ABORT_REASON_CANNOT_PROVIDE_SOURCE          = 2,
  CEC_ABORT_REASON_INVALID_OPERAND                = 3,
  CEC_ABORT_REASON_REFUSED                        = 4
} cec_abort_reason;

typedef enum cec_analogue_broadcast_type
{
  CEC_ANALOGUE_BROADCAST_TYPE_CABLE      = 0x00,
  CEC_ANALOGUE_BROADCAST_TYPE_SATELLITE  = 0x01,
  CEC_ANALOGUE_BROADCAST_TYPE_TERRESTIAL = 0x02
} cec_analogue_broadcast_type;

typedef enum cec_audio_rate
{
  CEC_AUDIO_RATE_RATE_CONTROL_OFF    = 0,
  CEC_AUDIO_RATE_STANDARD_RATE_100   = 1,
  CEC_AUDIO_RATE_FAST_RATE_MAX_101   = 2,
  CEC_AUDIO_RATE_SLOW_RATE_MIN_99    = 3,
  CEC_AUDIO_RATE_STANDARD_RATE_100_0 = 4,
  CEC_AUDIO_RATE_FAST_RATE_MAX_100_1 = 5,
  CEC_AUDIO_RATE_SLOW_RATE_MIN_99_9  = 6
} cec_audio_rate;

typedef enum cec_audio_status
{
  CEC_AUDIO_MUTE_STATUS_MASK      = 0x80,
  CEC_AUDIO_VOLUME_STATUS_MASK    = 0x7F,
  CEC_AUDIO_VOLUME_MIN            = 0x00,
  CEC_AUDIO_VOLUME_MAX            = 0x64,
  CEC_AUDIO_VOLUME_STATUS_UNKNOWN = 0x7F
} cec_audio_status;

typedef enum cec_boolean
{
  CEC_FALSE = 0,
  CEC_TRUE  = 1
} cec_boolean;

typedef enum cec_version
{
  CEC_VERSION_UNKNOWN = 0x00,
  CEC_VERSION_1_2     = 0x01,
  CEC_VERSION_1_2A    = 0x02,
  CEC_VERSION_1_3     = 0x03,
  CEC_VERSION_1_3A    = 0x04,
  CEC_VERSION_1_4     = 0x05
} cec_version;

typedef enum cec_channel_identifier
{
  CEC_CHANNEL_NUMBER_FORMAT_MASK = 0xFC000000,
  CEC_1_PART_CHANNEL_NUMBER      = 0x04000000,
  CEC_2_PART_CHANNEL_NUMBER      = 0x08000000,
  CEC_MAJOR_CHANNEL_NUMBER_MASK  = 0x3FF0000,
  CEC_MINOR_CHANNEL_NUMBER_MASK  = 0xFFFF
} cec_channel_identifier;

typedef enum cec_deck_control_mode
{
  CEC_DECK_CONTROL_MODE_SKIP_FORWARD_WIND   = 1,
  CEC_DECK_CONTROL_MODE_SKIP_REVERSE_REWIND = 2,
  CEC_DECK_CONTROL_MODE_STOP                = 3,
  CEC_DECK_CONTROL_MODE_EJECT               = 4
} cec_deck_control_mode;

typedef enum cec_deck_info
{
  CEC_DECK_INFO_PLAY                 = 0x11,
  CEC_DECK_INFO_RECORD               = 0x12,
  CEC_DECK_INFO_PLAY_REVERSE         = 0x13,
  CEC_DECK_INFO_STILL                = 0x14,
  CEC_DECK_INFO_SLOW                 = 0x15,
  CEC_DECK_INFO_SLOW_REVERSE         = 0x16,
  CEC_DECK_INFO_FAST_FORWARD         = 0x17,
  CEC_DECK_INFO_FAST_REVERSE         = 0x18,
  CEC_DECK_INFO_NO_MEDIA             = 0x19,
  CEC_DECK_INFO_STOP                 = 0x1A,
  CEC_DECK_INFO_SKIP_FORWARD_WIND    = 0x1B,
  CEC_DECK_INFO_SKIP_REVERSE_REWIND  = 0x1C,
  CEC_DECK_INFO_INDEX_SEARCH_FORWARD = 0x1D,
  CEC_DECK_INFO_INDEX_SEARCH_REVERSE = 0x1E,
  CEC_DECK_INFO_OTHER_STATUS         = 0x1F,
  CEC_DECK_INFO_OTHER_STATUS_LG      = 0x20
} cec_deck_info;

typedef enum cec_device_type
{
  CEC_DEVICE_TYPE_TV               = 0,
  CEC_DEVICE_TYPE_RECORDING_DEVICE = 1,
  CEC_DEVICE_TYPE_RESERVED         = 2,
  CEC_DEVICE_TYPE_TUNER            = 3,
  CEC_DEVICE_TYPE_PLAYBACK_DEVICE  = 4,
  CEC_DEVICE_TYPE_AUDIO_SYSTEM     = 5
} cec_device_type;

typedef enum cec_display_control
{
  CEC_DISPLAY_CONTROL_DISPLAY_FOR_DEFAULT_TIME = 0x00,
  CEC_DISPLAY_CONTROL_DISPLAY_UNTIL_CLEARED    = 0x40,
  CEC_DISPLAY_CONTROL_CLEAR_PREVIOUS_MESSAGE   = 0x80,
  CEC_DISPLAY_CONTROL_RESERVED_FOR_FUTURE_USE  = 0xC0
} cec_display_control;

typedef enum cec_external_source_specifier
{
  CEC_EXTERNAL_SOURCE_SPECIFIER_EXTERNAL_PLUG             = 4,
  CEC_EXTERNAL_SOURCE_SPECIFIER_EXTERNAL_PHYSICAL_ADDRESS = 5
} cec_external_source_specifier;

typedef enum cec_menu_request_type
{
  CEC_MENU_REQUEST_TYPE_ACTIVATE   = 0,
  CEC_MENU_REQUEST_TYPE_DEACTIVATE = 1,
  CEC_MENU_REQUEST_TYPE_QUERY      = 2
} cec_menu_request_type;

typedef enum cec_menu_state
{
  CEC_MENU_STATE_ACTIVATED   = 0,
  CEC_MENU_STATE_DEACTIVATED = 1
} cec_menu_state;

typedef enum cec_play_mode
{
  CEC_PLAY_MODE_PLAY_FORWARD              = 0x24,
  CEC_PLAY_MODE_PLAY_REVERSE              = 0x20,
  CEC_PLAY_MODE_PLAY_STILL                = 0x25,
  CEC_PLAY_MODE_FAST_FORWARD_MIN_SPEED    = 0x05,
  CEC_PLAY_MODE_FAST_FORWARD_MEDIUM_SPEED = 0x06,
  CEC_PLAY_MODE_FAST_FORWARD_MAX_SPEED    = 0x07,
  CEC_PLAY_MODE_FAST_REVERSE_MIN_SPEED    = 0x09,
  CEC_PLAY_MODE_FAST_REVERSE_MEDIUM_SPEED = 0x0A,
  CEC_PLAY_MODE_FAST_REVERSE_MAX_SPEED    = 0x0B,
  CEC_PLAY_MODE_SLOW_FORWARD_MIN_SPEED    = 0x15,
  CEC_PLAY_MODE_SLOW_FORWARD_MEDIUM_SPEED = 0x16,
  CEC_PLAY_MODE_SLOW_FORWARD_MAX_SPEED    = 0x17,
  CEC_PLAY_MODE_SLOW_REVERSE_MIN_SPEED    = 0x19,
  CEC_PLAY_MODE_SLOW_REVERSE_MEDIUM_SPEED = 0x1A,
  CEC_PLAY_MODE_SLOW_REVERSE_MAX_SPEED    = 0x1B
} cec_play_mode;

typedef enum cec_power_status
{
  CEC_POWER_STATUS_ON                          = 0x00,
  CEC_POWER_STATUS_STANDBY                     = 0x01,
  CEC_POWER_STATUS_IN_TRANSITION_STANDBY_TO_ON = 0x02,
  CEC_POWER_STATUS_IN_TRANSITION_ON_TO_STANDBY = 0x03,
  CEC_POWER_STATUS_UNKNOWN                     = 0x99
} cec_power_status;

typedef enum cec_record_source_type
{
  CEC_RECORD_SOURCE_TYPE_OWN_SOURCE                = 1,
  CEC_RECORD_SOURCE_TYPE_DIGITAL_SERVICE           = 2,
  CEC_RECORD_SOURCE_TYPE_ANALOGUE_SERVICE          = 3,
  CEC_RECORD_SOURCE_TYPE_EXTERNAL_PLUS             = 4,
  CEC_RECORD_SOURCE_TYPE_EXTERNAL_PHYSICAL_ADDRESS = 5
} cec_record_source_type;

typedef enum cec_record_status_info
{
  CEC_RECORD_STATUS_INFO_RECORDING_CURRENTLY_SELECTED_SOURCE            = 0x01,
  CEC_RECORD_STATUS_INFO_RECORDING_DIGITAL_SERVICE                      = 0x02,
  CEC_RECORD_STATUS_INFO_RECORDING_ANALOGUE_SERVICE                     = 0x03,
  CEC_RECORD_STATUS_INFO_RECORDING_EXTERNAL_INPUT                       = 0x04,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_UNABLE_TO_RECORD_DIGITAL_SERVICE  = 0x05,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_UNABLE_TO_RECORD_ANALOGUE_SERVICE = 0x06,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_UNABLE_TO_SELECT_REQUIRED_SERVICE = 0x07,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_INVALID_EXTERNAL_PLUG_NUMBER      = 0x09,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_INVALID_EXTERNAL_ADDRESS          = 0x0A,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_CA_SYSTEM_NOT_SUPPORTED           = 0x0B,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_NO_OR_INSUFFICIENT_ENTITLEMENTS   = 0x0C,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_NOT_ALLOWED_TO_COPY_SOURCE        = 0x0D,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_NO_FURTHER_COPIES_ALLOWED         = 0x0E,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_NO_MEDIA                          = 0x10,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_PLAYING                           = 0x11,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_ALREADY_RECORDING                 = 0x12,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_MEDIA_PROTECTED                   = 0x13,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_NO_SOURCE_SIGNAL                  = 0x14,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_MEDIA_PROBLEM                     = 0x15,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_NOT_ENOUGH_SPACE_AVAILABLE        = 0x16,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_PARENTAL_LOCK_ON                  = 0x17,
  CEC_RECORD_STATUS_INFO_RECORDING_TERMINATED_NORMALLY                  = 0x1A,
  CEC_RECORD_STATUS_INFO_RECORDING_HAS_ALREADY_TERMINATED               = 0x1B,
  CEC_RECORD_STATUS_INFO_NO_RECORDING_OTHER_REASON                      = 0x1F
} cec_record_status_info;

typedef enum cec_recording_sequence
{
  CEC_RECORDING_SEQUENCE_SUNDAY    = 0x01,
  CEC_RECORDING_SEQUENCE_MONDAY    = 0x02,
  CEC_RECORDING_SEQUENCE_TUESDAY   = 0x04,
  CEC_RECORDING_SEQUENCE_WEDNESDAY = 0x08,
  CEC_RECORDING_SEQUENCE_THURSDAY  = 0x10,
  CEC_RECORDING_SEQUENCE_FRIDAY    = 0x20,
  CEC_RECORDING_SEQUENCE_SATURDAY  = 0x40,
  CEC_RECORDING_SEQUENCE_ONCE_ONLY = 0x00
} cec_recording_sequence;

typedef enum cec_status_request
{
  CEC_STATUS_REQUEST_ON   = 1,
  CEC_STATUS_REQUEST_OFF  = 2,
  CEC_STATUS_REQUEST_ONCE = 3
} cec_status_request;

typedef enum cec_system_audio_status
{
  CEC_SYSTEM_AUDIO_STATUS_OFF = 0,
  CEC_SYSTEM_AUDIO_STATUS_ON  = 1
} cec_system_audio_status;

typedef enum cec_timer_cleared_status_data
{
  CEC_TIMER_CLEARED_STATUS_DATA_TIMER_NOT_CLEARED_RECORDING         = 0x00,
  CEC_TIMER_CLEARED_STATUS_DATA_TIMER_NOT_CLEARED_NO_MATCHING       = 0x01,
  CEC_TIMER_CLEARED_STATUS_DATA_TIMER_NOT_CLEARED_NO_INF0_AVAILABLE = 0x02,
  CEC_TIMER_CLEARED_STATUS_DATA_TIMER_CLEARED                       = 0x80
} cec_timer_cleared_status_data;

typedef enum cec_timer_overlap_warning
{
  CEC_TIMER_OVERLAP_WARNING_NO_OVERLAP           = 0,
  CEC_TIMER_OVERLAP_WARNING_TIMER_BLOCKS_OVERLAP = 1
} cec_timer_overlap_warning;

typedef enum cec_media_info
{
  CEC_MEDIA_INFO_MEDIA_PRESENT_AND_NOT_PROTECTED = 0x00,
  CEC_MEDIA_INFO_MEDIA_PRESENT_BUT_PROTECTED     = 0x01,
  CEC_MEDIA_INFO_MEDIA_NOT_PRESENT               = 0x02,
  CEC_MEDIA_INFO_FUTURE_USE                      = 0x03
} cec_media_info;

typedef enum cec_programmed_indicator
{
  CEC_PROGRAMMED_INDICATOR_NOT_PROGRAMMED = 0,
  CEC_PROGRAMMED_INDICATOR_PROGRAMMED     = 1
} cec_programmed_indicator;

typedef enum cec_programmed_info
{
  CEC_PROGRAMMED_INFO_FUTURE_USE                               = 0x0,
  CEC_PROGRAMMED_INFO_ENOUGH_SPACE_AVAILABLE_FOR_RECORDING     = 0x08,
  CEC_PROGRAMMED_INFO_NOT_ENOUGH_SPACE_AVAILABLE_FOR_RECORDING = 0x09,
  CEC_PROGRAMMED_INFO_MAY_NOT_BE_ENOUGH_SPACE_AVAILABLE        = 0x0B,
  CEC_PROGRAMMED_INFO_NO_MEDIA_INFO_AVAILABLE                  = 0x0A
} cec_programmed_info;

typedef enum cec_not_programmed_error_info
{
  CEC_NOT_PROGRAMMED_ERROR_INFO_FUTURE_USE                         = 0x0,
  CEC_NOT_PROGRAMMED_ERROR_INFO_NO_FREE_TIMER_AVAILABLE            = 0x01,
  CEC_NOT_PROGRAMMED_ERROR_INFO_DATE_OUT_OF_RANGE                  = 0x02,
  CEC_NOT_PROGRAMMED_ERROR_INFO_RECORDING_SEQUENCE_ERROR           = 0x03,
  CEC_NOT_PROGRAMMED_ERROR_INFO_INVALID_EXTERNAL_PLUG_NUMBER       = 0x04,
  CEC_NOT_PROGRAMMED_ERROR_INFO_INVALID_EXTERNAL_PHYSICAL_ADDRESS  = 0x05,
  CEC_NOT_PROGRAMMED_ERROR_INFO_CA_SYSTEM_NOT_SUPPORTED            = 0x06,
  CEC_NOT_PROGRAMMED_ERROR_INFO_NO_OR_INSUFFICIENT_CA_ENTITLEMENTS = 0x07,
  CEC_NOT_PROGRAMMED_ERROR_INFO_DOES_NOT_SUPPORT_RESOLUTION        = 0x08,
  CEC_NOT_PROGRAMMED_ERROR_INFO_PARENTAL_LOCK_ON                   = 0x09,
  CEC_NOT_PROGRAMMED_ERROR_INFO_CLOCK_FAILURE                      = 0x0A,
  CEC_NOT_PROGRAMMED_ERROR_INFO_RESERVED_FOR_FUTURE_USE_START      = 0x0B,
  CEC_NOT_PROGRAMMED_ERROR_INFO_RESERVED_FOR_FUTURE_USE_END        = 0x0D,
  CEC_NOT_PROGRAMMED_ERROR_INFO_DUPLICATE_ALREADY_PROGRAMMED       = 0x0E
} cec_not_programmed_error_info;

typedef enum cec_recording_flag
{
  CEC_RECORDING_FLAG_NOT_BEING_USED_FOR_RECORDING = 0,
  CEC_RECORDING_FLAG_BEING_USED_FOR_RECORDING     = 1
} cec_recording_flag;

typedef enum cec_tuner_display_info
{
  CEC_TUNER_DISPLAY_INFO_DISPLAYING_DIGITAL_TUNER  = 0,
  CEC_TUNER_DISPLAY_INFO_NOT_DISPLAYING_TUNER      = 1,
  CEC_TUNER_DISPLAY_INFO_DISPLAYING_ANALOGUE_TUNER = 2
} cec_tuner_display_info;

typedef enum cec_broadcast_system
{
  CEC_BROADCAST_SYSTEM_PAL_B_G      = 0,
  CEC_BROADCAST_SYSTEM_SECAM_L1     = 1,
  CEC_BROADCAST_SYSTEM_PAL_M        = 2,
  CEC_BROADCAST_SYSTEM_NTSC_M       = 3,
  CEC_BROADCAST_SYSTEM_PAL_I        = 4,
  CEC_BROADCAST_SYSTEM_SECAM_DK     = 5,
  CEC_BROADCAST_SYSTEM_SECAM_B_G    = 6,
  CEC_BROADCAST_SYSTEM_SECAM_L2     = 7,
  CEC_BROADCAST_SYSTEM_PAL_DK       = 8,
  CEC_BROADCAST_SYSTEM_OTHER_SYSTEM = 30
} cec_broadcast_system;

typedef enum cec_user_control_code
{
  CEC_USER_CONTROL_CODE_SELECT                      = 0x00,
  CEC_USER_CONTROL_CODE_UP                          = 0x01,
  CEC_USER_CONTROL_CODE_DOWN                        = 0x02,
  CEC_USER_CONTROL_CODE_LEFT                        = 0x03,
  CEC_USER_CONTROL_CODE_RIGHT                       = 0x04,
  CEC_USER_CONTROL_CODE_RIGHT_UP                    = 0x05,
  CEC_USER_CONTROL_CODE_RIGHT_DOWN                  = 0x06,
  CEC_USER_CONTROL_CODE_LEFT_UP                     = 0x07,
  CEC_USER_CONTROL_CODE_LEFT_DOWN                   = 0x08,
  CEC_USER_CONTROL_CODE_ROOT_MENU                   = 0x09,
  CEC_USER_CONTROL_CODE_SETUP_MENU                  = 0x0A,
  CEC_USER_CONTROL_CODE_CONTENTS_MENU               = 0x0B,
  CEC_USER_CONTROL_CODE_FAVORITE_MENU               = 0x0C,
  CEC_USER_CONTROL_CODE_EXIT                        = 0x0D,
  CEC_USER_CONTROL_CODE_NUMBER0                     = 0x20,
  CEC_USER_CONTROL_CODE_NUMBER1                     = 0x21,
  CEC_USER_CONTROL_CODE_NUMBER2                     = 0x22,
  CEC_USER_CONTROL_CODE_NUMBER3                     = 0x23,
  CEC_USER_CONTROL_CODE_NUMBER4                     = 0x24,
  CEC_USER_CONTROL_CODE_NUMBER5                     = 0x25,
  CEC_USER_CONTROL_CODE_NUMBER6                     = 0x26,
  CEC_USER_CONTROL_CODE_NUMBER7                     = 0x27,
  CEC_USER_CONTROL_CODE_NUMBER8                     = 0x28,
  CEC_USER_CONTROL_CODE_NUMBER9                     = 0x29,
  CEC_USER_CONTROL_CODE_DOT                         = 0x2A,
  CEC_USER_CONTROL_CODE_ENTER                       = 0x2B,
  CEC_USER_CONTROL_CODE_CLEAR                       = 0x2C,
  CEC_USER_CONTROL_CODE_NEXT_FAVORITE               = 0x2F,
  CEC_USER_CONTROL_CODE_CHANNEL_UP                  = 0x30,
  CEC_USER_CONTROL_CODE_CHANNEL_DOWN                = 0x31,
  CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL            = 0x32,
  CEC_USER_CONTROL_CODE_SOUND_SELECT                = 0x33,
  CEC_USER_CONTROL_CODE_INPUT_SELECT                = 0x34,
  CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION         = 0x35,
  CEC_USER_CONTROL_CODE_HELP                        = 0x36,
  CEC_USER_CONTROL_CODE_PAGE_UP                     = 0x37,
  CEC_USER_CONTROL_CODE_PAGE_DOWN                   = 0x38,
  CEC_USER_CONTROL_CODE_POWER                       = 0x40,
  CEC_USER_CONTROL_CODE_VOLUME_UP                   = 0x41,
  CEC_USER_CONTROL_CODE_VOLUME_DOWN                 = 0x42,
  CEC_USER_CONTROL_CODE_MUTE                        = 0x43,
  CEC_USER_CONTROL_CODE_PLAY                        = 0x44,
  CEC_USER_CONTROL_CODE_STOP                        = 0x45,
  CEC_USER_CONTROL_CODE_PAUSE                       = 0x46,
  CEC_USER_CONTROL_CODE_RECORD                      = 0x47,
  CEC_USER_CONTROL_CODE_REWIND                      = 0x48,
  CEC_USER_CONTROL_CODE_FAST_FORWARD                = 0x49,
  CEC_USER_CONTROL_CODE_EJECT                       = 0x4A,
  CEC_USER_CONTROL_CODE_FORWARD                     = 0x4B,
  CEC_USER_CONTROL_CODE_BACKWARD                    = 0x4C,
  CEC_USER_CONTROL_CODE_STOP_RECORD                 = 0x4D,
  CEC_USER_CONTROL_CODE_PAUSE_RECORD                = 0x4E,
  CEC_USER_CONTROL_CODE_ANGLE                       = 0x50,
  CEC_USER_CONTROL_CODE_SUB_PICTURE                 = 0x51,
  CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND             = 0x52,
  CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    = 0x53,
  CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING           = 0x54,
  CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION       = 0x55,
  CEC_USER_CONTROL_CODE_PLAY_FUNCTION               = 0x60,
  CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         = 0x61,
  CEC_USER_CONTROL_CODE_RECORD_FUNCTION             = 0x62,
  CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       = 0x63,
  CEC_USER_CONTROL_CODE_STOP_FUNCTION               = 0x64,
  CEC_USER_CONTROL_CODE_MUTE_FUNCTION               = 0x65,
  CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     = 0x66,
  CEC_USER_CONTROL_CODE_TUNE_FUNCTION               = 0x67,
  CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       = 0x68,
  CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION    = 0x69,
  CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION = 0x6A,
  CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       = 0x6B,
  CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION          = 0x6C,
  CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION           = 0x6D,
  CEC_USER_CONTROL_CODE_F1_BLUE                     = 0x71,
  CEC_USER_CONTROL_CODE_F2_RED                      = 0X72,
  CEC_USER_CONTROL_CODE_F3_GREEN                    = 0x73,
  CEC_USER_CONTROL_CODE_F4_YELLOW                   = 0x74,
  CEC_USER_CONTROL_CODE_F5                          = 0x75,
  CEC_USER_CONTROL_CODE_DATA                        = 0x76,
  CEC_USER_CONTROL_CODE_MAX                         = 0x76,
  CEC_USER_CONTROL_CODE_UNKNOWN
} cec_user_control_code;

typedef enum cec_an_user_control_code
{
  CEC_AN_USER_CONTROL_CODE_RETURN = 0x91
} cec_an_user_control_code;

typedef enum cec_logical_address
{
  CECDEVICE_UNKNOWN          = -1, //not a valid logical address
  CECDEVICE_TV               = 0,
  CECDEVICE_RECORDINGDEVICE1 = 1,
  CECDEVICE_RECORDINGDEVICE2 = 2,
  CECDEVICE_TUNER1           = 3,
  CECDEVICE_PLAYBACKDEVICE1  = 4,
  CECDEVICE_AUDIOSYSTEM      = 5,
  CECDEVICE_TUNER2           = 6,
  CECDEVICE_TUNER3           = 7,
  CECDEVICE_PLAYBACKDEVICE2  = 8,
  CECDEVICE_RECORDINGDEVICE3 = 9,
  CECDEVICE_TUNER4           = 10,
  CECDEVICE_PLAYBACKDEVICE3  = 11,
  CECDEVICE_RESERVED1        = 12,
  CECDEVICE_RESERVED2        = 13,
  CECDEVICE_FREEUSE          = 14,
  CECDEVICE_UNREGISTERED     = 15,
  CECDEVICE_BROADCAST        = 15
} cec_logical_address;

typedef enum cec_opcode
{
  CEC_OPCODE_ACTIVE_SOURCE                 = 0x82,
  CEC_OPCODE_IMAGE_VIEW_ON                 = 0x04,
  CEC_OPCODE_TEXT_VIEW_ON                  = 0x0D,
  CEC_OPCODE_INACTIVE_SOURCE               = 0x9D,
  CEC_OPCODE_REQUEST_ACTIVE_SOURCE         = 0x85,
  CEC_OPCODE_ROUTING_CHANGE                = 0x80,
  CEC_OPCODE_ROUTING_INFORMATION           = 0x81,
  CEC_OPCODE_SET_STREAM_PATH               = 0x86,
  CEC_OPCODE_STANDBY                       = 0x36,
  CEC_OPCODE_RECORD_OFF                    = 0x0B,
  CEC_OPCODE_RECORD_ON                     = 0x09,
  CEC_OPCODE_RECORD_STATUS                 = 0x0A,
  CEC_OPCODE_RECORD_TV_SCREEN              = 0x0F,
  CEC_OPCODE_CLEAR_ANALOGUE_TIMER          = 0x33,
  CEC_OPCODE_CLEAR_DIGITAL_TIMER           = 0x99,
  CEC_OPCODE_CLEAR_EXTERNAL_TIMER          = 0xA1,
  CEC_OPCODE_SET_ANALOGUE_TIMER            = 0x34,
  CEC_OPCODE_SET_DIGITAL_TIMER             = 0x97,
  CEC_OPCODE_SET_EXTERNAL_TIMER            = 0xA2,
  CEC_OPCODE_SET_TIMER_PROGRAM_TITLE       = 0x67,
  CEC_OPCODE_TIMER_CLEARED_STATUS          = 0x43,
  CEC_OPCODE_TIMER_STATUS                  = 0x35,
  CEC_OPCODE_CEC_VERSION                   = 0x9E,
  CEC_OPCODE_GET_CEC_VERSION               = 0x9F,
  CEC_OPCODE_GIVE_PHYSICAL_ADDRESS         = 0x83,
  CEC_OPCODE_GET_MENU_LANGUAGE             = 0x91,
  CEC_OPCODE_REPORT_PHYSICAL_ADDRESS       = 0x84,
  CEC_OPCODE_SET_MENU_LANGUAGE             = 0x32,
  CEC_OPCODE_DECK_CONTROL                  = 0x42,
  CEC_OPCODE_DECK_STATUS                   = 0x1B,
  CEC_OPCODE_GIVE_DECK_STATUS              = 0x1A,
  CEC_OPCODE_PLAY                          = 0x41,
  CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS      = 0x08,
  CEC_OPCODE_SELECT_ANALOGUE_SERVICE       = 0x92,
  CEC_OPCODE_SELECT_DIGITAL_SERVICE        = 0x93,
  CEC_OPCODE_TUNER_DEVICE_STATUS           = 0x07,
  CEC_OPCODE_TUNER_STEP_DECREMENT          = 0x06,
  CEC_OPCODE_TUNER_STEP_INCREMENT          = 0x05,
  CEC_OPCODE_DEVICE_VENDOR_ID              = 0x87,
  CEC_OPCODE_GIVE_DEVICE_VENDOR_ID         = 0x8C,
  CEC_OPCODE_VENDOR_COMMAND                = 0x89,
  CEC_OPCODE_VENDOR_COMMAND_WITH_ID        = 0xA0,
  CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN     = 0x8A,
  CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP       = 0x8B,
  CEC_OPCODE_SET_OSD_STRING                = 0x64,
  CEC_OPCODE_GIVE_OSD_NAME                 = 0x46,
  CEC_OPCODE_SET_OSD_NAME                  = 0x47,
  CEC_OPCODE_MENU_REQUEST                  = 0x8D,
  CEC_OPCODE_MENU_STATUS                   = 0x8E,
  CEC_OPCODE_USER_CONTROL_PRESSED          = 0x44,
  CEC_OPCODE_USER_CONTROL_RELEASE          = 0x45,
  CEC_OPCODE_GIVE_DEVICE_POWER_STATUS      = 0x8F,
  CEC_OPCODE_REPORT_POWER_STATUS           = 0x90,
  CEC_OPCODE_FEATURE_ABORT                 = 0x00,
  CEC_OPCODE_ABORT                         = 0xFF,
  CEC_OPCODE_GIVE_AUDIO_STATUS             = 0x71,
  CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS = 0x7D,
  CEC_OPCODE_REPORT_AUDIO_STATUS           = 0x7A,
  CEC_OPCODE_SET_SYSTEM_AUDIO_MODE         = 0x72,
  CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST     = 0x70,
  CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS      = 0x7E,
  CEC_OPCODE_SET_AUDIO_RATE                = 0x9A,
  /* when this opcode is set, no opcode will be sent to the device. this is one of the reserved numbers */
  CEC_OPCODE_NONE                          = 0xFD
} cec_opcode;

typedef enum cec_log_level
{
  CEC_LOG_ERROR   = 1,
  CEC_LOG_WARNING = 2,
  CEC_LOG_NOTICE  = 4,
  CEC_LOG_TRAFFIC = 8,
  CEC_LOG_DEBUG   = 16,
  CEC_LOG_ALL     = 31
} cec_log_level;

typedef enum cec_adapter_messagecode
{
  MSGCODE_NOTHING = 0,
  MSGCODE_PING,
  MSGCODE_TIMEOUT_ERROR,
  MSGCODE_HIGH_ERROR,
  MSGCODE_LOW_ERROR,
  MSGCODE_FRAME_START,
  MSGCODE_FRAME_DATA,
  MSGCODE_RECEIVE_FAILED,
  MSGCODE_COMMAND_ACCEPTED,
  MSGCODE_COMMAND_REJECTED,
  MSGCODE_SET_ACK_MASK,
  MSGCODE_TRANSMIT,
  MSGCODE_TRANSMIT_EOM,
  MSGCODE_TRANSMIT_IDLETIME,
  MSGCODE_TRANSMIT_ACK_POLARITY,
  MSGCODE_TRANSMIT_LINE_TIMEOUT,
  MSGCODE_TRANSMIT_SUCCEEDED,
  MSGCODE_TRANSMIT_FAILED_LINE,
  MSGCODE_TRANSMIT_FAILED_ACK,
  MSGCODE_TRANSMIT_FAILED_TIMEOUT_DATA,
  MSGCODE_TRANSMIT_FAILED_TIMEOUT_LINE,
  MSGCODE_FIRMWARE_VERSION,
  MSGCODE_START_BOOTLOADER,
  MSGCODE_FRAME_EOM = 0x80,
  MSGCODE_FRAME_ACK = 0x40,
} cec_adapter_messagecode;

typedef enum cec_bus_device_status
{
  CEC_DEVICE_STATUS_UNKNOWN,
  CEC_DEVICE_STATUS_PRESENT,
  CEC_DEVICE_STATUS_NOT_PRESENT,
  CEC_DEVICE_STATUS_HANDLED_BY_LIBCEC
} cec_bus_device_status;

typedef enum cec_vendor_id
{
  CEC_VENDOR_SAMSUNG   = 0x00F0,
  CEC_VENDOR_LG        = 0xE091,
  CEC_VENDOR_PANASONIC = 0x8045,
  CEC_VENDOR_PIONEER   = 0xE036,
  CEC_VENDOR_ONKYO     = 0x09B0,
  CEC_VENDOR_YAMAHA    = 0xA0DE,
  CEC_VENDOR_PHILIPS   = 0x903E,
  CEC_VENDOR_UNKNOWN   = 0
} cec_vendor_id;

typedef struct cec_menu_language
{
  char                language[4];
  cec_logical_address device;
} cec_menu_language;

typedef struct cec_osd_name
{
  char                name[14];
  cec_logical_address device;
} cec_osd_name;

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

typedef struct cec_datapacket
{
  uint8_t data[100];
  uint8_t size;

#ifdef __cplusplus
  cec_datapacket &operator =(const struct cec_datapacket &packet)
  {
    Clear();
    for (uint8_t iPtr = 0; iPtr < packet.size; iPtr++)
      PushBack(packet[iPtr]);

    return *this;
  }

  bool    IsEmpty(void) const             { return size == 0; }
  bool    IsFull(void) const              { return size == 100; }
  uint8_t operator[](uint8_t pos) const { return pos < size ? data[pos] : 0; }
  uint8_t At(uint8_t pos) const         { return pos < size ? data[pos] : 0; }

  void Shift(uint8_t iShiftBy)
  {
    if (iShiftBy >= size)
    {
      Clear();
    }
    else
    {
      for (uint8_t iPtr = 0; iPtr < size; iPtr++)
        data[iPtr] = (iPtr + iShiftBy < size) ? data[iPtr + iShiftBy] : 0;
      size = (uint8_t) (size - iShiftBy);
    }
  }

  void PushBack(uint8_t add)
  {
    if (size < 100)
      data[size++] = add;
  }

  void Clear(void)
  {
    memset(data, 0, sizeof(data));
    size = 0;
  }
#endif

} cec_datapacket;

typedef struct cec_command
{
  cec_logical_address initiator;
  cec_logical_address destination;
  int8_t              ack;
  int8_t              eom;
  cec_opcode          opcode;
  cec_datapacket      parameters;
  int8_t              opcode_set;
  int32_t             transmit_timeout;

#ifdef __cplusplus
  cec_command &operator =(const struct cec_command &command)
  {
    initiator        = command.initiator;
    destination      = command.destination;
    ack              = command.ack;
    eom              = command.eom;
    opcode           = command.opcode;
    opcode_set       = command.opcode_set;
    transmit_timeout = command.transmit_timeout;
    parameters       = command.parameters;

    return *this;
  }

  static void Format(cec_command &command, cec_logical_address initiator, cec_logical_address destination, cec_opcode opcode, int32_t timeout = CEC_DEFAULT_TRANSMIT_TIMEOUT)
  {
    command.Clear();
    command.initiator        = initiator;
    command.destination      = destination;
    command.transmit_timeout = timeout;
    if (opcode != CEC_OPCODE_NONE)
    {
      command.opcode     = opcode;
      command.opcode_set = 1;
    }
  }

  void PushBack(uint8_t data)
  {
    if (initiator == CECDEVICE_UNKNOWN && destination == CECDEVICE_UNKNOWN)
    {
      initiator   = (cec_logical_address) (data >> 4);
      destination = (cec_logical_address) (data & 0xF);
    }
    else if (!opcode_set)
    {
      opcode_set = 1;
      opcode = (cec_opcode) data;
    }
    else
      parameters.PushBack(data);
  }

  void Clear(void)
  {
    initiator        = CECDEVICE_UNKNOWN;
    destination      = CECDEVICE_UNKNOWN;
    ack              = 0;
    eom              = 0;
    opcode_set       = 0;
    opcode           = CEC_OPCODE_FEATURE_ABORT;
    transmit_timeout = CEC_DEFAULT_TRANSMIT_TIMEOUT;
    parameters.Clear();
  };
#endif
} cec_command;

typedef struct cec_device_type_list
{
  cec_device_type types[5];

#ifdef __cplusplus
  /*!
   * @deprecated
   */
  void clear(void) { Clear(); }
  /*!
   * @deprecated
   */
  void add(const cec_device_type type) { Add(type); }

  void Clear(void)
  {
    for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
     types[iPtr] = CEC_DEVICE_TYPE_RESERVED;
  }

  void Add(const cec_device_type type)
  {
    for (unsigned int iPtr = 0; iPtr < 5; iPtr++)
    {
      if (types[iPtr] == CEC_DEVICE_TYPE_RESERVED)
      {
        types[iPtr] = type;
        break;
      }
    }
  }

  bool IsSet(cec_device_type type)
  {
    bool bReturn(false);
    for (unsigned int iPtr = 0; !bReturn && iPtr < 5; iPtr++)
    {
      if (types[iPtr] == type)
        bReturn = true;
    }
    return bReturn;
  }

  bool IsEmpty()
  {
    bool bReturn(true);
    for (unsigned int iPtr = 0; bReturn && iPtr < 5; iPtr++)
    {
      if (types[iPtr] != CEC_DEVICE_TYPE_RESERVED)
        bReturn = false;
    }
    return bReturn;
  }

  cec_device_type operator[](uint8_t pos) const { return pos < 5 ? types[pos] : CEC_DEVICE_TYPE_RESERVED; }
#endif
} cec_device_type_list;

typedef struct cec_logical_addresses
{
  cec_logical_address primary;
  int                 addresses[16];

#ifdef __cplusplus
  void Clear(void)
  {
    primary = CECDEVICE_UNKNOWN;
    for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
      addresses[iPtr] = 0;
  }

  bool IsEmpty(void) const
  {
    return primary == CECDEVICE_UNKNOWN;
  }

  uint16_t AckMask(void) const
  {
    uint16_t mask = 0;
    for (unsigned int iPtr = 0; iPtr < 16; iPtr++)
      if (addresses[iPtr] == 1)
        mask |= 0x1 << iPtr;
    return mask;
  }

  void Set(cec_logical_address address)
  {
    if (primary == CECDEVICE_UNKNOWN)
      primary = address;

    addresses[(int) address] = 1;
  }

  void Unset(cec_logical_address address)
  {
    if (primary == address)
      primary = CECDEVICE_UNKNOWN;

    addresses[(int) address] = 0;
  }

  bool IsSet(cec_logical_address address) const { return addresses[(int) address] == 1; }
  bool operator[](uint8_t pos) const { return pos < 16 ? IsSet((cec_logical_address) pos) : false; }
#endif
} cec_logical_addresses;

#ifdef __cplusplus
};
};
#endif

#endif /* CECTYPES_H_ */
