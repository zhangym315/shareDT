#include "WindowsProcess.h"
#include "Logger.h"

#include <windows.h>
#include <vector>
#include <WtsApi32.h>

#pragma comment(lib, "WtsApi32.lib")

typedef std::basic_string<TCHAR> tstring;

void UserSession::FindSessionIds()
{
    WTS_SESSION_INFO *pSI = NULL;
    DWORD dwSICount;

    BOOL bRes = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSI, &dwSICount);
    if (bRes == 0)
        return ;

    String usrSession;
    for (unsigned int i = 0; i < dwSICount; ++i)
    {
        if(GetSessionUserName(pSI[i].SessionId, usrSession) &&
            _user == usrSession )
        {
            _sessionId = pSI[i].SessionId;
            break;
        }
    }

    WTSFreeMemory(pSI);
}

bool UserSession::GetSessionUserName(DWORD sid, String & username)
{
    LPTSTR	pBuffer = NULL;
    DWORD	dwBufferLen;

    BOOL bRes = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                           sid, WTSUserName, &pBuffer, &dwBufferLen);

    if (bRes == FALSE)
        return false;

    username = pBuffer;
    WTSFreeMemory(pBuffer);

    return true;
}

bool UserSession::GetSessionDomain(String & domain)
{
    LPTSTR	pBuffer = NULL;
    DWORD	dwBufferLen;

    BOOL bRes = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                            _sessionId, WTSDomainName, &pBuffer, &dwBufferLen);
    if (bRes == FALSE)
        return false;

    domain = pBuffer;
    WTSFreeMemory(pBuffer);

    return true;
}

UserSession::UserSession(const String & user) : _user(user) , _sessionId(0)
{
    LOGGER.info() << "UserSession user: " << user;
    FindSessionIds();
}

