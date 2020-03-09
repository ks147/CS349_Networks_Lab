//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h> //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <signal.h>
#include<stdbool.h>

#define TRUE 1 
#define FALSE 0 
int client_socket[100];
int bill[100];
int opt = 1;
int master_socket;
int addrlen;
int port;
int i;
int new_socket;
const int MAX = 100;
void Handler(int sig) {
	
	int i;
	for(i = 0 ; i < MAX ; i++) {
		if(client_socket[i] != 0) {
			printf("\nSocket number - %d is being closed.\n" , client_socket[i]);
			close(client_socket[i]);
			client_socket[i] = 0;
			bill[i] = 0;
		}
	}
	close(master_socket);
}
	
int main(int argc , char *argv[]) 
{ 
	char * port_num = argv[1];
	int port = atoi(port_num);
	
	if(argc!=2)
	{
		printf("Enter only 2 arguments in command line\n");
		exit(0);
	}

	//signal(SIGINT, Handler);
	int opt = TRUE; 
	int master_socket , addrlen , new_socket , 
		max_clients = 20 , activity, i , valread , sd; 
	int max_sd; 
	struct sockaddr_in address; 
		
	char buffer[1025]; //data buffer of 1K 
		
	//set of socket descriptors 
	fd_set readfds; 
		
	//initialise all client_socket[] to 0 so not checked 
	memset(client_socket,0,sizeof(client_socket));

	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	//this is just a good habit, it will work without this 
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
		sizeof(opt)) < 0 ) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(port); 
		
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Listener on port %d \n", port); 
		
	//try to specify maximum of max_clients pending connections for the master socket 
	if (listen(master_socket, max_clients) < 0) 
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
	
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
	
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
			
		//If something happened on the master socket , 
		//then its an incoming connection 
		if (FD_ISSET(master_socket, &readfds)) 
		{ 
			if ((new_socket = accept(master_socket, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
		
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
			read(new_socket,buffer,sizeof(buffer));
			char type = buffer[0];
			char upc[30] = "";
			char quantity[30] = "";
			
			if(type == '0')
			{
				bool upc_found = FALSE;

				int ind = 2;
				int j = 0;
				for(int i=ind;i<strlen(buffer);i++)
				{
					ind++;
					if(buffer[i]!='/')
						upc[j++] = buffer[i];
					else
						break;
				}
				j = 0;
				for(int i=ind;i<strlen(buffer);i++)
				{
					ind++;
					quantity[j++] = buffer[i];
				}

				FILE *filePointer;
				filePointer = fopen("inventory.txt", "r");
				char item_details[100];
				while(fgets(item_details,sizeof(item_details),filePointer))
				{
					char item_upc[10] = "";
					char item_name[50] = "";
					char item_price[50] = "";

					ind = 0;
					j = 0;
					for(int i = ind;i<strlen(item_details);i++)
					{
						ind++;
						if(item_details[i]!='/')
							item_upc[j++] = item_details[i];
						else
							break;
					}
					j = 0;
					for(int i = ind;i<strlen(item_details);i++)
					{
						ind++;
						if(item_details[i]!='/')
							item_name[j++] = item_details[i];
						else
							break;
					}
					j = 0;
					for(int i = ind;i<strlen(item_details);i++)
					{
						item_price[j++] = item_details[i];
					}
					
					if(strcmp(upc,item_upc)==0)
					{
						upc_found = TRUE;
						char response[200] = "";
						response[0] = '0';
						response[1] = '/';

						strcat(response,item_price);
						strcat(response,"/");
						strcat(response,item_name);
						write(new_socket,response,sizeof(response));

						break;
					}
				}
				fclose(filePointer);
				if(!upc_found)
				{
					char response[200] = "1/UPC not found in database";
					write(new_socket,response,sizeof(response));
				}
			}
			else if(type=='1')
			{
				char response[200] = "0/Total Bill = 0";
				write(new_socket,response,sizeof(response));	
			}
		} 
			
		//else its some IO operation on some other socket 
		for (i = 0; i < max_clients; i++) 
		{ 
			sd = client_socket[i]; 
				
			if (FD_ISSET( sd , &readfds)) 
			{ 
				//Check if it was for closing , and also read the 
				//incoming message 
				if ((valread = read( sd , buffer, 1024)) == 0) 
				{ 
					//Somebody disconnected , get his details and print 
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen); 
					printf("Host disconnected , ip %s , port %d \n" , 
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
						
					//Close the socket and mark as 0 in list for reuse 
					close(sd); 
					client_socket[i] = 0; 
				} 
					
				//Echo back the message that came in 
				else
				{ 
					//set the string terminating NULL byte on the end 
					//of the data read 
					buffer[valread] = '\0'; 
					send(sd , buffer , strlen(buffer) , 0 ); 
				} 
			} 
		} 
	} 
		
	return 0; 
} 
