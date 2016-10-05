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




main()
{
	char* host;
	int sock,cli,n;
	struct sockaddr_in server, client;
	unsigned int len;
	char mesg[] = "hello\n";
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


	//Set Host Name
	host = set_malloc(HOST_NAME_MAX + 1);
	gethostname(host, HOST_NAME_MAX);
		

		while (isQuit==0)  {
			//mesg = strcat("# munin node at ",host);
			send(cli,mesg,strlen(mesg),0);
			buf_len = 1;
			while(buf_len) {
				buf_len = recv(cli, buf, MAXLINE,0);
			   	if (strcmp(buf,"cap\n")==0) {
			   		send(cli , "cap multigraph dirtyconfig\n" , strlen("cap multigraph dirtyconfig\n") , 0 );
					
			   	}
			   	else if (strcmp(buf,"node\n")==0) {
			   		send(cli,host,strlen(host),0);
			   	}
			   	else if (strcmp(buf,"quit\n")==0) {
			   		isQuit = 1;
			   		buf_len = 0;
					
			   	}
		   	//printf("%s",buf);
			}			
   		
		   	
		

		//printf("send %d bytes to client : %s\n", sent, inet_ntoa(client.sin_addr));

		


	}
	close(cli);

	return 0;
}


