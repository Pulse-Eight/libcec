#include "CursesControl.h"
#include <curses.h>
#include <sstream>

CursesControl::CursesControl()
{ 
  Init();
}

CursesControl::~CursesControl()
{
  End();
}

CursesControl::CursesControl(std::string in, std::string out)
{
  this->in  = in;
  this->out = out;
  Init();
}

void CursesControl::Init()
{
  initscr();
  noecho();
  keypad(stdscr, TRUE);
  printw("Curses enabled.");
}

void CursesControl::End()
{
  endwin();
  printw("Curses closed.");
}

int CursesControl::GetKey()
{
  return getch();
}

void CursesControl::ParseCursesKey(const int& key, std::string& input)
{
  std::stringstream data;
  data << "tx" << " " << this->in << this->out << " " << "44" << " "; 
  switch(key){
    case KEY_DOWN:
      data << "42";
      input = data.str();
      break;
    case KEY_UP:
      data << "41";
      input = data.str();
      break;
    case 109:  // KEY_m
      data << "43";
      input = data.str();
      break;
    case 10:   // KEY_ENTER
      data << "6B";
      input = data.str();
      break;
    case 113: // KEY_q
      input = "q";
      break;
  }
}
