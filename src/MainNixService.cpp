#include "StartServer.h"
#include "MainService.h"
#include "Path.h"
#include "Logger.h"
#include "ThreadPool.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

CaptureProcessManager * CaptureProcessManager::_instance = 0;


/*
 * Fork new child process to run the command
 */
static void invokeCommand(char * command)
{

    /* create new process to run the command */
//    if ( fork() == 0 ) {
//        execv( argv[0], argv);
//        LOGGER.error() << "Failed to invoke command: " <<  command;
//    }

    return ;
}

void HandleCommandLine::setWID()
{
    // needs additional two arguments
    if(_argc > MAX_ARG-2)
        return;

    _argv[_argc++] = strdup("--wid");
    _argv[_argc++] = strdup(CapServerHome::instance()->getCid().c_str());
}

/*
 * New request from MainServiceServer
 *
 * 1. New capture, construct the capture id(cid)
 *    a). Generate cid and fork a new process.
 *    b). Add the cid and fd to CaptureProcessManager
 * 2. Stopping existing capture
 *    a). Find out the capture id and fd
 *    b). Send stopping message to capture process
 * 3. Start existing capture id
 *    a). Find out the capture directory
 *    b). Start the capture id
 *    c). Push the fd anc cid to CaptureProcessManager
 * 4. Delete capture id
 *    a). Find out the capture id and fd in CaptureProcessManager
 *    b). Inform capture server process to stop
 *    c). Delete the directory and records in CaptureProcessManager
 *
 * If new capture server reqeust is received
 *
 */
static void HandleCommandSocket(int fd, char * buf)
{
    Socket sk(fd);
    HandleCommandLine hcl(buf);

    hcl.initParsing();
    if(!hcl.hasWid())
        hcl.setWID();

    String capServer = CapServerHome::instance()->getHome();

    char ** argv = hcl.getArgv();

    LOGGER.info() << "New Capture Server Argument: " << hcl.toString();

    if(fork() == 0) {
        execv(argv[0], argv);
    }

    String ret("Capture ID(CID) = ");
    ret.append(CapServerHome::instance()->getCid());

    sk.send(ret.c_str());
}

/*
 * Create Service listening socket
 * Set _valid=true if success.
 */
MainServiceServer::MainServiceServer() : _valid(false), _backlog(10) // default 10 queued request
{
    const String pipeFile = CapServerHome::instance()->getHome() +
                            PATH_SEP_STR + FILEPIPE;
    int rc;
    sockaddr_un server_sockaddr;

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
    /* giving it a filepath to bind to.    */
    /*                                     */
    /* Unlink the file so the bind will    */
    /* succeed, then bind to that file.    */
    /***************************************/
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, pipeFile.c_str());

    unlink(pipeFile.c_str());
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
    const String pipeFile = CapServerHome::instance()->getHome() + PATH_SEP_STR + FILEPIPE;
    MainServiceServer mss;   // main service socket
    char buf[BUFSIZE];
    int clientSocket;
    ThreadPool tp(2);

    /* something error during init server socket */
    if(!mss.getValid())
        return RETURN_CODE_SERVICE_ERROR;

    /* start to listening */
    if(mss.listening())
        return RETURN_CODE_SERVICE_ERROR;

    while(true) {
        /* error on getting new client connection */
        if((clientSocket=mss.getNewConnection()) == -1)
            continue;

        LOGGER.info("waiting to read from cmd ...");

        int bytes_rec = recv(clientSocket, buf, sizeof(buf), 0);
        if (bytes_rec < -1 || bytes_rec > BUFSIZE){
            LOGGER.info("RECV ERROR from client: %d", bytes_rec);
            close(clientSocket);
            continue;
        }
        else {
            buf[bytes_rec] = '\0';
            LOGGER.info("ShareDTServer service DATA RECEIVED = %s, clientSocket=%d", buf, clientSocket);
        }

        // first check if stop command
        if(!memcmp(buf, MAIN_SERVICE_STOPPING, sizeof(MAIN_SERVICE_STOPPING))){
            LOGGER.info() << "Stopping ShareDTServer Service" ;
            break;
        }

        // send to thread pool to handle the request
        tp.enqueue(HandleCommandSocket, clientSocket, buf);
    }


    int rc = send(clientSocket, buf, strlen(buf), 0);
    if (rc == -1) {
        LOGGER.error("SEND ERROR ");
        close(clientSocket);
        return RETURN_CODE_SERVICE_ERROR;
    }
    else {
        LOGGER.info("Data sent to client to inform server is stopped");
    }

    LOGGER.info("ShareDTServer Service stopped");

    return 0;
}


MainServiceClient::MainServiceClient() : _valid(false)
{
    std::srand ((unsigned int) std::time (NULL));
    const String clientSockFile = CapServerHome::instance()->getHome() +
                            PATH_SEP_STR + "CLIENT_" + std::to_string (std::rand ());
    const String serverSockFile = CapServerHome::instance()->getHome() +
                            PATH_SEP_STR + FILEPIPE;
    int rc;
    socklen_t len;
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

int infoServiceToCapture(const char * execCmd)
{
    MainServiceClient msc;
    char buf[BUFSIZE];
    if(!msc.getValid())
        return RETURN_CODE_SERVICE_ERROR;

    if(msc.sentTo(execCmd) < 0)
        return RETURN_CODE_SERVICE_ERROR;

    if(msc.rcvFrom(buf, BUFSIZE) < 0)
        return RETURN_CODE_SERVICE_ERROR;

    LOGGER.noPre("%s", buf);
    return EXIT_SUCCESS;

}
