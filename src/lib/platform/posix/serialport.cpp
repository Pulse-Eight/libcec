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

#include "../os.h"
#include <stdio.h>
#include <fcntl.h>
#include "../sockets/serialport.h"
#include "../util/baudrate.h"

#if defined(__APPLE__)
#ifndef XCASE
#define XCASE	0
#endif
#ifndef OLCUC
#define OLCUC	0
#endif
#ifndef IUCLC
#define IUCLC	0
#endif
#endif
using namespace std;
using namespace PLATFORM;

CSerialPort::CSerialPort()
{
  m_bToStdOut = false;
}

//setting all this stuff up is a pain in the ass
bool CSerialPort::Open(string name, uint32_t baudrate, uint8_t databits /* = 8 */, uint8_t stopbits /* = 1 */, uint8_t parity /* = PAR_NONE */)
{
  m_strName = name;
  CLockObject lock(m_mutex);

  if (databits < 5 || databits > 8)
  {
    m_strError = "Databits has to be between 5 and 8";
    return false;
  }

  if (stopbits != 1 && stopbits != 2)
  {
    m_strError = "Stopbits has to be 1 or 2";
    return false;
  }

  if (parity != PAR_NONE && parity != PAR_EVEN && parity != PAR_ODD)
  {
    m_strError = "Parity has to be none, even or odd";
    return false;
  }

  m_socket = open(name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if (m_socket == INVALID_SOCKET)
  {
    m_strError = strerror(errno);
    return false;
  }

  SocketSetBlocking(m_socket, false);

  if (!SetBaudRate(baudrate))
    return false;

  m_options.c_cflag |= (CLOCAL | CREAD);
  m_options.c_cflag &= ~HUPCL;

  m_options.c_cflag &= ~CSIZE;
  if (databits == 5) m_options.c_cflag |= CS5;
  if (databits == 6) m_options.c_cflag |= CS6;
  if (databits == 7) m_options.c_cflag |= CS7;
  if (databits == 8) m_options.c_cflag |= CS8;

  m_options.c_cflag &= ~PARENB;
  if (parity == PAR_EVEN || parity == PAR_ODD)
    m_options.c_cflag |= PARENB;
  if (parity == PAR_ODD)
    m_options.c_cflag |= PARODD;

#ifdef CRTSCTS
  m_options.c_cflag &= ~CRTSCTS;
#elif defined(CNEW_RTSCTS)
  m_options.c_cflag &= ~CNEW_RTSCTS;
#endif

  if (stopbits == 1) m_options.c_cflag &= ~CSTOPB;
  else m_options.c_cflag |= CSTOPB;
  
  //I guessed a little here
  m_options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | XCASE | ECHOK | ECHONL | ECHOCTL | ECHOPRT | ECHOKE | TOSTOP);

  if (parity == PAR_NONE)
  {
    m_options.c_iflag &= ~INPCK;
  }
  else
  {
    m_options.c_iflag |= INPCK | ISTRIP;
  }

  m_options.c_iflag &= ~(IXON | IXOFF | IXANY | BRKINT | INLCR | IGNCR | ICRNL | IUCLC | IMAXBEL);
  m_options.c_oflag &= ~(OPOST | ONLCR | OCRNL);

  if (tcsetattr(m_socket, TCSANOW, &m_options) != 0)
  {
    m_strError = strerror(errno);
    return false;
  }
  
  SocketSetBlocking(m_socket, true);

  return true;
}

bool CSerialPort::SetBaudRate(uint32_t baudrate)
{
  int rate = IntToBaudrate(baudrate);
  if (rate == -1)
  {
    char buff[255];
    sprintf(buff, "%i is not a valid baudrate", baudrate);
    m_strError = buff;
    return false;
  }
  
  //get the current port attributes
  if (tcgetattr(m_socket, &m_options) != 0)
  {
    m_strError = strerror(errno);
    return false;
  }

  if (cfsetispeed(&m_options, rate) != 0)
  {
    m_strError = strerror(errno);
    return false;
  }
  
  if (cfsetospeed(&m_options, rate) != 0)
  {
    m_strError = strerror(errno);
    return false;
  }

  return true;
}
