/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "PTY.h"

#ifdef VISIT_USE_PTY

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#if !defined(TIOCGWINSZ) || defined(__APPLE__)
#include <sys/ioctl.h>   // 44BSD requires this too
#endif

// ****************************************************************************
//  Function:  pty_fork
//
//  Purpose:
//    Like fork(), but creates a PTY and returns its file descriptor/name.
//
//  Arguments:
//    fdm       : new file descriptor
//    slave_name: new pty path
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 22 15:00:37 PDT 2002
//    Added an optional signal child handler.  Since setting this must happen
//    before fork() and after grantpt(), it must be set in this function.
//
// ****************************************************************************
pid_t
pty_fork(int &fdm, void (*sigchld_handler)(int), char *slave_name)
{
    int         fds;
    pid_t       pid;
    char        pts_name[20];

    if ( (fdm = ptym_open(pts_name)) < 0)
        fprintf(stderr,"can't open master pty: %s, returned %d\n", pts_name, fdm);
    if (slave_name != NULL)
        strcpy(slave_name, pts_name);   // return name of slave

    if (sigchld_handler != NULL)
        signal(SIGCHLD, sigchld_handler);

    if ( (pid = fork()) < 0)
        return(-1);

    else if (pid == 0) {                // child
        if (setsid() < 0)
            fprintf(stderr,"setsid error");

        // SVR4 acquires controlling terminal on open()
        if ( (fds = ptys_open(fdm, pts_name)) < 0)
            fprintf(stderr,"can't open slave pty: %d\n",fds);
        close(fdm);             // all done with master in child

#if     defined(TIOCSCTTY) && !defined(CIBAUD)
        // 44BSD way to acquire controlling terminal
        // !CIBAUD to avoid doing this under SunOS
        if (ioctl(fds, TIOCSCTTY, (char *) 0) < 0)
            fprintf(stderr,"TIOCSCTTY error");
#endif
        // slave becomes stdin/stdout/stderr of child
        if (dup2(fds, STDIN_FILENO) != STDIN_FILENO)
            fprintf(stderr,"dup2 error to stdin");
        if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
            fprintf(stderr,"dup2 error to stdout");
        if (dup2(fds, STDERR_FILENO) != STDERR_FILENO)
            fprintf(stderr,"dup2 error to stderr");
        if (fds > STDERR_FILENO)
            close(fds);
        return(0);              // child returns 0 just like fork()

    } else {            // parent
        return(pid);    // return pid of child
    }
}


#ifdef PTY_SYSV

// ****************************************************************************
//  Method:  ptym_open  (SYSV)
//
//  Purpose:
//    Open the pty master.
//
//  Arguments:
//    pts_name:  output pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
// ****************************************************************************
#include <visitstream.h>
int
ptym_open(char *pts_name)
{
    char *ptr;
    int   fdm;

    strcpy(pts_name, "/dev/ptmx");  // in case open fails
    if ( (fdm = open(pts_name, O_RDWR)) < 0)
        return(-1);
    if (grantpt(fdm) < 0) {         // grant access to slave
        close(fdm);
        return(-2);
    }
    if (unlockpt(fdm) < 0) {        // clear slave's lock flag
        close(fdm);
        return(-3);
    }
    if ( (ptr = ptsname(fdm)) == NULL) {    // get slave's name
        close(fdm);
        return(-4);
    }

    strcpy(pts_name, ptr);  // return name of slave
    return(fdm);            // return fd of master
}

// ****************************************************************************
//  Method:  ptys_open  (SYSV)
//
//  Purpose:
//    Open the pty slave
//
//  Arguments:
//    fdm      : the master file descriptor
//    pts_name : pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu May 17 11:32:26 PDT 2001
//    Removed a line of debug output.
//
//    Eric Brugger, Mon Oct 23 13:34:37 PDT 2006
//    Remove the checks for the error codes from the ioctl calls since they
//    may fail on some systems.
//
//    Eric Brugger, Tue Mar 12 11:48:46 PDT 2013
//    Remove the ioctl calls since streams are obsolete.
//
// ****************************************************************************
int
ptys_open(int fdm, char *pts_name)
{
    int             fds;

    // following should allocate controlling terminal
    if ( (fds = open(pts_name, O_RDWR)) < 0) {
        close(fdm);
        return(-5);
    }

    return(fds);
}


#else
#include "FileFunctions.h"

// ****************************************************************************
// Method: ug_RW
//
// Purpose: 
//   Checks if a file has RW permissions for usr/group.
//
// Arguments:
//   name : The name of the file to check.
//
// Returns:    True if the permissions are RW for usr/group.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 19 16:18:55 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

static bool
ug_RW(const char *name)
{
    VisItStat_t s;
    VisItStat(name, &s);

    bool uRW = ((s.st_mode & S_IRUSR) != 0) &&
               ((s.st_mode & S_IWUSR) != 0);

    bool gRW = ((s.st_mode & S_IRGRP) != 0) &&
               ((s.st_mode & S_IWGRP) != 0);

    return uRW && gRW;
}

// ****************************************************************************
//  Method:  ptym_open  (BSD)
//
//  Purpose:
//    Open the pty master.
//
//  Arguments:
//    pts_name:  output pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Apr 19 16:18:30 PDT 2010
//    I made sure that the pty and tty both have 660 permissions before trying
//    to use either one.
//
// ****************************************************************************

int
ptym_open(char *tty_name)
{
    int  fdm = -1;
    char pty_name[20];

    strcpy(pty_name, "/dev/ptyXY");
    strcpy(tty_name, "/dev/ttyXY");

    // array index: 0123456789 (for references in following code)
    for (const char *ptr1 = "pqrstuvwxyzPQRST"; *ptr1 != 0; ptr1++)
    {
        pty_name[8] = *ptr1;
        tty_name[8] = *ptr1;

        for (const char *ptr2 = "0123456789abcdef"; *ptr2 != 0; ptr2++)
        {
            pty_name[9] = *ptr2;
            tty_name[9] = *ptr2;

            // Make sure that pty and tty have compatible permissions.
            if(!ug_RW(pty_name))
                continue;
            if(!ug_RW(tty_name))
                continue;

            // try to open master
            if ( (fdm = open(pty_name, O_RDWR)) < 0)
            {
                if (errno == ENOENT)    // different from EIO
                    return(-1);         // out of pty devices
                else
                    continue;           // try next pty device
            }

            return(fdm);        // got it, return fd of master
        }
    }

    return(-1);         // out of pty devices
}

// ****************************************************************************
//  Method:  ptys_open  (BSD)
//
//  Purpose:
//    Open the pty slave
//
//  Arguments:
//    fdm      : the master file descriptor
//    pts_name : pts name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 27, 2001
//
// ****************************************************************************
int
ptys_open(int fdm, char *pts_name)
{
    int fds;

    if ( (fds = open(pts_name, O_RDWR)) < 0) {
        close(fdm);
        return(-1);
    }
    return(fds);
}




#endif


#endif
