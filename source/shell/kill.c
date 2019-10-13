#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "jobsconst.h"


int 
main(int argc, char **argv)
{
	int fd = open(JOBS_FILENAME, O_RDONLY);

	int i;
	if (argc < 2) {
		printf(2, "usage: kill pid...\n");
		exit();
	}
	for (i = 1; i < argc; i++) {
		//printf(2,"%s\n", argv[i])
		if (argv[i][0] == '%') {//kill backstage process
			//int found = 0;
			char newargv[10];//remove %
			int len = strlen(argv[i]);
			for (int j = 0; j < len; j++) {
				newargv[j] = argv[i][j + 1];
			}
			newargv[len-1] = '\0';
			int id_to_found = atoi(newargv);
			//printf(2,"%s\n", newargv);
			//int fd;//file discriptor
			if (fd < 0) {//file not exist
				printf(2, "no backstage processes\n");
				continue;
			}
			char pid_backstage[10] = "";
			char buf[100]; 
			for(int k = 0; k < id_to_found; k++) {//get id from file
				strcpy(buf, "");
				int n = jobs_readline(fd, buf, sizeof(buf));
				//printf(2,"%d\n", n);
				if (n < 0) {
					printf(2,"read error\n");
					//found = -1; //no warnings again
					break;
				}
				else if (n == 0) {//read over
					printf(2, "process not found\n");
					break;
					}
			}
			if(id_to_found == 0) {
				printf(2,"process id numbered from 1\n");
				continue;
			}
			strcpy(pid_backstage, "");
			int j = 0;
			while (buf[j] == ' ') {
					j++;
			}
			while (buf[j] != ' ' && buf[j] != '\0') {
				char bufarray[2];
				bufarray[0] = buf[j];
				bufarray[1] = '\0';
				strcpy(pid_backstage + strlen(pid_backstage), bufarray);
				j++;
			}
			pid_backstage[strlen(pid_backstage)] = '\0';
			//printf(2,"%s\n", pid_backstage);
			kill(atoi(pid_backstage));//kill the backstage process
		}
		else {
			kill(atoi(argv[i]));
		}
	}
	exit();
}
