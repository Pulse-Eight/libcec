#include <curses.h>
#include <string>
#include <sstream>

using namespace std;

class CursesControl {
  private:
    string in, out;

  public:
    CursesControl();
    CursesControl(string&, string&);
    void Init();
    void End();
    void SetInput(string&);
    void SetOutput(string&);
    int GetKey();
    void ParseCursesKey(const int&, string&);
};
