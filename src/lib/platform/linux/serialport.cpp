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

#include <stdio.h>
#include <fcntl.h>
#include "../serialport.h"
#include "../baudrate.h"
#include "../timeutils.h"

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
using namespace CEC;

CSerialPort::CSerialPort()
{
  m_fd = -1;
}

CSerialPort::~CSerialPort()
{
  Close();
}

int8_t CSerialPort::Write(CCECAdapterMessage *data)
{
  fd_set port;

  CLockObject lock(&m_mutex);
  if (m_fd == -1)
  {
    m_error = "port closed";
    return -1;
  }

  int32_t byteswritten = 0;

  while (byteswritten < (int32_t) data->size())
  {
    FD_ZERO(&port);
    FD_SET(m_fd, &port);
    int returnv = select(m_fd + 1, NULL, &port, NULL, NULL);
    if (returnv == -1)
    {
      m_error = strerror(errno);
      return -1;
    }

    returnv = write(m_fd, data->packet.data + byteswritten, data->size() - byteswritten);
    if (returnv == -1)
    {
      m_error = strerror(errno);
      return -1;
    }
    byteswritten += returnv;
  }

  //print what's written to stdout for debugging
//  if (m_tostdout)
//  {
//    printf("%s write:", m_name.c_str());
//    for (int i = 0; i < byteswritten; i++)
//      printf(" %02x", (unsigned int)data[i]);
//
//    printf("\n");
//  }

  return byteswritten;
}

int32_t CSerialPort::Read(uint8_t* data, uint32_t len, uint64_t iTimeoutMs /*= 0*/)
{
  fd_set port;
  struct timeval timeout, *tv;
  int64_t now(0), target(0);
  int32_t bytesread = 0;

  CLockObject lock(&m_mutex);
  if (m_fd == -1)
  {
    m_error = "port closed";
    return -1;
  }

  if (iTimeoutMs > 0)
  {
    now    = GetTimeMs();
    target = now + (int64_t) iTimeoutMs;
  }

  while (bytesread < (int32_t) len && (iTimeoutMs == 0 || target > now))
  {
    if (iTimeoutMs == 0)
    {
      tv = NULL;
    }
    else
    {
      timeout.tv_sec  = ((long int)target - (long int)now) / (long int)1000.;
      timeout.tv_usec = ((long int)target - (long int)now) % (long int)1000.;
      tv = &timeout;
    }

    FD_ZERO(&port);
    FD_SET(m_fd, &port);
    int32_t returnv = select(m_fd + 1, &port, NULL, NULL, tv);

    if (returnv == -1)
    {
      m_error = strerror(errno);
      return -1;
    }
    else if (returnv == 0)
    {
      break; //nothing to read
    }

    returnv = read(m_fd, data + bytesread, len - bytesread);
    if (returnv == -1)
    {
      m_error = strerror(errno);
      return -1;
    }

    bytesread += returnv;

    if (iTimeoutMs > 0)
      now = GetTimeMs();
  }

  //print what's read to stdout for debugging
//  if (m_tostdout && bytesread > 0)
//  {
//    printf("%s read:", m_name.c_str());
//    for (int i = 0; i < bytesread; i++)
//      printf(" %02x", (unsigned int)data[i]);
//
//    printf("\n");
//  }

  return bytesread;
}

//setting all this stuff up is a pain in the ass
bool CSerialPort::Open(string name, uint32_t baudrate, uint8_t databits /* = 8 */, uint8_t stopbits /* = 1 */, uint8_t parity /* = PAR_NONE */)
{
  m_name = name;
  m_error = strerror(errno);
  CLockObject lock(&m_mutex);

  if (databits < 5 || databits > 8)
  {
    m_error = "Databits has to be between 5 and 8";
    return false;
  }

  if (stopbits != 1 && stopbits != 2)
  {
    m_error = "Stopbits has to be 1 or 2";
    return false;
  }

  if (parity != PAR_NONE && parity != PAR_EVEN && parity != PAR_ODD)
  {
    m_error = "Parity has to be none, even or odd";
    return false;
  }

  m_fd = open(name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if (m_fd == -1)
  {
    m_error = strerror(errno);
    return false;
  }

  fcntl(m_fd, F_SETFL, 0);

  if (!SetBaudRate(baudrate))
  {
    return false;
  }

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

  if (tcsetattr(m_fd, TCSANOW, &m_options) != 0)
  {
    m_error = strerror(errno);
    return false;
  }
  
  //non-blocking port
  fcntl(m_fd, F_SETFL, FNDELAY);

  return true;
}

void CSerialPort::Close()
{
  CLockObject lock(&m_mutex);
  if (m_fd != -1)
  {
    close(m_fd);
    m_fd = -1;
    m_name = "";
    m_error = "";
  }
}

bool CSerialPort::SetBaudRate(uint32_t baudrate)
{
  int rate = IntToBaudrate(baudrate);
  if (rate == -1)
  {
    char buff[255];
    sprintf(buff, "%i is not a valid baudrate", baudrate);
    m_error = buff;
    return false;
  }
  
  //get the current port attributes
  if (tcgetattr(m_fd, &m_options) != 0)
  {
    m_error = strerror(errno);
    return false;
  }

  if (cfsetispeed(&m_options, rate) != 0)
  {
    m_error = strerror(errno);
    return false;
  }
  
  if (cfsetospeed(&m_options, rate) != 0)
  {
    m_error = strerror(errno);
    return false;
  }

  return true;
}

bool CSerialPort::IsOpen()
{
  CLockObject lock(&m_mutex);
  return m_fd != -1;
}
