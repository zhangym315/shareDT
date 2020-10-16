#include "StartServer.h"
#include "MainService.h"
#include "ReadWriteFD.h"
#include "Path.h"
#include "Logger.h"
#include "ThreadPool.h"
#include "MainManagementProcess.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __SHAREDT_LINUX__
#include <sys/wait.h>
#include <signal.h>
#endif

CaptureProcessManager * CaptureProcessManager::_instance = 0;


static void sig_child(int signo)
{
     pid_t  pid;
     int    stat;

     (void) signo;
     while ((pid = waitpid(-1, &stat, WNOHANG)) >0)
            LOGGER.info("Child process %d terminated.", pid);

}

/*
 * Create Service listening socket
 * Set _valid=true if success.
 */
MainServiceServer::MainServiceServer() : _valid(false), _backlog(10) // default 10 queued request
{
    int rc;
    sockaddr_un server_sockaddr;

    _socketFile = CapServerHome::instance()->getHome() +
                  PATH_SEP_STR + SOCKET_FILE;
    /**************************************/
    /* Create a UNIX domain stream socket */
    /**************************************/
    _serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (_serverSock == -1){
        LOGGER.error("SOCKET ERROR\n");
        return ;
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to bind to.     */
    /*                                     */
    /* Unlink the file so the bind will     */
    /* succeed, then bind to that file.     */
    /***************************************/
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, _socketFile.c_str());

    unlink(_socketFile.c_str());
    rc = ::bind(_serverSock, (struct sockaddr *) &server_sockaddr,
                SUN_LEN(&server_sockaddr));
    if (rc == -1){
        LOGGER.error("BIND ERROR");
        close(_serverSock);
        return;
    }

    setValid(true);
}

MainServiceServer::~MainServiceServer()
{
    LOGGER.info("Closing all socket connecting to client");
    close(_serverSock);

    for(std::vector<int>::const_iterator it=_clientsSock.begin();
            it!=_clientsSock.end(); it++) {
        close(*it);
    }
}

void MainServiceServer::removeSocketFile()
{
    std::remove(_socketFile.c_str());
    LOGGER.info() << "Removed socket file: " << _socketFile;
}

/*********************************/
/* Listen for any client sockets */
/*********************************/
int MainServiceServer::listening()
{
    LOGGER.info("Main Service socket started to listening...");
    int rc = ::listen(_serverSock, _backlog);
    if (rc == -1){
        LOGGER.error("LISTEN ERROR");
        close(_serverSock);
    }
    return rc;
}

/*********************************/
/* Accept an incoming connection */
/*********************************/
int MainServiceServer::getNewConnection()
{
    int client_sock = ::accept(_serverSock, NULL, NULL);
    if (client_sock == -1){
        LOGGER.error("ACCEPT ERROR");
        close(client_sock);
        return client_sock;
    }

    LOGGER.info("New Client request received, clientSocket=%d", client_sock);

    _clientsSock.push_back(client_sock);
    return client_sock;
}

int MainWindowsServices() {
    const String pipeFile = CapServerHome::instance()->getHome() + PATH_SEP_STR + SOCKET_FILE;
    char buf[BUFSIZE];
    int clientSocket;
    ThreadPool tp(2);

    signal(SIGCHLD,sig_child);

    /* Main service port */
    SocketServer ss(SHAREDT_INTERNAL_PORT_START, 10);
    LOGGER.info() << "MainService started on port=" << ss.getPort() ;
    String alive = ShareDTHome::instance()->getHome() + String(MAIN_SERVER_PATH) + String(PATH_ALIVE_FILE);
    {
        if(fs::exists(alive) && !fs::remove(alive)){
            LOGGER.error() << "Failed to remove the file: " << alive;
            return RETURN_CODE_SERVICE_ERROR;
        }
        Path aliveWriter(alive);
        aliveWriter.write(ss.getPort());
    }

    while(true) {
        Socket* s=ss.Accept();
        String received = s->ReceiveBytes();
        LOGGER.info("ShareDTServer service DATA RECEIVED CMD=\"%s\", "
                        " clientSocket=%d", received.c_str(), s->getSocket());


        bzero(buf, BUFSIZE);
        strcpy(buf, received.c_str());

        /* first check if stop command */
        if(!memcmp(buf, MAIN_SERVICE_STOPPING, sizeof(MAIN_SERVICE_STOPPING))){
            stopAllSC();
            s->send("ShareDTServer Stopped");
            break;
        }

        LOGGER.info() << "Client connected, creating a processing thread.";

        /* send to thread pool to handle the request */
        tp.enqueue(HandleCommandSocket, s, buf);
    }

    LOGGER.info("ShareDTServer Service stopped");

    return 0;
}


MainServiceClient::MainServiceClient() : _valid(false)
{
    std::srand ((unsigned int) std::time (NULL));
    const String serverSockFile = CapServerHome::instance()->getHome() +
                            PATH_SEP_STR + SOCKET_FILE;
    int rc;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;

    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));

    /****************************************/
    /* Create a UNIX domain datagram socket */
    /****************************************/
    _clientSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (_clientSock == -1) {
        LOGGER.noPre("SOCKET ERROR: error created");
        return ;
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* for the server socket and connect   */
    /* to it.                              */
    /***************************************/
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, serverSockFile.c_str());

    rc = connect(_clientSock, (struct sockaddr *) &server_sockaddr,
                SUN_LEN(&server_sockaddr));
    if(rc == -1){
        perror("connect() failed");
        close(_clientSock);
        return ;
    }
    _valid = true;
}

MainServiceClient::~MainServiceClient()
{
    close(_clientSock);
}

int MainServiceClient::sentTo(const char * cmd)
{
    int rc = send(_clientSock, cmd, strlen(cmd), 0);
    if (rc == -1) {
        LOGGER.noPre("SEND ERROR to server");
        close(_clientSock);
    }

    return rc;
}

int MainServiceClient::rcvFrom(char * buf, size_t size)
{
    int rc = recv(_clientSock, buf, size, 0);
    if (rc == -1) {
        LOGGER.noPre("RECV ERROR from server");
        close(_clientSock);
    }
    buf[rc] = '\0';
    return rc;
}
