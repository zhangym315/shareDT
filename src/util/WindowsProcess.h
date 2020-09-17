#ifndef SHAREDT_WINDOWSPROCESS_H
#define SHAREDT_WINDOWSPROCESS_H
#include <windows.h>
#include <vector>
#include "StringTools.h"

class UserSession
{
  public:
    UserSession(const String & user);
    bool GetSessionDomain(String & domain);
    bool GetSessionUserName(DWORD sid, String & username);
    DWORD getSid()      { return _sessionId; }
    HANDLE getToken()   { return _hToken; }
    bool  isValid()     { return _sessionId != 0; }
    const String & getReason() const { return _reason; }

  private:
    bool FindAndSetSessionIds();
    void FindAndSetTokens();

    String  _user;
    DWORD   _sessionId;
    HANDLE  _hToken;
    String  _reason;
};

#endif //SHAREDT_WINDOWSPROCESS_H
