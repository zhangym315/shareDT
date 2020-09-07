#ifndef SHAREDT_WINDOWSPROCESS_H
#define SHAREDT_WINDOWSPROCESS_H
#include <windows.h>
#include <vector>
#include <string>

class UserSession
{
  public:
    UserSession();

  private:
    DWORD _sessionId;
};

#endif //SHAREDT_WINDOWSPROCESS_H
