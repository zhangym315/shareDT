#include "WindowsProcess.h"
#include "Logger.h"

#include <windows.h>
#include <vector>
#include <WtsApi32.h>

#pragma comment(lib, "WtsApi32.lib")

bool UserSession::FindAndSetSessionIds()
{
    WTS_SESSION_INFO *pSI = NULL;
    DWORD dwSICount;

    BOOL bRes = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSI, &dwSICount);
    if (bRes == 0)
    {
        _reason = "Failed to enumerating sessions.";
        LOGGER.error() << _reason;
        return false;
    }

    std::string usrSession;
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
    return true;
}

bool UserSession::GetSessionUserName(DWORD sid, std::string & username)
{
    LPTSTR	pBuffer = NULL;
    DWORD	dwBufferLen;

    BOOL bRes = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                           sid, WTSUserName, &pBuffer, &dwBufferLen);
    if (bRes == FALSE) {
        LOGGER.error() << "Failed to query session information for user name";
        return false;
    }

    username = pBuffer;
    WTSFreeMemory(pBuffer);

    return true;
}

bool UserSession::GetSessionDomain(std::string & domain)
{
    LPTSTR	pBuffer = NULL;
    DWORD	dwBufferLen;

    BOOL bRes = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                            _sessionId, WTSDomainName, &pBuffer, &dwBufferLen);
    if (bRes == FALSE)
    {
        LOGGER.error() << "Failed to query session information for domain" ;
        return false;
    }

    domain = pBuffer;
    WTSFreeMemory(pBuffer);

    return true;
}

UserSession::UserSession(const std::string & user) : _user(user) , _sessionId(0)
{
    if(FindAndSetSessionIds())
        FindAndSetTokens();
}

void UserSession::FindAndSetTokens()
{
    if (!WTSQueryUserToken (_sessionId, &_hToken))
    {
        _reason = "Error on querying token for user: " + _user;
        LOGGER.error() << _reason;
        return;
    }
}