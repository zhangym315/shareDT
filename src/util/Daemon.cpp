#include <stdlib.h>
#ifndef __SHAREDT_WIN__
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#endif
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "Daemon.h"
#include "Path.h"

DaemonizeProcess* DaemonizeProcess::_instance = 0;

DaemonizeProcess::DaemonizeProcess() {
}

DaemonizeProcess * DaemonizeProcess::instance ()
{
    if(_instance == 0) {
        _instance = new DaemonizeProcess();
    }
    return _instance;
}

#ifdef __SHAREDT_WIN__
/* windows do nothing */
void DaemonizeProcess::daemonize() {

}
#else

static void daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);
}

/**
 * \brief This function will daemonize this application
 */
void DaemonizeProcess::daemonize()
{
    daemon();
}

void DaemonizeProcess::daemonizeInit()
{
    /* Change the working directory  */
    const char * cwd = CapServerHome::instance()->getHome().c_str();
    chdir(cwd);

    /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
    char captureLog[MAX_PATH];
    memset(captureLog, 0, MAX_PATH);
    strcat(captureLog, cwd);
    strcat(captureLog, PATH_VNCSERVER_LOG);

    stdin  = fopen(captureLog, "r");
    stdout = fopen(captureLog, "w+");
    stderr = fopen(captureLog, "w+");

    /* Try to write PID of daemon to lockfile */
    char pid_file_name[MAX_PATH];
    memset(pid_file_name, 0, MAX_PATH);
    strcat(pid_file_name, cwd);
    strcat(pid_file_name, PATH_PID_FILE);

    char str[256];
    int pid_fd = open(pid_file_name, O_RDWR|O_CREAT|O_APPEND, 0640);

    if (pid_fd < 0) {
        /* Can't open lockfile */
        exit(EXIT_FAILURE);
    }

    if (lockf(pid_fd, F_TLOCK, 0) < 0) {
        /* Can't lock file */
        exit(EXIT_FAILURE);
    }

    /* Get current PID */
    snprintf(str, 256, "%d\n", getpid());

    /* Write PID to lockfile */
    write(pid_fd, str, strlen(str));
    close(pid_fd);
}
#endif
