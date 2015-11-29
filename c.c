/* Client side of an ftp service.  Actions:
   - connect to server and request service
   - send size-of-sile info to server
   - start receiving file from server
   - close connection
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>   
#include <string.h>

#define SERVER_PORT_ID 6081
#define CLIENT_PORT_ID 6086
#define SERVER_HOST_ADDR "127.0.0.1"

#define RESEIVEMSGSUCCESS						5   
#define RESEIVEMSGFAIL 							10                     
#define RESULTSUCCESS      				 		100
#define RESULTFAIL	            			    200
#define COMMANDNOTSUPPORTED   	                150
#define COMMANDSUPPORTED      		 			160
#define LSCOMMAND             			 		600
#define CWDCOMMAND						 		700	
#define EXITCOMMAND								1000	

int readn(int sd,char *ptr,int size);
int writen(int sd,char *ptr,int size);

  main(int argc,char *argv[])
  
{


    int sockid, newsockid,i,msg;
    struct sockaddr_in my_addr, server_addr;
    printf("client: creating socket\n");
    if ((sockid = socket(AF_INET,SOCK_STREAM,0)) < 0)
      { printf("client: socket error : %d\n", errno); exit(0);}
  
    printf("client: binding my local socket\n");
    bzero((char *) &my_addr,sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(CLIENT_PORT_ID);
    if (bind(sockid ,(struct sockaddr *) &my_addr,sizeof(my_addr)) < 0)
           {printf("client: bind  error :%d\n", errno); exit(0);
           }
                                             
    printf("client: starting connect\n");				 	
    bzero((char *) &server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_HOST_ADDR);		//IPv4 -> binary form
    server_addr.sin_port = htons(SERVER_PORT_ID);
     if (connect(sockid ,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
           {printf("client: connect  error :%d\n", errno); exit(0);}
doitagain:
     printf("client: I'm connected. Type command.\n");
  /* Once we are here, we've got a connection to the server */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  char getcom[80];
  fgets(getcom, sizeof(getcom), stdin);
		if(strcmp(getcom, "ls\n")==0 || strcmp(getcom, "cwd\n")==0)
		{
			  if(strcmp(getcom, "ls\n")==0)
			  {
					/* tell server that we want to get list of directories*/
					int dols = htons(LSCOMMAND);
					printf("client: sending command request to ftp server\n");
					if((writen(sockid,(char *)&dols,sizeof(dols))) < 0)
					   {printf("client: write  error :%d\n", errno); exit(0);} 
			  }
		   
			  if(strcmp(getcom, "cwd\n")==0)
			  {
					/* tell server that we want to get current directory */
					int docwd = htons(CWDCOMMAND);
					printf("client: sending command request to ftp server\n");
					if((writen(sockid,(char *)&docwd,sizeof(docwd))) < 0)
							{printf("client: write  error :%d\n", errno); exit(0);} 
			  }
			
				/* want for go-ahead from server */
				msg = 0;  
				if((readn(sockid,(char *)&msg,sizeof(msg)))< 0)
						{printf("client: read  error :%d\n", errno); exit(0); }
				msg = ntohs(msg);   
				if (msg==COMMANDNOTSUPPORTED) {
						printf("client: server refused command. goodbye\n"); exit(0);}
				 else
						printf("client: server replied %d, command supported\n",msg);
				 
				/* reseive and print result */
				 while(1==1)
				 {			 	 
					 char result[80];
					 while(readn(sockid, result, sizeof(result))<0)
						{ }
							
					msg = RESEIVEMSGSUCCESS;
					if((writen(sockid,(char *)&msg,sizeof(msg))) < 0)
									{printf("client: write  error :%d\n", errno);} 
					if(strcmp(result, "end") ==0)
						{printf("\n"); break;}
					 printf("%s		", result);
				 }
				 
				 msg = RESULTSUCCESS;
				 if((writen(sockid,(char *)&msg,sizeof(msg))) < 0)
						{printf("client: write  error :%d\n", errno); exit(0);} 
		}
		  
		 else  if(strcmp(getcom, "exit")==0)
		  {
			/* tell server that we want to get current directory */
			int doexit = htons(EXITCOMMAND);
			printf("client: sending command request to ftp server\n");
			if((writen(sockid,(char *)&doexit,sizeof(doexit))) < 0)
					{printf("client: write  error :%d\n", errno); exit(0);} 
		  }
			  
	  else printf("not supported command");
	goto doitagain;
	 
    close(sockid);
}	     
  
  
/* DUE TO THE FACT THAT BUFFER LIMITS IN KERNEL FOR THE SOCKET MAY BE 
   REACHED, IT IS POSSIBLE THAT READ AND WRITE MAY RETURN A POSITIVE VALUE
   LESS THAN THE NUMBER REQUESTED. HENCE WE CALL THE TWO PROCEDURES
   BELOW TO TAKE CARE OF SUCH EXIGENCIES */

int readn(int sd,char *ptr,int size)

{         int no_left,no_read;
          no_left = size;
          while (no_left > 0) 
                     { no_read = read(sd,ptr,no_left);
                       if(no_read <0)  return(no_read);
                       if (no_read == 0) break;
                       no_left -= no_read;
                       ptr += no_read;
                     }
          return(size - no_left);
}


int writen(int sd,char *ptr,int size)
{         int no_left,no_written;
          no_left = size;
          while (no_left > 0) 
                     { no_written = write(sd,ptr,no_left);
                       if(no_written <=0)  return(no_written);
                       no_left -= no_written;
                       ptr += no_written;
                     }
          return(size - no_left);
}