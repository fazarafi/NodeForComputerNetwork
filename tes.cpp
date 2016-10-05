/**
    Handle multiple socket connections with select and fd_set on Linux
     
    Silver Moon ( m00n.silv3r@gmail.com)
*/
  
#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/sysinfo.h>
#include <fstream>
#include <iostream>
#include <sys/utsname.h>
using namespace std;
  
#define TRUE   1
#define FALSE  0
#define PORT 4949

void getMemory(long &total, long &freemem) {
    FILE *fp;
    char buf[100],buf2[100];

    fp = fopen("/proc/meminfo","r");
        fscanf(fp,"%s",buf);
        fscanf(fp,"%lu",&total);
        fscanf(fp,"%s",buf2);
        fscanf(fp,"%s",buf);
        fscanf(fp,"%lu",&freemem);
        fscanf(fp,"%s",buf2);
    fclose(fp);

    total *= 1024;
    freemem *= 1024;
}
 
int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
      
    char buffer[1025];  //data buffer of 1K
      
    //set of socket descriptors
    fd_set readfds;
    
    struct utsname hostname;
    int u = uname(&hostname);

    char list[30] = "list ";
    strcat(list,hostname.nodename);
    strcat(list,"\r\n");

    //a message
    char message[30] = "# munin node at ";
    strcat(message,hostname.nodename);
    strcat(message,"\r\n");

    //baca file memory (untuk nantinya dipakai)
    long total, freem;
    getMemory(total,freem);
    char strtot[20] = "";
    char strfree[20] = "";
    char strused[20] = "";
    sprintf(strtot,"%lu",total);
    sprintf(strfree,"%lu",freem);
    sprintf(strused,"%lu",total-freem);
  
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
      
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
  
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);
     
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
      
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    
     
    while(TRUE) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
  
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
         
        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) 
        {
            //socket descriptor
            sd = client_socket[i];
             
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);
             
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
  
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
          
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
          
            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
        
            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
            {
                perror("send");
            }
              
            puts("Welcome message sent successfully");
              
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) 
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);
                     
                    break;
                }
            }
        }
          
        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) 
        {
            sd = client_socket[i];
              
            if (FD_ISSET( sd , &readfds)) 
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                      
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                  
                //Echo back the message that came in
                else
                {
                    if(buffer[0] == 'c' && buffer[1] == 'a' && buffer[2] == 'p') {
                        //printf("cap2\n");
                        send(sd , "cap multigraph dirtyconfig\n" , strlen("cap multigraph dirtyconfig\n") , 0 );
                    }
                    else if(strcmp(buffer, "nodes\r\n") == 0)  {
                        //printf("case 2");
                        send(sd,hostname.nodename,strlen(hostname.nodename),0);
                         send(sd,"\n.\n",strlen("\n.\n"),0);
                    }
                    else if(strcmp(buffer, list) == 0)  {
                        //printf("case 3");
                         send(sd,"memory\n",8,0);
                    }
                    else if(strcmp(buffer, "config memory\r\n") == 0)  {
                        //printf("case 4");
                        long total, freem;
                        getMemory(total,freem);
                        char strtot[20] = "";
                        //char strfree[20] = "";
                        //char strused[20] = "";
                        sprintf(strtot,"%lu",total);
                        printf("%lu",total);
                        //sprintf(strfree,"%lu",freem);
                        //sprintf(strused,"%lu",total-freem);
                        
                        send(sd,"graph_args --base 1024 -l 0 --upper-limit ",strlen("graph_args --base 1024 -l 0 --upper-limit "),0);
                        send(sd,strtot,strlen(strtot),0); 
                        send(sd,"\n",strlen("\n"),0);
                        send(sd,"graph_vlabel Bytes\n",strlen("graph_vlabel Bytes\n"),0);
                        send(sd,"graph_title Memory usage\n",strlen("graph_title Memory usage\n"),0);
                        send(sd,"graph_category system\n",strlen("graph_category system\n"),0);
                        send(sd,"graph_info This graph shows this machine memory.\n",strlen("graph_info This graph shows this machine memory.\n"),0);
                        send(sd,"graph_order used free\n",strlen("graph_order used free\n"),0);
                        send(sd,"used.label used\n",strlen("used.label used\n"),0);
                        send(sd,"used.draw STACK\n",strlen("used.draw STACK\n"),0);
                        send(sd,"used.info Used memory.\n",strlen("used.info Used memory.\n"),0);
                        send(sd,"free.label free\n",strlen("free.label free\n"),0);
                        send(sd,"free.draw STACK\n",strlen("free.draw STACK\n"),0);
                        send(sd,"free.info Free memory.\n",strlen("free.info Free memory.\n"),0);
                        send(sd,".\n",strlen(".\n"),0);
                    }
                    else if(strcmp(buffer, "fetch memory\r\n") == 0)  {
                        //printf("case 5");
                        
                        long total, freem;
                        getMemory(total,freem);
                        //char strtot[20] = "";
                        char strfree[20] = "";
                        char strused[20] = "";
                        //sprintf(strtot,"%lu",total);
                        sprintf(strfree,"%lu",freem);
                        sprintf(strused,"%lu",total-freem);
                         send(sd,"used.value ",strlen("used.value "),0);
                         send(sd,strused,strlen(strused),0);
                         send(sd,"\n",strlen("\n"),0);

                         send(sd,"free.value ",strlen("free.value "),0);
                         send(sd,strfree,strlen(strfree),0);
                         send(sd,"\n",strlen("\n"),0);

                         send(sd,".\n",strlen(".\n"),0);
                    }
                    else if(strcmp(buffer, "version\r\n") == 0)  {
                        //printf("case 6");
                         send(sd,"lovely node on ",strlen("lovely node on "),0);
                         send(sd,hostname.nodename,strlen(hostname.nodename),0);
                         send(sd," version: 1.00\n",strlen(" version: 1.00\n"),0);
                    }
                    else if(strcmp(buffer, "quit\r\n") == 0 || strcmp(buffer, ".\r\n") == 0)  {
                        //printf("case 7");
                        //quit = 1;
                        close(sd);
                    }
                    else  {
                        //printf("case 8");
                         send(sd,"# Unknown command. Try cap, list, nodes, config, fetch, version or quit\n", strlen("# Unknown command. Try cap, list, nodes, config, fetch, version or quit\n"),0);   
                    }
                    //set the string terminating NULL byte on the end of the data read
                    puts(buffer);
                    buffer[valread] = '\0';
                    
                    memset(buffer,0,256);
                }
            }
        }
    }
      
    return 0;
}