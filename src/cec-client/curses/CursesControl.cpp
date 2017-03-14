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

#include "CursesControl.h"
#include "p8-platform/util/StringUtils.h"
#include <curses.h>

void CCursesControl::Init()
{
  initscr();
  noecho();
  keypad(stdscr, true);
  printw("Curses enabled.");
}

void CCursesControl::End(void)
{
  endwin();
  printw("Curses closed.");
}

void CCursesControl::SetInput(const std::string& in)
{
  m_in = in;
}

void CCursesControl::SetOutput(const std::string& out)
{
  m_out = out;
}

std::string CCursesControl::ParseCursesKey(void)
{
  int key = getch();
  std::string strKey;
  switch(key){
    case KEY_DOWN:
      strKey = "42";
      break;
    case KEY_UP:
      strKey = "41";
      break;
    case 109:  // KEY_m
      strKey = "43";
      break;
    case 10:   // KEY_ENTER
      strKey = "6B";
      break;
    case 113: // KEY_q
      strKey = "q";
      break;
  }

  return strKey.empty() ?
      "" :
      StringUtils::Format("tx %s%s 44 %s", m_in.c_str(), m_out.c_str(), strKey.c_str());
}
