#ifndef SHAREDT_WINDOWSPROCESS_H
#define SHAREDT_WINDOWSPROCESS_H
#include <windows.h>
#include <vector>
#include "StringTools.h"

class UserSession
{
  public:
    UserSession(const std::string & user);
    bool GetSessionDomain(std::string & domain);
    bool GetSessionUserName(DWORD sid, std::string & username);
    DWORD getSid()      { return _sessionId; }
    HANDLE getToken()   { return _hToken; }
    bool  isValid()     { return _sessionId != 0; }
    const std::string & getReason() const { return _reason; }

  private:
    bool FindAndSetSessionIds();
    void FindAndSetTokens();

    std::string  _user;
    DWORD   _sessionId;
    HANDLE  _hToken;
    std::string  _reason;
};

#endif //SHAREDT_WINDOWSPROCESS_H
