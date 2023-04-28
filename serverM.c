#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
// required header files
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#define SERVERAPORT "21015"
#define SERVERBPORT "22015"
#define UDPPORT 23015
#define TCPPORT 24015
#define LOCALHOST "127.0.0.1"
#define BACKLOG 3 // how many pending connections queue will hold
#define MAXBUFLEN 800
#define MAXSLOTLEN 100

char usersA[200][25], usersB[200][25]; // will not change in a new request
int sizeA = 0; // the number of users in serverA, will not change in a new request
int sizeB = 0; // the number of users in serverB, will not change in a new request

/* judge whether the name is in serverA or serverB */
int nameInServer(char c, char* name);
/*
   func: convert the time slots received from Server A/B into an int array
   recvdata: [[1,3],[5,7],[10,20]]
   return value: 011101110011111111111000...
*/
int *convertTime(char *recvdata);
/*
   parameter time is the final time slots, like 001033330201...(length equals MAXSLOTLEN)
   parameter src_num is the number of users used to calculate the intersection
   return value: [[4,7],..]
*/
char *displayIntersection(int time[MAXSLOTLEN], int src_num);

int main(){
	struct addrinfo hints, *resA, *resB; // resA and resB can be used for sending msg
	int udpsockfd;
	int tcpsockfd;
	int childsockfd; // child socket

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	// get ready
	getaddrinfo(LOCALHOST, SERVERAPORT, &hints, &resA); // get info of serverA
   getaddrinfo(LOCALHOST, SERVERBPORT, &hints, &resB); // get info of serverA

	char recvstr[MAXBUFLEN]; // used to receive the info
	/* sockaddr: used by OS, programmers should only use sockaddr_in */
	/* sockaddr_in: ipv4, sockaddr_in6: ipv6, sockaddr_storage: protocol-independent */
	struct sockaddr_in udpsock; // the address of sender

	udpsockfd = socket(AF_INET, SOCK_DGRAM, 0); // get udp sockfd

	bzero(&udpsock, sizeof(udpsock)); // initiate mysock
	udpsock.sin_family = AF_INET; // set addr family
	udpsock.sin_port = htons(UDPPORT); // set port
	udpsock.sin_addr.s_addr = inet_addr(LOCALHOST); // set ip address

	bind(udpsockfd, (struct sockaddr *)&udpsock, sizeof(struct sockaddr)); // bind udpport to udp sockfd

	struct sockaddr_in tcpsock; // the address of sender

	tcpsockfd = socket(AF_INET, SOCK_STREAM, 0); // get tcp sockfd

	bzero(&tcpsock, sizeof(tcpsock)); // initiate mysock
	tcpsock.sin_family = AF_INET; // set addr family
	tcpsock.sin_port = htons(TCPPORT); // set port
	tcpsock.sin_addr.s_addr = inet_addr(LOCALHOST); // set ip address

	bind(tcpsockfd, (struct sockaddr *)&tcpsock, sizeof(struct sockaddr)); // bind tcpport to tcp sockfd
	listen(tcpsockfd, BACKLOG);

	/*
	struct sockaddr_in{
		sa_family_t      sin_family;
		uint16_t         sin_port;
		struct in_addr   sin_addr;
		char             sin_zero[8];
	}
	*/
	struct sockaddr_in src_addr;
	socklen_t addr_len = sizeof(src_addr);

	for(int i = 0; i < 2; i++){
		memset(recvstr, 0, sizeof(recvstr)); // clear recvstr
		recvfrom(udpsockfd, recvstr, MAXBUFLEN-1, 0,
			(struct sockaddr *)&src_addr, &addr_len); // receive msg via udpsockfd
		// printf("recvstr is %s\n",recvstr);
		//printf("src_addr is %s, port is %d\n", inet_ntoa(src_addr.sin_addr),
      //      ntohs(src_addr.sin_port));

		// parse recvdata
		int count = 0;
		char *token = strtok(recvstr, " "); // split the content by whitespace
		while(token != NULL) {
		// printf("current token is %s\n", token);
			if(ntohs(src_addr.sin_port) == atoi(SERVERAPORT)) { // receiced from serverA
				strcpy(usersA[sizeA], token);
				sizeA++;
			} else { // ntohs(src_addr.sin_port) == 22015, received from serverB
				strcpy(usersB[sizeB], token);
				sizeB++;
			}
			token = strtok(NULL, " ");
		}
		if(ntohs(src_addr.sin_port) == atoi(SERVERAPORT))
		   printf("Main Server received the username list from server A using UDP over port %d.\n", UDPPORT);
		else
		   printf("Main Server received the username list from server B using UDP over port %d.\n", UDPPORT);
	}

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	

	while(1) {
	   // TCP connection can only be used one time, which is quite different from UDP
	   childsockfd = accept(tcpsockfd, (struct sockaddr *)&client_addr, &client_addr_len);
		char clientstr[MAXBUFLEN];
		// printf("client port is %d\n", ntohs(client_addr.sin_port));
		// lsof -i | grep 38582  -> check the process of specified port number
		char clientport[10];
		sprintf(clientport, "%d", ntohs(client_addr.sin_port));
		send(childsockfd, clientport, 9, 0); // send the client port info to the client
		recv(childsockfd, clientstr, MAXBUFLEN-1, 0); // receive names info from the client
		clientstr[strlen(clientstr)-1] = '\0'; // remove the '\n' symbol at the end
		printf("Main Server received the request from the client using TCP over port %d.\n", TCPPORT);
		// printf("serverM has received string from client: %s\n", clientstr);
		char inputusers[10][25];
		char notexist[MAXBUFLEN] = {0}; // send to the client
		char inA[MAXBUFLEN] = {0}; // send to serverA: name1, name2, name3,
		char inB[MAXBUFLEN] = {0}; // send to serverB
		int count = 0;
		char *token = strtok(clientstr, " "); // split the content by whitespace
		while(token != NULL){
			strcpy(inputusers[count], token);
			//printf("token is %s, its length is %d\n", token, strlen(token));
			if(nameInServer('A', token) == 0 && nameInServer('B', token) == 0){
			   strcat(notexist, token);
			   strcat(notexist, ", ");
			} else if (nameInServer('A', token)) {
			   strcat(inA, token);
			   strcat(inA, ", ");
			} else {
			   strcat(inB, token);
			   strcat(inB, ", ");
			}
			token = strtok(NULL, " ");
			count++;
		}

		// remove the comma and whitespace at the end
		notexist[strlen(notexist)-2] = '\0';
		inA[strlen(inA)-2] = '\0';
		inB[strlen(inB)-2] = '\0';

		/*
		printf("notexist: %s\n", notexist);
		printf("inA: %s\n", inA);
		printf("inB: %s\n", inB);
		*/

		/* names that do not exist; // if its len equals 0, means all exist
		   names that exist;
		   time intervals that fit for all
		*/
		char info2client[MAXBUFLEN] = {0}; // info to send to the client
		char recvfromA[MAXBUFLEN] = {0};
		char recvfromB[MAXBUFLEN] = {0};

		struct sockaddr_in src_addrA, src_addrB;
		socklen_t addr_lenA = sizeof(src_addrA);
		socklen_t addr_lenB = sizeof(src_addrB);

		// fill the array info2client
		if(strlen(notexist) != 0) {
			printf("%s do not exist. Send a reply to the client.\n", notexist);
			strcat(info2client, notexist); // notexist is not empty
		}
		strcat(info2client, ";"); // add a semicolon to separate the three groups

		if(strlen(inA) != 0) {
			printf("Found %s at Server A. Send to Server A.\n", inA);
			// query serverA, inA form: aa, bb, cc, dd
			sendto(udpsockfd, inA, MAXBUFLEN-1, 0, resA->ai_addr, resA->ai_addrlen);
			// receive response from serverA
			recvfrom(udpsockfd, recvfromA, MAXBUFLEN-1, 0, (struct sockaddr *)&src_addrA, &addr_lenA);
			// printf("recvstr from A is %s\n", recvfromA);
		}

		if(strlen(inB) != 0) {
			printf("Found %s at Server B. Send to Server B.\n", inB);
			// query serverB
			sendto(udpsockfd, inB, MAXBUFLEN-1, 0, resB->ai_addr, resB->ai_addrlen);
			// receive response from serverB
			recvfrom(udpsockfd, recvfromB, MAXBUFLEN-1, 0, (struct sockaddr *)&src_addrB, &addr_lenB);
			// printf("recvstr from B is %s\n", recvfromB);
		}

		if(strlen(inA) != 0)
			printf("Main Server received from server A the intersection result using UDP over port %d:\n%s.\n", UDPPORT, recvfromA);
		if(strlen(inB) != 0)
			printf("Main Server received from server B the intersection result using UDP over port %d:\n%s.\n", UDPPORT, recvfromB);

      int intersectionA[MAXSLOTLEN];
      int intersectionB[MAXSLOTLEN];  
		memcpy(intersectionA, convertTime(recvfromA), MAXSLOTLEN * sizeof(int));
		memcpy(intersectionB, convertTime(recvfromB), MAXSLOTLEN * sizeof(int));
		
		/*
      printf("intersectionA is ");
      for(int i = 0; i < MAXSLOTLEN; i++) {
         printf("%d ", intersectionA[i]);
      }
      printf("\n");
      
      printf("intersectionB is ");
      for(int i = 0; i < MAXSLOTLEN; i++) {
         printf("%d ", intersectionB[i]);
      }
      printf("\n");
      */

		int intersection[MAXSLOTLEN];
		memset(intersection, 0, sizeof(intersection));

      for(int i = 0; i < MAXSLOTLEN; i++) {
         intersection[i] = intersectionA[i] + intersectionB[i];
      }

      int srcNum = 0; // data from ServerA or ServerB or both or none
      if(strlen(inA) != 0 && strlen(inB) != 0)
         srcNum = 2;
      else if(strlen(inA) == 0 || strlen(inB) == 0)
         srcNum = 1;
      else // all names do not exist in serverA or serverB
         srcNum = -1;
      printf("Found the intersection between the results from server A and B:\n%s.\n", displayIntersection(intersection, srcNum));

		if(strlen(inA) != 0) {
			strcat(info2client, inA);			
		}
		if(strlen(inB) != 0) {
		   if(strlen(inA) != 0) // some names come from serverA
		      strcat(info2client, ", ");
			strcat(info2client, inB);
		}
		strcat(info2client, ";");

		char intersectionstr[MAXBUFLEN];
		char temp[5];
		memset(intersectionstr, 0, sizeof(intersectionstr));
		for(int i = 0; i < MAXSLOTLEN; i++) {
		   sprintf(temp, "%d", intersection[i]);
		   strcat(intersectionstr, temp);
		}
		// printf("intersectionstr is %s\n", intersectionstr);
		strcat(info2client, displayIntersection(intersection, srcNum));

		// printf("info2client is %s\n", info2client);

		// send the result to the client
		send(childsockfd, info2client, MAXBUFLEN-1, 0);
		printf("Main Server sent the result to the client.\n");

		close(childsockfd);
	} // end while

   close(tcpsockfd);
   close(udpsockfd);
	return 0;
}

int nameInServer(char c, char* name){ // c == 'A'/'B'
   if(c == 'A'){
      for(int i = 0; i < sizeA; i++) {
      //printf("userA[i] is %s, its length is %d\n", usersA[i], strlen(usersA[i]));
         if(strcmp(usersA[i], name) == 0) {
            return 1;
         }
      }
      return 0;
   } else { // c == 'B'
      for(int i = 0; i < sizeB; i++) {
         if(strcmp(usersB[i], name) == 0) {
            return 1;
         }
      }
      return 0;
   }
}

/*
   func: convert the time slots received from A/B into a int array
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

/*
   parameter time is the final time slots, like 001033330201...(length equals MAXSLOTLEN)
   parameter src_num is the number of users used to calculate the intersection
   return value: [[4,7],..]
*/
char *displayIntersection(int time[MAXSLOTLEN], int src_num) {
   static char intersection[MAXBUFLEN];
   memset(intersection, 0, sizeof(intersection));
   char str[5]; // used to convert int to string
   // intersection is time[i] == src_num
   // handle the first element specifically
   if(time[0] == src_num) {
         strcat(intersection, "0");
         strcat(intersection, ","); // separate the time slots with comma
   }
   for(int i = 1; i < MAXSLOTLEN-1; i++) {
      if(time[i-1] != src_num && time[i] == src_num && time[i+1] != src_num) { // a single number
         sprintf(str, "%d", i); // convert int i into string and store it into var str
         strcat(intersection, str);
         strcat(intersection, ",");
         sprintf(str, "%d", i+1); // convert int i into string and store it into var str
         strcat(intersection, str);
         strcat(intersection, ",");
      }
      else if(time[i-1] != src_num && time[i] == src_num) { // a start
         // printf("start is %d\n", i);
         sprintf(str, "%d", i); // convert int i into string and store it into var str
         strcat(intersection, str);
         strcat(intersection, ","); // separate the time slots with comma
      } else if(time[i-1] == src_num && time[i] == src_num && time[i+1] != src_num) { // an end
         // printf("end is %d\n", i);
         sprintf(str, "%d", i+1); // convert int i into string and store it into var str
         strcat(intersection, str);
         strcat(intersection, ","); // separate the time slots with comma
      }
   }
   // handle the last element specifically
   char temp[5];
   if(time[MAXSLOTLEN-1] == src_num && time[MAXSLOTLEN-2] == src_num) {
      sprintf(temp, "%d", MAXSLOTLEN-1);
      strcat(intersection, temp);
      strcat(intersection, ","); // separate the time slots with comma
   }
   // remove the comma at the end
   if(strlen(intersection) != 0) // have intersection
      intersection[strlen(intersection)-1] = '\0';

   // printf("intersection is %s\n", intersection);
   
   static char result[MAXBUFLEN];
   memset(result, 0, sizeof(result));
   strcat(result, "[");
   int count = 0;
   char* token = strtok(intersection, ",");
   while(token != NULL) {
      // printf("token is %s\n", token);
      if(count%2 == 0) { // even position
         strcat(result, "[");
         strcat(result, token);
         strcat(result, ",");
      } else { // odd position
         strcat(result, token);
         strcat(result, "],");
      }
      // printf("result is %s\n", result);
      token = strtok(NULL, ",");
      count++;
   }
   if(strlen(result) != 1)
      // remove a redundant comma at the end
      result[strlen(result)-1] = ']';
   else // under the scenario with no intersection, only with a '[' at the beginning
      strcat(result, "]");
   // printf("result is %s\n", result);
   return result;
}
