/* This is the server for a very simple file transfer
   service.  This is a "concurrent server" that can
   handle requests from multiple simultaneous clients.
   For each client:
    - get file name and check if it exists
    - send size of file to client
    - send file to client, a block at a time
    - close connection with client
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
#include <unistd.h>
#include <dirent.h>

#define MY_PORT_ID  							6081  

#define RESEIVEMSGSUCCESS						5   
#define RESEIVEMSGFAIL 							10
#define RESULTSUCCESS      				 		100
#define RESULTFAIL	            			 	200
#define COMMANDNOTSUPPORTED   	 				150
#define COMMANDSUPPORTED      		 			160
#define LSCOMMAND             				    600
#define CWDCOMMAND						 		700	
#define EXITCOMMAND						 		1000

int writen(int sd,char *ptr,int size);
int readn(int sd,char *ptr,int size);

main()  
	{

	   int sockid, newsd, pid, clilen;
	   struct sockaddr_in my_addr, client_addr;   

	   printf("server: creating socket\n");
	   if ((sockid = socket(AF_INET,SOCK_STREAM,0)) < 0)	//creating socket domain=Internet protocols, type=stream, protocol=TCP
	     	{printf("server: socket error : %d\n", errno); exit(0); }

	   printf("server: binding my local socket\n");
	   bzero((char *) &my_addr,sizeof(my_addr));
	   my_addr.sin_family = AF_INET;
	   my_addr.sin_port = htons(MY_PORT_ID);
	   my_addr.sin_addr.s_addr = htons(INADDR_ANY);		//host to network short
	   if (bind(sockid ,(struct sockaddr *) &my_addr,sizeof(my_addr)) < 0)		//addr to socket
	    	 {printf("server: bind  error :%d\n", errno); exit(0); }
	   printf("server: starting listen \n");
	   if (listen(sockid,5) < 0)		//backlog = max length of the queue of pending connections
	     	{ printf("server: listen error :%d\n",errno);exit(0);}                                        

	   while(1==1) 
	   { 
		     /* ACCEPT A CONNECTION AND THEN CREATE A CHILD TO DO THE WORK */
		     /* LOOP BACK AND WAIT FOR ANOTHER CONNECTION                  */
		     printf("server: starting accept\n");
		     if ((newsd = accept(sockid ,(struct sockaddr *) &client_addr, &clilen)) < 0)
		        	{printf("server: accept  error :%d\n", errno); exit(0); }
	         printf("server: return from accept, socket for this ftp: %d\n", newsd);
		     if ( (pid=fork()) == 0) 
		     {
		         /* CHILD PROC STARTS HERE. IT WILL DO ACTUAL FILE TRANSFER */
		         close(sockid);   /* child shouldn't do an accept */
		         doftp(newsd);
		         close (newsd);
		         exit(0);         /* child all done with work */
	         }
		      /* PARENT CONTINUES BELOW HERE */
	     	 close(newsd);        /* parent all done with client, only child */
	    }              /* will communicate with that client from now on */
	}

/* CHILD PROCEDURE, WHICH ACTUALLY DOES THE FILE TRANSFER */
doftp(int newsd)
  {       
	    int i,req, msg_ok;
	      
	    
	     /* START SERVICING THE CLIENT */ 
	  
	     /* get command code from client.*/
	     /* 600 -  ls */
		 /* 700 - cwd */
		 doitagain:
	     req = 0;
	     while((readn(newsd,(char *)&req,sizeof(req))) < 0)
			{}
	     req = ntohs(req);
	     printf("server: client request code is: %d\n",req);
	    if(req != EXITCOMMAND)
		{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if(req == LSCOMMAND || req == CWDCOMMAND)
			{
				 msg_ok = COMMANDSUPPORTED;
						  msg_ok = htons(msg_ok);
						  if((writen(newsd,(char *)&msg_ok,sizeof(msg_ok))) < 0)
								{printf("server: write error :%d\n",errno);exit(0);}
							
					 if(req == LSCOMMAND)
					 {					 
							  DIR *dir = opendir(".");
							  if(dir)
							  {
								  struct dirent *ent;
								  while((ent = readdir(dir)) != NULL)
								  {
										if ((writen(newsd, ent->d_name, strlen(ent->d_name)))<0) 
											{printf("server: sending msg error :%d\n", errno); exit(0);}
										printf("server:msg being sent: %s\n",ent->d_name);
										
										while((readn(newsd,(char *)&msg_ok,sizeof(msg_ok))) < 0)
												{}
											if(msg_ok == RESEIVEMSGFAIL)
												{printf("server: client get msg error :%d\n", errno); exit(0);}

								  }
								  if ((writen(newsd,"end", strlen("end")))<0) 
											{printf("server: sending last msg error :%d\n", errno); exit(0);}
							  }
							  else
							  {printf("server : opening directory error :%d\n", errno); exit(0);}
						  
						 
					 }
					 
					if(req == CWDCOMMAND)
					{
							char cwd[255];
							if (getcwd(cwd, sizeof(cwd)) == NULL)
								{printf("server: get current directory error :%d\n", errno); exit(0);}				
							else{
								 if ((writen(newsd,(char*)&cwd, strlen(cwd)))<0) 
										{printf("server: write error :%d\n", errno); exit(0);}
							   printf("server: msg was send: %s\n", cwd);
							}
							if ((writen(newsd,"end", strlen("end")))<0) 
											{printf("server: sending last msg error :%d\n", errno); exit(0);}
					}
					
					//get information about result
					if (readn(newsd, (char*)&msg_ok, sizeof(msg_ok)))
						{printf("server: read result msg error :%d\n", errno); exit(0);}
					if(msg_ok == RESULTFAIL)
						{printf("server: it's a pity, that u've miss my perfect result");}
			}
			 //if not supported command
			 else
			 {
				   printf("server: unsupported operation. goodbye\n");
				 /* reply to client: command not OK  (code: 150) */
				 msg_ok = COMMANDNOTSUPPORTED; 
				 msg_ok = htons(msg_ok);
				 if((writen(newsd,(char *)&msg_ok,sizeof(msg_ok))) < 0)
					{printf("server: write error :%d\n",errno);exit(0);}
				 exit(0);
			  }
	          goto doitagain;
		}					 
	   close(newsd);
    }



/*
  TO TAKE CARE OF THE POSSIBILITY OF BUFFER LIMMITS IN THE KERNEL FOR THE
 SOCKET BEING REACHED (WHICH MAY CAUSE READ OR WRITE TO RETURN FEWER CHARACTERS
  THAN REQUESTED), WE USE THE FOLLOWING TWO FUNCTIONS */  
   
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


           