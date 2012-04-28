#pragma once
/*
 * This file is part of the libCEC(R) library.
 *
 * libCEC(R) is Copyright (C) 2011-2012 Pulse-Eight Limited.  All rights reserved.
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

#include "CECBusDevice.h"
#include "CECPlaybackDevice.h"
#include "CECTuner.h"

namespace CEC
{
  class CCECRecordingDevice : public CCECBusDevice
  {
  public:
    CCECRecordingDevice(CCECProcessor *processor, cec_logical_address address, uint16_t iPhysicalAddress = CEC_INVALID_PHYSICAL_ADDRESS);
    virtual ~CCECRecordingDevice(void) {};

    /* playback device methods */
    cec_deck_info GetDeckStatus(void);
    cec_deck_control_mode GetDeckControlMode(void);

    void SetDeckStatus(cec_deck_info deckStatus);
    void SetDeckControlMode(cec_deck_control_mode mode);

    bool TransmitDeckStatus(cec_logical_address dest);

    /* tuner methods */
    //TODO

  protected:
    CCECPlaybackDevice m_playbackDevice;
    CCECTuner          m_tuner;
  };
}
