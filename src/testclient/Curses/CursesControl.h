#include <string>

class CursesControl {
  private:
    std::string in, out;

  public:
    CursesControl();
    CursesControl(std::string, std::string);
    ~CursesControl();
    void Init();
    void End();
    void SetInput(std::string);
    void SetOutput(std::string);
    int GetKey();
    void ParseCursesKey(const int&, std::string&);
};
