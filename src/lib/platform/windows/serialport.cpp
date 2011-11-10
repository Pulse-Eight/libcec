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

#include "../serialport.h"
#include "../baudrate.h"
#include "../timeutils.h"

using namespace std;
using namespace CEC;

void FormatWindowsError(int iErrorCode, string &strMessage)
{
  if (iErrorCode != ERROR_SUCCESS)
  {
    char strAddMessage[1024];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, iErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), strAddMessage, 1024, NULL);
    strMessage.append(": ");
    strMessage.append(strAddMessage);
  }
}

CSerialPort::CSerialPort(void) :
  m_handle(INVALID_HANDLE_VALUE),
  m_bIsOpen(false),
  m_iBaudrate(0),
  m_iDatabits(0),
  m_iStopbits(0),
  m_iParity(0)
{
}

CSerialPort::~CSerialPort(void)
{
  Close();
}

bool CSerialPort::Open(string name, uint32_t baudrate, uint8_t databits, uint8_t stopbits, uint8_t parity)
{
  CStdString strComPath = "\\\\.\\" + name;
  CLockObject lock(&m_mutex);
  m_handle = CreateFile(strComPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (m_handle == INVALID_HANDLE_VALUE)
  {
    m_error = "Unable to open COM port";
    FormatWindowsError(GetLastError(), m_error);
    return false;
  }

  COMMCONFIG commConfig = {0};
  DWORD dwSize = sizeof(commConfig);
  commConfig.dwSize = dwSize;
  if (GetDefaultCommConfig(strComPath.c_str(), &commConfig,&dwSize))
  {
    if (!SetCommConfig(m_handle, &commConfig,dwSize))
    {
      m_error = "unable to set default config";
      FormatWindowsError(GetLastError(), m_error);
    }
  }
  else
  {
    m_error = "unable to get default config";
    FormatWindowsError(GetLastError(), m_error);
  }

  if (!SetupComm(m_handle, 64, 64))
  {
    m_error = "unable to set up the com port";
    FormatWindowsError(GetLastError(), m_error);
  }

  m_iDatabits = databits;
  m_iStopbits = stopbits;
  m_iParity   = parity;
  if (!SetBaudRate(baudrate))
  {
    m_error = "unable to set baud rate";
    FormatWindowsError(GetLastError(), m_error);
    Close();
    return false;
  }

  if (!SetTimeouts(false))
  {
    m_error = "unable to set timeouts";
    FormatWindowsError(GetLastError(), m_error);
    Close();
    return false;
  }

  m_bIsOpen = true;
  return m_bIsOpen;
}

bool CSerialPort::SetTimeouts(bool bBlocking)
{
  if (m_handle == INVALID_HANDLE_VALUE)
	  return false;

  COMMTIMEOUTS cto;
  if (!GetCommTimeouts(m_handle, &cto))
  {
    m_error = "GetCommTimeouts failed";
    FormatWindowsError(GetLastError(), m_error);
    return false;
  }

  if (bBlocking)
  {
    cto.ReadIntervalTimeout         = 0;
    cto.ReadTotalTimeoutConstant    = 0;
    cto.ReadTotalTimeoutMultiplier  = 0;
  }
  else
  {
    cto.ReadIntervalTimeout         = MAXDWORD;
    cto.ReadTotalTimeoutConstant    = 0;
    cto.ReadTotalTimeoutMultiplier  = 0;
  }

  if (!SetCommTimeouts(m_handle, &cto))
  {
    m_error = "SetCommTimeouts failed";
    FormatWindowsError(GetLastError(), m_error);
    return false;
  }

  return true;
}

void CSerialPort::Close(void)
{
  CLockObject lock(&m_mutex);
  if (m_bIsOpen)
  {
    CloseHandle(m_handle);
    m_bIsOpen = false;
  }
}

int8_t CSerialPort::Write(CCECAdapterMessage *data)
{
  CLockObject lock(&m_mutex);
  DWORD iBytesWritten = 0;
  if (!m_bIsOpen)
    return -1;

  if (!WriteFile(m_handle, data->packet.data, data->size(), &iBytesWritten, NULL))
  {
    m_error = "Error while writing to COM port";
    FormatWindowsError(GetLastError(), m_error);
    return -1;
  }

  return (int8_t)iBytesWritten;
}

int32_t CSerialPort::Read(uint8_t* data, uint32_t len, uint64_t iTimeoutMs /* = 0 */)
{
  CLockObject lock(&m_mutex);
  int32_t iReturn(-1);
  DWORD iBytesRead = 0;
  if (m_handle == 0)
  {
    m_error = "Error while reading from COM port: invalid handle";
    return iReturn;
  }

  if(!ReadFile(m_handle, data, len, &iBytesRead, NULL) != 0)
  {
    m_error = "unable to read from device";
    FormatWindowsError(GetLastError(), m_error);
    iReturn = -1;
  }
  else
  {
    iReturn = (int32_t) iBytesRead;
  }

  return iReturn;
}

bool CSerialPort::SetBaudRate(uint32_t baudrate)
{
  int32_t rate = IntToBaudrate(baudrate);
  if (rate < 0)
    m_iBaudrate = baudrate > 0 ? baudrate : 0;
  else
    m_iBaudrate = rate;

  DCB dcb;
  memset(&dcb,0,sizeof(dcb));
  dcb.DCBlength = sizeof(dcb);
  dcb.BaudRate      = IntToBaudrate(m_iBaudrate);
  dcb.fBinary       = true;
  dcb.fDtrControl   = DTR_CONTROL_DISABLE;
  dcb.fRtsControl   = RTS_CONTROL_DISABLE;
  dcb.fOutxCtsFlow  = false;
  dcb.fOutxDsrFlow  = false;
  dcb.fOutX         = false;
	dcb.fInX          = false;
  dcb.fAbortOnError = true;

  if (m_iParity == PAR_NONE)
    dcb.Parity = NOPARITY;
  else if (m_iParity == PAR_EVEN)
    dcb.Parity = EVENPARITY;
  else
    dcb.Parity = ODDPARITY;

  if (m_iStopbits == 2)
    dcb.StopBits = TWOSTOPBITS;
  else
    dcb.StopBits = ONESTOPBIT;

  dcb.ByteSize = m_iDatabits;

  if(!SetCommState(m_handle,&dcb))
  {
    m_error = "SetCommState failed";
    FormatWindowsError(GetLastError(), m_error);
    return false;
  }

  return true;
}

bool CSerialPort::IsOpen()
{
  CLockObject lock(&m_mutex);
  return m_bIsOpen;
}
