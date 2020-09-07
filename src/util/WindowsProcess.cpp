#include <windows.h>
#include <vector>
#include <string>

#include <WtsApi32.h>
#pragma comment(lib, "WtsApi32.lib")

//
typedef std::basic_string<TCHAR> tstring;

// Get current sessions
bool EnumSessionIds(std::vector<DWORD>& list)
{
    list.clear();

    WTS_SESSION_INFO *pSI = NULL;
    DWORD dwSICount;

    BOOL bRes = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSI, &dwSICount);
    if (bRes == 0)
        return false;

    for (unsigned int i = 0; i < dwSICount; ++i)
        list.push_back(pSI[i].SessionId);

    WTSFreeMemory(pSI);

    return true;
}

// Get username from session id
bool GetSessionUserName(DWORD dwSessionId, tstring& username)
{
    LPTSTR	pBuffer = NULL;
    DWORD	dwBufferLen;

    BOOL bRes = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSUserName, &pBuffer, &dwBufferLen);

    if (bRes == FALSE)
        return false;

    username = pBuffer;
    WTSFreeMemory(pBuffer);

    return true;
}

// Get domain name from session id
bool GetSessionDomain(DWORD dwSessionId, tstring& domain)
{
    LPTSTR	pBuffer = NULL;
    DWORD	dwBufferLen;

    BOOL bRes = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSDomainName, &pBuffer, &dwBufferLen);

    if (bRes == FALSE)
        return false;

    domain = pBuffer;
    WTSFreeMemory(pBuffer);

    return true;
}
