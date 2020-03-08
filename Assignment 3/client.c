// Client side C/C++ program to demonstrate Socket programming 
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
const int ITEM = 0;
const int CLOSE = 1;
//The below function converts any char *
//into a string
//Required to convert port number 
//from string to integer
int sockfd , connfd;

//There is some error there when 
//you delcare buffer as a global variable
//That's why buffer has been declared as 
// a local variable inside the main function
const int N = 1000;
void Handler(int sig) {
	printf("\nClient exiting because of manual break...\n");
	char buf[] = "Termination of Client\n";
	write(sockfd , buf , strlen(buf));
	close(sockfd);
	exit(0);
	return;
}

int main(int argc, char *argv[]) 
{ 
	char * ip = argv[1];
	char * port_num = argv[2];
	int port = atoi(port_num);
	signal(SIGINT, Handler);
	struct sockaddr_in serv_addr,client; 
	 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1;
	} 
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET,ip, &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
	else
	{
		printf("Connected to Server\n");
	}
	char buffer[1024] = "";
	int buffer_len = read(sockfd, buffer,sizeof(buffer));
	printf("%s\n\n",buffer);

	FILE *filePointer;
	filePointer = fopen("inventory.txt", "r");
	char item_details[100];
	while(fgets(item_details,sizeof(item_details),filePointer))
	{
		char item_upc[10] = "";
		char item_name[50] = "";
		char item_price[50] = "";

		int ind = 0;
		int j = 0;
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
		printf("%s  %s  %s\n",item_upc,item_name,item_price);
	}
	fclose(filePointer);
	printf("\nRequest Type = 0 for adding items to cart\n");
	printf("Request Type = 1 for closing connection with server and checking out\n");
	while(1)
	{
		int request_type;
		printf("Enter request type: ");
		scanf("%d",&request_type);
		printf("\n");
		


		if(request_type==0)
		{
			char upc[10];
			printf("Enter UPC code: ");
			scanf("%s",upc);
			printf("\n\n");
			char quantity[10];
			printf("Enter Quantity: ");
			scanf("%s",quantity);
			printf("\n\n");

			
			char request_msg[150] = "";
			request_msg[0] = '0';
			request_msg[1] = '/';
			int ind = 2;
			for(int i=0;i<strlen(upc);i++)
				request_msg[ind++] = upc[i];

			request_msg[ind++] = '/';

			for(int i=0;i<strlen(quantity);i++)
				request_msg[ind++] = quantity[i];

			request_msg[ind++] = '\0';
			write(sockfd,request_msg,sizeof(request_msg));
			memset(buffer,'\0',sizeof(buffer));
			buffer_len = read(sockfd, buffer,sizeof(buffer));
			printf("Echo from server\n%s\n\n",buffer);
		}
		else
		{
			char request_msg[150] = "";
			request_msg[0] = '1';
			request_msg[1] = '\0';
			write(sockfd,request_msg,sizeof(request_msg));
			memset(buffer,'\0',sizeof(buffer));
			buffer_len = read(sockfd, buffer,sizeof(buffer));
			printf("Echo from server\n%s\n\n",buffer);

			close(sockfd);
			printf("Closed from client side\n");
			break;
		}
	}
	return 0; 
} 
