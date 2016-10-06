#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>
#include <fnmatch.h>
#include <ctype.h>
#include <spawn.h>
#include <sys/sysinfo.h>


#ifndef HOST_NAME_MAX
	#define HOST_NAME_MAX 256
#endif
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 4949 /*port*/
#define LISTENQ 8 /*maximum number of client connections */
#define MAX_ALLOC_SIZE (16 * 1024 * 1024)


//Malloc Allocation Handler
static void *set_malloc(size_t size) {
	void* ptr;
	assert(size < MAX_ALLOC_SIZE);

	ptr = malloc(size);
	if (ptr == NULL) {
		abort();
	}
	return ptr;
}

//
int GetNumber(const char *str) {
    while (!(*str >= '0' && *str <= '9') && (*str != '-') && (*str != '+')) str++;
    int number;
    if (sscanf(str, "%d", &number) == 1) {
        return number;
    }
        // No int found
        return -1;
}


//read file for fetching memory
int* readfile() {
    FILE * fp;
    char * line = NULL;
    int j;
    int* mem;

    mem = set_malloc(3);
    int i = 0;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if (i<3) {
            mem[i] = GetNumber(line);
            printf("total %d\n",mem[i]);
            i++;

        }
    }

    fclose(fp);
    if (line) {
        free(line);
    }

    return mem;
}

static int ultoc(char* out, unsigned long ul) {
    unsigned long temp = ul;
    int n = 1;
    while (temp > 10) {
        n++;
        temp = temp/10;
    }


    temp = ul;
    int i;
    for(i = n-1; i >= 0; i--) {
        out[i] = '0'+temp%10;
        temp = temp/10;
    }
    return n;
}

main()
{
	int* Mem;
	char* host;
	int sock,cli,n;
	struct sockaddr_in server, client;
	unsigned int len;
	char mesg[MAXLINE] = "";
	char list[MAXLINE] = "";
	int sent;
	char buf[MAXLINE];
	int isQuit = 0;
	int buf_len;


	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket: ");
		exit(-1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(SERV_PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&server.sin_zero, 8);

	len = sizeof(struct sockaddr_in);

	if ((bind(sock, (struct sockaddr *)&server, len)) == -1)
	{
		perror("bind");
		exit(-1);
	}

	host = set_malloc(HOST_NAME_MAX + 1);
	gethostname(host, HOST_NAME_MAX);

	strcat(list,"list ");
	strcat(list,host);
	strcat(list,"\n");
		
	strcat(mesg,"# munin node at ");
	strcat(mesg,host);
	strcat(mesg,"\n");

	while (isQuit==0)  {


        if ((listen(sock, 5)) == -1)
		{
			perror("listen");
			exit(-1);
		}


		if((cli = accept(sock, (struct sockaddr *)&client, &len)) ==-1)
		{
			perror("accept");
			exit(-1);
		}
			send(cli,mesg,strlen(mesg),0);
			buf_len = 1;
			while(buf_len) {
				buf_len = recv(cli, buf, MAXLINE,0);
				printf(buf);
			   	if ((buf[0]=='c')&&(buf[1]=='a')&&(buf[2]=='p'))
			   	{
			   		send(cli , "cap multigraph dirtyconfig\n" , strlen("cap multigraph dirtyconfig\n") , 0 );
			   	}
			   	else if (strcmp(buf,"config memory\n")==0)
                {
                    struct sysinfo info;
                    int n = sysinfo(&info);
                    char out[255];
                    char ram[255] = "";
                    ultoc(out, info.totalram);
                    strcat(ram,"graph_args --base 1024 -l 0 upper-limit ");
                    strcat(ram, out);
                    strcat(ram,"\n");
                    send(cli,ram,strlen(ram),0);
                    send(cli,"graph_vlabel Bytes\n",strlen("graph_vlabel Bytes\n"),0);
                    send(cli,"graph_title Memory usage\n",strlen("graph_title Memory usage\n"),0);
                    send(cli,"graph_category system\n",strlen("graph_category system\n"),0);
                    send(cli,"graph_info This graph shows this machine memory.\n",strlen("graph_info This graph shows this machine memory.\n"),0);
                    send(cli,"graph_order used free\n",strlen("graph_order used free\n"),0);
                    send(cli,"used.label used\n",strlen("used.label used\n"),0);
                    send(cli,"used.draw STACK\n",strlen("used.draw STACK\n"),0);
                    send(cli,"used.info Used memory.\n",strlen("used.info Used memory.\n"),0);
                    send(cli,"free.label free\n",strlen("free.label free\n"),0);
                    send(cli,"free.draw STACK\n",strlen("free.draw STACK\n"),0);
                    send(cli,"free.info Free memory.\n",strlen("free.info Free memory.\n"),0);
                    send(cli,".\n",strlen(".\n"),0);
			   	}

			   	else if (strcmp(buf,"nodes\n")==0) {
			   		send(cli,host,strlen(host),0);
			   		send(cli,"\n",strlen("\n"),0);
			   		send(cli,".\n",strlen(".\n"),0);
			   	}
			   	else if (strcmp(buf,list)==0) {
			   		send(cli,"memory\n",strlen("memory\n"),0);
			   	}
			   	else if (strcmp(buf,"version\n")==0) {
			   		send(cli,"lovely node on ",strlen("lovely node on "),0);
                	send(cli,host,strlen(host),0);
                 	send(cli," version: 1.23\n",strlen(" version: 1.23\n"),0);
			   	}


			   	else if (strcmp(buf,"fetch memory\n")==0) {
			   		if ((buf[6]=='m') && (buf[7]=='e') &&(buf[8]=='m') &&(buf[9]=='o') &&(buf[10]=='r') &&(buf[11]=='y') &&(buf[12]=='\n')) {
			   		   	struct sysinfo info;
			   		   	sysinfo(&info);
						                    
	                    char used_m[MAXLINE],us[MAXLINE] = "";
	                    char free_m[MAXLINE],fr[MAXLINE] = "";
		                strcat(used_m,"used.value ");
						strcat(free_m,"free.value ");
	                    sprintf(us,"%llu",info.bufferram *(unsigned long long)info.mem_unit);
	                    sprintf(fr,"%llu",info.freeram *(unsigned long long)info.mem_unit);
	                    strcat(used_m,us);
	                    strcat(used_m,"\n");

	                    strcat(free_m,fr);
	                    strcat(free_m,"\n");
	                    send(cli,used_m,strlen(used_m),0);
	                    send(cli,free_m,strlen(free_m),0);
	                    send(cli,".\n",strlen(".\n"),0);
	                }
			   	}

			   	else if (strcmp(buf,"quit \n")==0) {
			   		//isQuit = 1;
			   		buf_len = 0;

			   	}

			   	else if (strcmp(buf,"keluar\n")==0) {
			   		isQuit = 1;
			   		buf_len = 0;

			   	}
			   	else if ((strstr(buf,"fetch")!=NULL)||(strstr(buf,"config")!= NULL)||(strstr(buf,"list")!=NULL)) {
			   		char msg[MAXLINE] = "";
			   		strcat (msg,"# Unknown service\n");
			   		send(cli,msg,strlen(msg),0);
			   	}
			   	else {
			   		char msg[MAXLINE] = "";
			   		strcat (msg,"# Unknown command. Try cap, list, nodes, config, fetch, version or quit\n");
			   		send(cli,msg,strlen(msg),0);
			   	}
		   	//printf("%s",buf);
			   	bzero(buf,MAXLINE);
 			}

 			close(cli);
 


		//printf("send %d bytes to client : %s\n", sent, inet_ntoa(client.sin_addr));




	}
	close(sock);
	
	return 0;
}


