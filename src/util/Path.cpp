#include <iostream>
#include <cstdio>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include "Path.h"
#include "Logger.h"
#include <string>

#ifdef __SHAREDT_WIN__
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

static std::string getExecuteFromPath(const char * arg)
{
    char* pPath;

#ifdef __SHAREDT_WIN__
    size_t len;
    errno_t err = _dupenv_s( &pPath, &len, "PATH" );
#else
    pPath = getenv ("PATH");
#endif

    std::string s(pPath);
    std::string delimiter = ":";  /* PATH delimiter */
    size_t pos;
    std::string token;

    while ((pos = s.find(delimiter)) != std::string::npos) {
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

static std::string getCWD()
{
    char temp[MAX_PATH];
#ifdef __SHAREDT_WIN__
    char* buffer;
    if ( (buffer = _getcwd( NULL, 0 )) == NULL ) {
        perror( "_getcwd error" );
        return std::string("");
    } else {
        if(strlen(buffer) > MAX_PATH)
            perror( "current direcroty error: path length more than 256");
        else
            strcpy_s(temp, buffer);
        free(buffer);
    }
    return std::string (temp);
#else
    return (getcwd(temp, sizeof(temp)) ? std::string( temp ) : std::string("") );
#endif
}

static bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

static size_t stringReverserFind(const std::string & s, char c) {
    size_t i = s.length()-1;

    for (; i>0; i--) {
        if (s[i] != c) return i+1;
    }

    return i;
}

static std::string getParentDir(const std::string & path, int recur=1) {
    std::string ret;
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
    LPSTR path;
    GetModuleFileNameA(NULL, path, MAX_PATH);

    _execPath = reinterpret_cast<const char *const>(path);
    _home = getParentDir(path, 2);
    _execDir = getParentDir(path, 1);
    _valid = true;
#else
    std::string path(argv);
    std::string appended, execName;

    std::size_t pos;
    execName = std::string::npos != (pos = path.rfind ('/')) ? path.substr (pos + 1) : path;
    /* relative path */
    if(argv[0] == '.' && argv[1] == '/') {
        path = getCWD();
        path.append(&(argv[1]));   // skip the first .
    } else if (!strchr(argv, '/')) {
        path = getExecuteFromPath(argv);
        if(path.length() == 0) {
            LOGGER.error() << "Can't determine home path for: " << std::string(argv);
            return;
        }
    }

    /* replace /./ to / in the path */
    if((pos = path.find("/./")) != std::string::npos)
        path.replace(pos, pos+3, "/");

    appended = std::string(execName);
    /* be sure it's ending with bin/ShareDTServer */
    if(!hasEnding(path, appended))
    {
        LOGGER.error() << "Can't determine home path for: " << std::string(argv);
        return;
    }

    _execPath = path;
    path = path.substr(0, path.rfind(appended));
    _execDir = path;
    _home = getParentDir(path);
    _valid = true;
#endif
}

std::string & ShareDTHome::getHome()  { return _home; }
const std::string & ShareDTHome::getArgv0() const { return _execPath; }
ShareDTHome * ShareDTHome::instance()
{
    if (_instance == nullptr)
    {
        _instance = new ShareDTHome();
    }
    return _instance;
}

bool ShareDTHome::isValid() const  { return _valid; }

const std::string &ShareDTHome::getArgv0Dir() const {
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

void CapServerHome::setHome(const std::string &path, const std::string & cid)
{
    _home = path;
    _cid  = cid;
    _valid = true;
}

const std::string & CapServerHome::getHome() {
    return _home;
}
const std::string & CapServerHome::getCid() {
    return _cid;
}

void CapServerHome::init ()
{

}

void Path::write(const std::string &data)
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

std::string Path::readAll ()
{
    std::stringstream ret;
    ret << _ffs.rdbuf();
    return ret.str();
}

bool Path::checkAndWait(std::string & path, int waitSeconds)
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

void Path::removeContent(const std::string & path)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) 
        std::filesystem::remove_all(entry.path());
}