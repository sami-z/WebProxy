//Name: Sami Zeremariam
//UCID: 30078936
/*Description: This program serves as a web proxy that determines whether
* requests from a web client should be sent to a web server. If the request is sent
* to the web server, the the response from the web server is then sent back to the web browser
* client. Otherwise, the request is intercepted by the proxy, and the proxy sends a message
* to the web browser client redirecting the client to the an error page. The proxy determines
* what requests to intercept based on keywords in the url, which the user can pick.
*/

//LOCAL IP ADDRESS = 127.0.0.1
//FLIP THE NAMES OF CLIENT/SERVER!!!

#include<stdio.h>
#include<string.h>	
#include<stdlib.h>
#include<pthread.h>
#include<sys/socket.h>
#include<stdbool.h>
#include<arpa/inet.h>	
#include<unistd.h>	
#include<netdb.h>	

//Global variables
int numBadWords = 0;
int counter = 0;

/*Thread function for accepting connections from telnet
* and updating the array of blocked (bad) words.
*/
void *serverThread(void *vargp){
	//Initialize variables for use in thread and convert passed in argument back to a char
	char (* badWordArr) [500] = vargp;
	char threadMsg[5000];
	bool isFound = false;
	struct sockaddr_in serverThreadStruct;
	int serverThread, threadRecv, threadSock;
		
	//Create a socket	
	serverThread = socket(AF_INET, SOCK_STREAM, 0);
	if (serverThread == -1){
		printf("Could not create serverThread\n");
		exit(1);
	}
	puts("Socket created\n");

	//Create a socket
	threadSock = socket(AF_INET, SOCK_STREAM, 0);
	if (threadSock == -1){
		printf("Could not create threadSock\n");
		exit(1);
	}
	puts("thread Socket created\n");

	//Address initialization
	serverThreadStruct.sin_family = AF_INET;
	serverThreadStruct.sin_addr.s_addr = INADDR_ANY;
	serverThreadStruct.sin_port = htons(8359);
  		
	//Bind server socket
	int serverBind = bind(serverThread, (struct sockaddr *)&serverThreadStruct, sizeof(serverThreadStruct));
	if (serverBind == -1){
		perror("serverBind binding failed");
		exit(1);
	}

	//Listen on socket for connections
	listen(serverThread, 3);

	//Infinite while loop to keep accepting connections and adding/unadding
	//blocked words
	while (1){
		//Accept a connection
		threadSock = accept(serverThread, NULL, NULL);
		if (threadSock < 0){
			perror("threadSock connection failed");
			exit(1);
		}
		
		printf("Connection Success\n");
		memset(threadMsg, 5, 5000);

		//While loop for until specfic end phrase is typed on telnet "END"
		memset(threadMsg, 0, 5000);
		while (strncmp(threadMsg, "END", 3) != 0 ){
			isFound = false;
			memset(threadMsg, 0, 5000);
			threadRecv = recv(threadSock, threadMsg, 5000, 0);
			if (threadRecv == -1){
				printf("threadRecv could not receive");
				exit(1);
			}

			//Check if user want to unblock a word
			if (strncmp(threadMsg, "UB", 2) == 0){
				char *temp2 = strtok(threadMsg, " ");
				temp2 = strtok(NULL, "\r");

				//Loop through entire array for word to unblock
				//Memset the index to 1
				for (int q = 0; q < 3; q++){
					if (strncmp(temp2,  badWordArr[q], strlen(temp2)) == 0){
						memset(badWordArr[q], 1, 500);
						isFound = true;
						counter--;		
					}
				}

				//Check if blocked word is not in array
				if (isFound == false){
					printf("Blocked word not in list!\n");
				}
				continue;
			}

			//Check if there are already 3 blocked words
			if (counter == 3){
				printf("Cannot add more words to block!\n");
			}

			//Add the word to the array as a blocked word
			else{
				char *temp = strtok(threadMsg, "\r\n");
				strcpy(badWordArr[counter], threadMsg);
				printf("Blocked word is: %s\n", badWordArr[counter]);
				counter++;
			}
		}

		printf("END success\n");
		close(threadSock);
	}
}


int main(int argc, char *argv[]){
	//Initialize variables
	int socket_desc, client_sock, server_sock, badRecvStatus;;
	struct sockaddr_in server, client;
	char blockedWord [5000][500];
	char blockedRedirect[5000];
	char blockedURL[5000] = "http://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/error.html";
	char client_message[5000];
	char badRecv[5000];
	char server_reply[5000];
	char server_reply_copy[5000];
	char client_message_copy[5000];
	char client_message_copy2[5000];
	char ip[100];
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	//NAME CHANGE OF THREAD
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, serverThread, (void *)blockedWord);

	//Memset array of blocked words to 1
	for(int q = 0; q < 5000; q++){
		memset(blockedWord[q], 1, 500);	
	}

	//Create socket to bind to web browser
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1){
		printf("Could not create socket_desc\n");
	}
	
	puts("Socket created\n");

	//Address initialization
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = INADDR_ANY;
	client.sin_port = htons(8282);

	//Bind server socket
	int bindStatus = bind(socket_desc, (struct sockaddr *)&client, sizeof(client));
	if( bindStatus == -1){
		perror("Binding failed");
		exit(1);
	}

	while(1){
		//Create socket
		client_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (client_sock == -1){
			printf("Could not create client_sock\n");
			exit(1);
		}

		puts("client_sock created\n");

		//Create socket
		server_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (server_sock == -1){
			printf("Could not create server_sock\n");
			exit(1);
		}

		puts("server_sock created\n");
		
		//Listen on socket for connections
		listen(socket_desc , 3);

		//Accept and incoming connection
		printf("Waiting for clients...\n");
		
		//Accept connection from an incoming client
		client_sock = accept(socket_desc, NULL, NULL);
		if (client_sock < 0){
			perror("Connection failed");
			return 1;
		}

		printf("Connection accepted\n");

		//Forking to create child process to handle requests
		int pFork = fork();
		if (pFork < 0){
			printf("Fork call failed!\n");
			exit(1);
		}

		else if (pFork > 0 ){
			close(client_sock);
		} 
		
		else{
			close(socket_desc);	
			//Receive a message from client
			int recvStatus;
			recvStatus = recv(client_sock, client_message, 5000, 0);
			
			if (recvStatus==-1){
				printf("Error in receiving!");
				return 1;
			}

			//Print out client response
			printf("Client says: %s\n", client_message);

			//Copy client response 
			strcpy( client_message_copy, client_message);
			strcpy( client_message_copy2, client_message);

			//Parse client response for the host name
			char * host = strstr(client_message,"Host: ");
			int loop = 6;
			int loop2 = 0;
			char hostName[100];
			while(host[loop] != '\r'){
				hostName[loop2] = host[loop];
				loop++;
				loop2++;
			}

			printf("Host name is %s\n", hostName);

			//Check to make sure we are only accepting requests from the right Host and only GET requests
			if (strcmp(hostName, "pages.cpsc.ucalgary.ca")!=0 || strstr(client_message_copy2, "GET") == NULL){
				printf("Intercepted request! Only accepting GET requests from pages.cpsc.ucalgary.ca\n");
				close(client_sock);
				close(socket_desc);
				close(server_sock);
				continue;
			}

			//Check if parsed host name is null, exit if true
			//CREDIT FOR LINES 271 - 284: https://www.binarytides.com/socket-programming-c-linux-tutorial/
			if ( (he = gethostbyname(hostName)) == NULL){
				herror("gethostbyname");
				return 1;
			}
			
			//Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only CREDIT THIS!!!!
			addr_list = (struct in_addr **) he->h_addr_list;
			
			for(i = 0; addr_list[i] != NULL; i++) {
				//Copy ip address
				strcpy(ip , inet_ntoa(*addr_list[i]) );
			}
			
			printf("%s resolved to : %s\n" , hostName , ip);

			//Address initialization
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = inet_addr(ip);
			server.sin_port = htons(80);

			//Socket connection request
			int connectionStatus = connect(server_sock, (struct sockaddr *)&server, sizeof(server));
			if(connectionStatus == -1) {
				puts("Error: connection failed.");
				return 1;
			}

			puts("Connection established.");
			
			//Parse for url of webpage
			char *url = strtok(client_message_copy2, " ");
			url = strtok(NULL, " ");
			printf("URL is: %s\n", url);

			//Compare if a blocked word is contained in the url of webpage
			bool findBad = false;
			for(int t = 0; t < counter; t++){ 
				if (strstr(url, blockedWord[t]) != NULL ){  
					printf("URL CONTAINS BAD WORD!\n");
					findBad = true;
				}
			}

			//Redirect to inappropriate URL error page is bad word is found in URL
			if(findBad){
				sprintf(blockedRedirect, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", blockedURL, hostName);
				printf("%s\n", blockedRedirect);

				//Send response to webserver
				int badSend = send(server_sock, blockedRedirect, strlen(blockedRedirect), 0);
				if ( badSend< 0){
					puts("Send failed");
					exit(1);
				}

				//Receive response from webserver
				badRecvStatus = recv(server_sock, badRecv, 5000, 0);
				if (badRecvStatus < 0){
					printf("Error in receiving\n");
					exit(1);
				}

				//Send response from webserver back to web browser
				int badSendClient = send(client_sock, badRecv, badRecvStatus, 0);
				if (badSendClient < 0){
					puts("Send to browser failed");
					exit(1);
				}

				//Close sockets
				close(client_sock);
				close(socket_desc);
				close(server_sock);
			
				findBad = false;
				continue;
			}

			//If bad word is not found in URL, send reuest as normal to webserver
			int serverSend1 = send(server_sock, client_message_copy, strlen(client_message_copy), 0);
			if (serverSend1 < 0){
				puts("Send failed");
				exit(1);
			}

			//Receive reply from web server 
			int countBytes = recv(server_sock, server_reply, 5000, 0);
			if (countBytes < 0){
				puts("Could not receive response from web server");
				return 1;
			}

			//Check is webserver sent bad a 304 cached response and the hostname is correct
			//Ignore if true
			if (strstr(server_reply, "304") != NULL && strcmp(hostName, "pages.cpsc.ucalgary.ca") == 0){
				if(send(client_sock, server_reply , strlen(server_reply) , 0) < 0){
					puts("Send failed");
					return 1;
				}
				
				//Close sockets
				close(client_sock);
				close(socket_desc);
				close(server_sock);
				continue;
			}


			//Send response back to web browser
			int clientSend1 = send(client_sock, server_reply, countBytes, 0);
			if (clientSend1 < 0){
				puts("Send failed");
				exit(1);
			}	

			//printf("Server reply: %s\n", server_reply);
			strcpy(server_reply_copy, server_reply);
			printf("This is the server reply: %s\n", server_reply);

			char content[5000];
			char *conLength;
			strcpy(content, server_reply);

			//Get content length from server reply
			char * contentLength = strstr(content, "Content-Length: ");
			conLength = strtok(contentLength, " ");
			conLength = strtok(NULL, " ");
			conLength = strtok(conLength, "\n");
			conLength = strtok(conLength, "\r");

			//Convert content length to int
			int getContentLength = atoi(conLength);
			printf("Content length as an int is: %d\n", getContentLength);

			//get header length from server reply
			char *headerLength = strstr(server_reply_copy, "\r\n\r\n");

			int hLength = headerLength - server_reply_copy;
			printf("header length is: %d\n", hLength);

			int tLength = getContentLength + hLength;
			printf("total length is: %d\n", tLength);

			int loopRecvStatus;
			int loopSendStatus;

			//While loop to keep receving bytes until we receive the total amount of bytes from the web page
			while (countBytes < tLength ){
				char loopRecv[5000];

				//Receive response from the web server
				loopRecvStatus = recv(server_sock, loopRecv, 5000, 0);
				if (loopRecvStatus < 0 ){
					printf("Error in receiving!");
					return 1;
				}

				//Send response from webserver back to web browser
				loopSendStatus = send(client_sock, loopRecv, loopRecvStatus, 0);
				if ( loopSendStatus< 0){
					puts("Send failed");
					return 1;
				} 

				countBytes = countBytes + loopRecvStatus;
			}

			//Close all sockets and end child process
			close(client_sock);
			close(socket_desc);
			close(server_sock);
			exit(0);
		}
	}
	return 0;
}