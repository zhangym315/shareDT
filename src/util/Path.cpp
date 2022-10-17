#include <iostream>
#include <cstdio>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include "Path.h"
#include "Logger.h"

#ifdef __SHAREDT_WIN__
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

static String getExecuteFromPath(const char * arg)
{
    char* pPath;

#ifdef __SHAREDT_WIN__
    size_t len;
    errno_t err = _dupenv_s( &pPath, &len, "PATH" );
#else
    pPath = getenv ("PATH");
#endif

    String s(pPath);
    String delimiter = ":";  /* PATH delimiter */
    size_t pos;
    String token;

    while ((pos = s.find(delimiter)) != String::npos) {
        token = s.substr(0, pos);
        if(token.back() != '/')
            token.append("/");
        token.append(arg);
        struct stat buf{};

        if(stat (token.c_str(), &buf) == 0) {
                return token;
            }
        s.erase(0, pos + delimiter.length());
    }

    token = s.substr(0, pos);
    if(token.back() != '/')
        token.append("/");
    token.append(arg);
    struct stat buf{};

    if(stat (token.c_str(), &buf) == 0) {
            return token;
        }
    return token;
}

static String getCWD()
{
    char temp[MAX_PATH];
#ifdef __SHAREDT_WIN__
    char* buffer;
    if ( (buffer = _getcwd( NULL, 0 )) == NULL ) {
        perror( "_getcwd error" );
        return String("");
    } else {
        if(strlen(buffer) > MAX_PATH)
            perror( "current direcroty error: path length more than 256");
        else
            strcpy_s(temp, buffer);
        free(buffer);
    }
    return String (temp);
#else
    return (getcwd(temp, sizeof(temp)) ? String( temp ) : String("") );
#endif
}

static bool hasEnding (String const &fullString, String const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

static size_t stringReverserFind(const String & s, char c) {
    size_t i = s.length()-1;

    for (; i>0; i--) {
        if (s[i] != c) return i+1;
    }

    return i;
}

static String getParentDir(const String & path, int recur=1) {
    String ret;
    ret = path;
    do {
        ret = ret.back () == PATH_SEP_CHAR ?
              ret.substr (0, stringReverserFind(ret, PATH_SEP_CHAR)) : ret;
#ifdef __SHAREDT_WIN__
        ret = ret.substr(0, ret.rfind(PATH_SEP_CHAR));
#else
        ret = ret.substr(0, ret.rfind(PATH_SEP_CHAR)+1);
#endif
        recur --;
    } while(recur > 0);

    return ret;
}

ShareDTHome* ShareDTHome::_instance = nullptr;

ShareDTHome::ShareDTHome() : _valid(false) {}
void ShareDTHome::set(const char * argv)
{
    reSet(argv);
}

void ShareDTHome::reSet(const char *argv)
{
    _valid = false;
#if defined(_WIN32)
    TCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    _execPath = path;
    _home = getParentDir(path, 2);
    _execDir = getParentDir(path, 1);
    _valid = true;
#else
    String path(argv);
    String appended, execName;

    std::size_t pos;
    execName = String::npos != (pos = path.rfind ('/')) ? path.substr (pos + 1) : path;
    /* relative path */
    if(argv[0] == '.' && argv[1] == '/') {
        path = getCWD();
        path.append(&(argv[1]));   // skip the first .
    } else if (!strchr(argv, '/')) {
        path = getExecuteFromPath(argv);
        if(path.length() == 0) {
            LOGGER.error() << "Can't determine home path for: " << String(argv);
            return;
        }
    }

    /* replace /./ to / in the path */
    if((pos = path.find("/./")) != String::npos)
        path.replace(pos, pos+3, "/");

    appended = String(execName);
    /* be sure it's ending with bin/ShareDTServer */
    if(!hasEnding(path, appended))
    {
        LOGGER.error() << "Can't determine home path for: " << String(argv);
        return;
    }

    _execPath = path;
    path = path.substr(0, path.rfind(appended));
    _execDir = path;
    _home = getParentDir(path);
    _valid = true;
#endif
}

String & ShareDTHome::getHome()  { return _home; }
const String & ShareDTHome::getArgv0() const { return _execPath; }
ShareDTHome * ShareDTHome::instance()
{
    if (_instance == nullptr)
    {
        _instance = new ShareDTHome();
    }
    return _instance;
}

bool ShareDTHome::isValid() const  { return _valid; }

const String &ShareDTHome::getArgv0Dir() const {
    return _execDir;
}


CapServerHome* CapServerHome::_instance = nullptr;

CapServerHome::CapServerHome() : _valid(false) {
}

CapServerHome * CapServerHome::instance ()
{
    if(_instance == nullptr) {
        _instance = new CapServerHome();
    }
    return _instance;
}

void CapServerHome::setHome(const String &path, const String & cid)
{
    _home = path;
    _cid  = cid;
    _valid = true;
}

const String & CapServerHome::getHome() {
    return _home;
}
const String & CapServerHome::getCid() {
    return _cid;
}

void CapServerHome::init ()
{

}

void Path::write(const String &data)
{
    _ffs << data;
    _ffs.flush();
}

void Path::write(char * data)
{
    _ffs << data;
    _ffs.flush();
}

void Path::write(int data)
{
    _ffs << std::to_string(data);
    _ffs.flush();
}

int Path::readLineAsInt()
{
    char buffer[1024];
    _ffs.read (buffer,1024);
    return std::stoi(buffer);
}

String Path::readAll ()
{
    std::stringstream ret;
    ret << _ffs.rdbuf();
    return ret.str();
}

bool Path::checkAndWait(String & path, int waitSeconds)
{
    using namespace std;
    int count = waitSeconds * 2;
    while(!fs::exists(path) && waitSeconds < count)
    {
        waitSeconds++;
        this_thread::sleep_for(500ms);
    }

    return (waitSeconds == count);
}

void Path::removeContent(const String & path)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) 
        std::filesystem::remove_all(entry.path());
}