#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// required header files
#include <errno.h>
#include <sys/wait.h>

#define LOCALHOST "127.0.0.1"
#define SERVERPORT "24015"
#define MAXBUFLEN 250

int main(){
	printf("Client is up and running.\n");
	printf("Please enter the usernames to check schedule availability:\n");
	char namestr[MAXBUFLEN];
	int sockfd;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(LOCALHOST, SERVERPORT, &hints, &res);

	while(fgets(namestr, sizeof(namestr), stdin)){ // keep receiving the query request from the client
	   sockfd = socket(AF_INET, SOCK_STREAM, 0); // get fd
      // establish connection
      connect(sockfd, res->ai_addr, res->ai_addrlen);
      // send the input usernames to the main server
		send(sockfd, namestr, MAXBUFLEN-1, 0);
		char port[10];
		recv(sockfd, port, 9, 0); // receive the port number that the client is using to communicate with serverM

		printf("Client finished sending the usernames to Main Server.\n");

		char recvstr[MAXBUFLEN] = {0}; // received string from the main server
		recv(sockfd, recvstr, MAXBUFLEN-1, 0);
		// printf("recvstr is %s\n", recvstr);

		// deal with the info sent from the main server
		/* names that do not exist
		   names that exist
		   time intervals that fit for all existing users
		 */
		int count = 0; // the seq of group
		char group[3][MAXBUFLEN]; // store the above info
		memset(group, 0, sizeof(char) * 3 * MAXBUFLEN);

		int i = 0; // used to traverse the array recvstr
		int position = 0; // label the position in each group
		while(recvstr[i] != '\0') {
	      if(recvstr[i] == ';') {
	         group[count][position] = '\0';
	         position = 0;
	         count++;
	      } else {
	         group[count][position] = recvstr[i];
	         position++;
	      }
	      i++;
		}

      /*
		for(int i = 0; i < 3; i++){
		   printf("group[%d] is %s\n", i, group[i]);
		}
      */

		if(strlen(group[0]) != 0)
		   printf("Client received the reply from Main Server using TCP over port %s:\n%s do not exist.\n", port, group[0]);

		if(strlen(group[1]) != 0)
		   printf("Client received the reply from Main Server using TCP over port %s:\nTime intervals %s works for %s.\n", port, group[1], group[2]);

		memset(namestr, MAXBUFLEN, 0); // clear namestr and get ready for the next request

		printf("-----Start a new request-----\n");
		printf("Please enter the usernames to check schedule availability:\n");

		close(sockfd);
	}

	return 0;
}

