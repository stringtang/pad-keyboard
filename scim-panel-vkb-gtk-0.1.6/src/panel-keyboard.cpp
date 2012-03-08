/*
 * panel-keyboard.cpp : keyboard related staffs
 *
 * Copyright (C) 2009, Intel Corporation.
 *
 * Author: Raymond Liu <raymond.liu@intel.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <gtk/gtk.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
using namespace std;

static void
killchilds(int sig)
{
    kill (0, SIGTERM);
    exit (0);
}

unsigned long
launch_keyboard(const char *cmd, const char *params, int *vkb_pid)
{
    int    i = 0;
    int    stdout_pipe[2];
    int    stdin_pipe[2];
    char   buf[256], c;
    size_t n;

    unsigned long result = 0;

    if (pipe (stdout_pipe) != 0)
        return 0;
    if (pipe (stdin_pipe) != 0)
        return 0;

    switch (*vkb_pid = fork ()) {
    case 0:
        setpgid (0,0);
        signal(SIGQUIT, killchilds);
        signal(SIGTERM, killchilds);
        signal(SIGINT,  killchilds);
        signal(SIGHUP,  killchilds);

	/* Close the Child process' STDOUT */
        close (1);
        dup2(stdout_pipe[1], 1);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        execlp (cmd, cmd, params, NULL);

        // if run to here, then execlp must failed, so we send out a fake id to notify the parent and exit.
        fprintf(stdout, "0\n");
        fflush(stdout);
        exit(0);

    case -1:
      cerr << "### unable to launch VKB ### \n\n";
      return 0;
    }

    /* Parent */

    /* Close the write end of STDOUT */
    close(stdout_pipe[1]);

    /* FIXME: This could be a little safer... */
    do {
        n = read(stdout_pipe[0], &c, 1);
        if (n == 0 || c == '\n')
            break;
        buf[i++] = c;
    } while (i < 256);

    buf[i] = '\0';
    result = atol (buf);

    close(stdout_pipe[0]);
    if (result <= 0)
        cerr << "### Failed to launch '" << cmd << " " << params << "', is it installed? ### \n\n";

    return result;
}
