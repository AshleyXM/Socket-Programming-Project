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
#define MAXSLOTLEN 100

/*
   func: convert the time slots into a int array
   recvdata: [[1,3],[5,7],[10,20]]
   return value: 011101110011111111111000...
*/
int *convertTime(char *recvdata);
// check the validity of the input time slot
// if valid, return 1; otherwise, return 0
int checkValidity(char *available, char *selected);

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
		   printf("Client received the reply from Main Server using TCP over port %s:\nTime intervals %s works for %s.\n", port, group[2], group[1]);
 
      // if the available intersection is not empty
      if(strcmp(group[2],"[]") != 0) {
         printf("Please enter the final meeting time to register an meeting:\n");

         char selectedslot[MAXBUFLEN];
         while(1) { // read until the input is valid
            fgets(selectedslot, sizeof(selectedslot), stdin);
            selectedslot[strlen(selectedslot)-1] = '\0'; // remove the \n at last
            // if the input interval is invalid, then ask the user to re-enter
            if(checkValidity(group[2], selectedslot) == 0) // group[2] is the available intersection
               printf("Time interval %s is not valid. Please enter again:\n", selectedslot);
            else
               break;
         }
         send(sockfd, selectedslot, MAXBUFLEN-1, 0);
         char status[10];
         recv(sockfd, status, 9, 0);
         if(strcmp(status, "success") == 0)
		      printf("All servers updated successfully.\n");
      }

		memset(namestr, MAXBUFLEN, 0); // clear namestr and get ready for the next request

		printf("-----Start a new request-----\n");
		printf("Please enter the usernames to check schedule availability:\n");

		close(sockfd);
	}

	return 0;
}

/*
   func: convert the time slots into a int array
   recvdata: [[1,3],[5,7],[10,20]]
   return value: 011101110011111111111000...
*/
int *convertTime(char *recvdata) {
   int *converted = malloc(sizeof(int) * MAXSLOTLEN); // the converted data, which is the final result
   memset(converted, 0, sizeof(converted));
   int slots[50];
   int position = 0; // the number of numbers in recvdata
   char recvdata_copy[MAXBUFLEN];
   strcpy(recvdata_copy, recvdata);
   char *token = strtok(recvdata_copy, ",[]");
   while(token != NULL) {
      slots[position] = atoi(token);
      position++;
      token = strtok(NULL, ",[]");
   }

   /*
   // print the array slots
   printf("slots in func convertTime: ");
   for(int i = 0; i < position; i++)
      printf("%d ", slots[i]);
   printf("\n");
   */
   for(int i = 0; i < position; i += 2) {
      for(int j = slots[i]; j <= slots[i+1]-1; j++) {
         converted[j] = 1;
      }
   }

   /*
   // print the array time
   printf("converted in func convertTime: ");
   for(int i = 0; i < MAXSLOTLEN; i++)
      printf("%d ", converted[i]);
   printf("\n");
   */
   
   return converted;
}

// check the validity of the input time slot
// if valid, return 1; otherwise, return 0
// selected form: [a,b]
int checkValidity(char *available, char *selected) {
   int *avlblslots = convertTime(available);
   /*
   printf("avlblslots is ");
   for(int i = 0; i < MAXSLOTLEN; i++)
      printf("%d ", avlblslots[i]);
   printf("\n");
   */

   char selected_copy[20];
   strcpy(selected_copy, selected);
   char *token = strtok(selected_copy, ",[]");
   int slots[2]; // store the left and right index of the interval
   int position = 0; // the position in array slots
   while(token != NULL) {
      slots[position] = atoi(token);
      position++;
      token = strtok(NULL, ",[]");
   }
   int left = slots[0];
   int right = slots[1];
   for(int i = left; i < right; i++) {
      if(avlblslots[i] == 0)
         return 0;
   }
   return 1;
}
