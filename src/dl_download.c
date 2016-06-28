/* ympd
   (c) 2013-2014 Andrew Karpow <andy@ndyk.de>
   This project's homepage is: http://www.ympd.org
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <mpd/client.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mpd/directory.h>
#include <mpd/recv.h>


int download_stream(char *p_charbuf, char *dir)
{
	
	int pipefd[2];
	char res[4096];
	int status;
	FILE *fp;
	
	if (pipe(pipefd) == -1){
		exit(EXIT_FAILURE);
	}
	
	
	pid_t pid = fork();
	if (pid == 0) /*child */
	{
		close(pipefd[0]);  /* close unused read end */
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		
		char cmd[512] = "cd ";
		strcat(cmd, dir);
		strcat(cmd, " ; youtube-dl --no-mtime --restrict-filenames --extract-audio --audio-format=m4a -o '%(title)s.%(ext)s' --write-description ");
		strcat(cmd, p_charbuf);
		strcat(cmd, " ; mpc update --wait && VV=$(ls *.description -t | head -n1) ; VV=$(basename $VV .description) ; mpc add $VV.m4a && echo $VV ; find -type f -mtime +5 -delete");
                
                fprintf(pipefd[1], "/bin/bash -c %s\n\n", cmd);
                
                
        char *name[] = {"/bin/bash", "-c", cmd, NULL };
        
        
        
        execvp(name[0], name);
        _exit(EXIT_FAILURE);

	}
	else
	{
	        fp = fopen("/tmp/log", "w+");
	        fprintf(fp, "begin\n");
	        fprintf(fp , "Fork\n");
		close(pipefd[1]); /* close write end */
		waitpid(pid, &status, 0);
		read(pipefd[0], res, sizeof(res));
		
		fprintf(fp, "Status: %d\n", status);
		fprintf(fp, "Output:\n%s\n", res);
		
		if(status == 0)
		{
		        fprintf(fp, "\n\nDownloaded\n");
			fprintf(stderr, "Downloaded:\n %s \n", res);
			fclose(fp);
			return 0;
		}
		else
		{
		        fprintf(fp, "\n\nNot Downloaded\n");
			fprintf(stderr, "Error while downloading :\n %s\n", p_charbuf);
			fclose(fp);
			return 1;
		}

	}
	
}
