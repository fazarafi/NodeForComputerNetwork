#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 4949 /*port*/
#define LISTENQ 8 /*maximum number of client connections */

main()
{
	int sock,cli,n;
	struct sockaddr_in server, client;
	unsigned int len;
	char mesg[] = "# munin node at MyComputer\n";	
	int sent;
	char buf[MAXLINE];
	int isQuit = 0;

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
	sent = send(cli, mesg, strlen(mesg), 0);

		
		
		while (!isQuit && (n = recv(cli, buf, MAXLINE,0)) > 0 )  {
			//printf("%s","String received from and resent to the client:");
		   	//handle_connection();
		   		printf("%d",strcmp(mesg,"# munin node at MyComputer\n"));

		   	//printf("%d\n",n);
		   	if (strcmp(buf,"cap\n")==0) {
		   		send(cli , "cap multigraph dirtyconfig\n" , strlen("cap multigraph dirtyconfig\n") , 0 );
				
		   	}
		   	else if (strcmp(buf,"quit\n")==0) {
		   		isQuit = 1;
				
		   	}
		   	//printf("%s",buf);
			
   		
		   	
		

		//printf("send %d bytes to client : %s\n", sent, inet_ntoa(client.sin_addr));

		


	}
	close(cli);

	return 0;
}


