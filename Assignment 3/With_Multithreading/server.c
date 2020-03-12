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
#include <unistd.h> 
#include <pthread.h> 

#define TRUE 1 
#define FALSE 0 
int master_socket;
int addrlen;
int port;
int new_socket;

const int MAX = 100;
// Utility function for converting int to string
char* itoa(int value, char* result, int base) 
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
// ctrl + C Handler
void Handler(int sig) {
	
	printf("Server exiting due to MANUAL BREAK\n");
	close(master_socket);
	exit(0);
}
// Function that executes everytime new client is added
void * socketThread(void *arg)
{

  int new_socket = *((int *)arg);
  char buffer[1025] = "";
  int valread;
  int total_bill = 0;
  while(TRUE)
  {
	  memset(buffer,'\0',sizeof(buffer));
	  if ((valread = read(new_socket, buffer, 1024)) > 0)
	  { 
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

	  				// Calculating bill for client[i]
	  				total_bill+= atoi(quantity)*atoi(item_price);
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
	  		// convert bill to string
	  		char bill_string[20] = "";
	  		itoa(total_bill,bill_string,10);

	  		char response[200] = "0/Total Bill = ";
	  		strcat(response,bill_string);
	  		write(new_socket,response,sizeof(response));

	  	}
	  } 
  }
  pthread_exit(NULL);

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

	signal(SIGINT, Handler);
	int max_clients = 20; 
	struct sockaddr_in address; 
		
	char buffer[1025]; //data buffer of 1K 
		
	

	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	}
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(port);  

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
	addrlen = sizeof(address); 
	puts("Waiting for connections ..."); 

	pthread_t tid[60];
	int no_clients = 0;
	while(TRUE)
	{
		if ((new_socket = accept(master_socket, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
		else 
			no_clients++;
		//inform user of socket number - used in send and receive commands 
		printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
		
		
		if( pthread_create(&tid[no_clients], NULL, socketThread, &new_socket) != 0 )
			printf("Failed to create thread\n");

		// if max clients are connected
		//  then wait for each client to disconnect
		if( no_clients>= max_clients)
		{
		  no_clients = 0;
		  while(no_clients < max_clients)
		  {
		  	pthread_join(tid[no_clients++],NULL);
		  }
		  no_clients = 0;

		}
		

	}

}
	
