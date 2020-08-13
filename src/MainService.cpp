#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "MainService.h"
#include "Path.h"
#include "Logger.h"
#include "MainConsole.h"

#ifdef __SHAREDT_WIN__
#include <windows.h>
#endif

HandleCommandLine::HandleCommandLine(char * buf) : _hasWid(false)
{
    char ** argv = (char **) malloc (sizeof(char * ) * MAX_ARG);
    char * token = strtok(buf, " ");
    int i = 0;

    // loop through the string to extract all other tokens
    while( token != NULL ) {
        argv[i++] = strdup(token);
        if(!strcmp(argv[i-1], "--wid"))
            _hasWid = true;
        token = strtok(NULL, " ");
    }

    argv[i] = NULL;
    _argv = argv;
    _argc = i;
}

HandleCommandLine::~HandleCommandLine()
{
    for(int i=0; i<_argc; i++) {
        if(_argv[i]) {
            free(_argv[i]);
            _argv[i] = NULL;
        }
    }

    if(_argv) free(_argv);
}

String HandleCommandLine::toString(int offset)
{
    if(offset < 0 || offset > _argc)
        return (String&) "";

    String ret;
    for(int i=offset; i<_argc; i++)
    {
        if(_argv[i])
        {
            ret.append(_argv[i]);
            ret.append(" ");
        }
    }

    return ret;
}

bool setMainProcessServiceHome(const struct cmdConf * conf)
{
    // get the main server running path
#ifdef __SHAREDT_WIN__
    TCHAR szPath[MAX_PATH];
    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        printf("Cannot get module file name\n");
        return false;
    }
    ShareDTHome::instance()->set(szPath);
#else
    ShareDTHome::instance()->set(conf->argv[0]);
#endif

    CapServerHome::instance()->setHome(ShareDTHome::instance()->getHome()+String(MAIN_SERVER_PATH), MAINSERVER);

    // create home working directory
    const String & path = CapServerHome::instance()->getHome();
    if(!fs::exists(path) && !fs::create_directory(path)) {
        std::cerr << "Failed to create working directory: " << path << std::endl;
        return false;
    }

    return true;
}

bool checkMainServiceStarted()
{
    return true;
}

bool setMainServiceFile()
{
    // set service log
    String pathLog = CapServerHome::instance()->getHome() + PATH_SEP_STR + String(MAINSER_LOG);
    LOGGER.setLogFile(pathLog.c_str());
    LOGGER.info() << "main service log set to: " << pathLog ;

#ifdef __SHAREDT_WIN__
    String pathPid = CapServerHome::instance()->getHome() + PATH_SEP_STR + String(PATH_PID_FILE);
    int curPid = _getpid();
    std::ofstream fs(pathPid.c_str());
    if(!fs)
    {
        LOGGER.error() << "Cannot open pid file: " << pathPid;
        return false;
    }
    fs<< curPid;
    fs.close();

    LOGGER.info() << "Starting server service process: " << curPid;
#else
    // Linux/MacOS will set the pid file later after fork
#endif
    return true;
}
