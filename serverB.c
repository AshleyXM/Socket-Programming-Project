#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
// required header files
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define LOCALHOST "127.0.0.1"
#define MYPORT 22015
#define SERVERMPORT "23015"
#define MAXBUFLEN 1000

typedef struct user{
	char username[25]; // a maximum length of 20
	int slots[10][2]; // contain a maximum of 10 time intervals, a minimum of 1
} user;

user users[200]; // up to 200 lines in each file
int usernum = 0; // the total number of users

/* get the intersection of time slots of given usernames */
/* the names in parameter username are separated with comma */
char *findIntersection(char *usernames, int src_num);
char *formatTime(char* timestr);

int main(){
	printf("Server B is up and running using UDP on port %d.\n", MYPORT);
	FILE *fp;
	if((fp = fopen("b.txt","r")) == NULL){
		puts("Fail to open file a.txt!");
		exit(-1);
	}
	char left[1000], right[1000];
	char temp[2000] = {0};
	// dump the data into struct variable
	while(fgets(temp, 2000, fp) != NULL){ // read by line
		// remove whitespaces
		int i = 0;
		while(temp[i] != '\0'){
			if(temp[i] == ' ')
				for(int j = i; j < strlen(temp); j++) // move the substring behind the whitespace forward by one unit
					temp[j] = temp[j+1];
			else
				i++;
		}
		temp[strlen(temp)-1] = '\0'; // remove the last \n symbol
		// dump left data
		int pos_semicolon = 0; // the position of semicolon
		while(temp[pos_semicolon] != ';')
			pos_semicolon++;
		memcpy(left, temp, pos_semicolon);
		strcpy(users[usernum].username, left); // set username
		// dump right data
		strcpy(right, temp+pos_semicolon+1);
		int count = 0;
		char *token = strtok(right, ",[]"); // split the content by three symbols: , [ ]
		while(token != NULL){
			users[usernum].slots[count/2][count%2] = atoi(token);
			token = strtok(NULL, ",[]");
			count++;
		}
		// clear temporary variable
		memset(temp, 0, sizeof(temp));
		memset(left, 0, sizeof(left));
		memset(right, 0, sizeof(right));
		usernum++; // move to the next user
	}

	fclose(fp); // close the fd

	// define the username info to be sent to the main server
	char usernameInfo[MAXBUFLEN] = {0};
	for(int i = 0; i <= 200; i++){
		if(strlen(users[i].username) != 0){
			strcat(usernameInfo, users[i].username); // copy the username into usernameInfo
			strcat(usernameInfo, " "); // add a whitespace to separate the names
			continue;
		}
		break;
	}

	// send to main server
	struct addrinfo hints;
	struct addrinfo *res, *p; // store the address of the main server
	int sockfd;
	struct sockaddr_in mysock;
	memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP datagram socket
	// get ready to connect
	getaddrinfo(LOCALHOST, SERVERMPORT, &hints, &res);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0); // get fd

	bzero(&mysock, sizeof(mysock));
	mysock.sin_family = AF_INET;
	mysock.sin_port = htons(MYPORT);
	mysock.sin_addr.s_addr = inet_addr(LOCALHOST);

	bind(sockfd, (struct sockaddr *)&mysock, sizeof(struct sockaddr)); // bind my port to sockfd

	sendto(sockfd, usernameInfo, MAXBUFLEN-1, 0, res->ai_addr, res->ai_addrlen); // send the usernames in serverB to the main server

	printf("Server B finished sending a list of usernames to Main Server.\n");

	while(1){
		struct sockaddr_in src_addr; // the address of the sender, to be got
		socklen_t addr_len = sizeof(src_addr);
		char recvnames[MAXBUFLEN];
		memset(recvnames, 0, sizeof(recvnames)); // initiate the array recvnames
		recvfrom(sockfd, recvnames, MAXBUFLEN-1, 0, NULL, NULL); // receive usernames from the main server
		// printf("recvnames in serverB is %s\n", recvnames);
      printf("Server B received the usernames from Main Server using UDP over port %d.\n", MYPORT);
		// printf("serverA recvnames is %s\n",recvnames);
		int currentnum = 1; // the number of names in recvnames
		for(int i = 0; i < strlen(recvnames); i++) { // n commas -> (n+1) names
		   if(recvnames[i] == ',')
		      currentnum++;
		}
		// printf("currentnum is %d\n", currentnum);
		char intersection[MAXBUFLEN];
		memset(intersection, 0, sizeof(intersection));
		strcpy(intersection, findIntersection(recvnames, currentnum));
		printf("Found the intersection result: %s for %s.\n", formatTime(intersection), recvnames);
		// send the intersection to the main server
		sendto(sockfd, intersection, MAXBUFLEN-1, 0, res->ai_addr, res->ai_addrlen);
		printf("Server B finished sending the response to Main Server.\n");
	}

	freeaddrinfo(res);
	close(sockfd);

}


/* get the intersection of time slots of given usernames */
/* the names in parameter username are separated with comma */
char *findIntersection(char *usernames, int src_num) {
   int time[101];
   memset(time, 0, sizeof(time)); // initiate the time array
   char usernames_copy[MAXBUFLEN];
   strcpy(usernames_copy, usernames);
   char* token = strtok(usernames_copy, ", ");
   while(token != NULL) {
      // printf("token is %s\n",token);
      // fill the time array with time slots
      for(int i = 0; i < usernum; i++) {
         if(strcmp(token, users[i].username) == 0) { // token is users[i]
            for(int j = 0; users[i].slots[j][0] != '\0'; j++) {
               for(int k = users[i].slots[j][0]; k <= users[i].slots[j][1]; k++) {
                  time[k]++;
                  // printf("time[%d] is %d\n",k,time[k]);
               }
            }
         }

      }
      token = strtok(NULL, ", ");
   }

   /*
   for(int i = 0; i < 101;i++){
      printf("%d",time[i]);
   }
   printf("\n");
   */

   static char intersection[MAXBUFLEN];
   memset(intersection, 0, sizeof(intersection));
   char str[5]; // used to convert int to string
   // intersection is time[i] == count
   // handle the first element specifically
   if(time[0] == src_num) {
         strcat(intersection, "0");
         strcat(intersection, ","); // separate the time slots with comma
   }
   for(int i = 1; i < 100; i++) {
      if(time[i-1] != src_num && time[i] == src_num && time[i+1] == src_num) { // a start
         // printf("start is %d\n", i);
         sprintf(str, "%d", i); // convert int i into string and store it into var str
         strcat(intersection, str);
         strcat(intersection, ","); // separate the time slots with comma
      } else if(time[i-1] == src_num && time[i] == src_num && time[i+1] != src_num) { // an end
         // printf("end is %d\n", i);
         sprintf(str, "%d", i); // convert int i into string and store it into var str
         strcat(intersection, str);
         strcat(intersection, ","); // separate the time slots with comma
      }
   }
   // handle the last element specifically
   if(time[100] == src_num && time[99] == src_num) {
      strcat(intersection, "100");
      strcat(intersection, ","); // separate the time slots with comma
   }
   // remove the comma at the end
   if(strlen(intersection) != 0) // have intersection
      intersection[strlen(intersection)-1] = '\0';

   // printf("intersection is %s\n", intersection);
   return intersection;
}

/* timestr: 2,3,5,7,  return value: [[2,3],[5,7]]*/
char *formatTime(char* timestr) {
   char timestr_copy[MAXBUFLEN];
   strcpy(timestr_copy, timestr);
   static char result[MAXBUFLEN];
   memset(result, 0, sizeof(result));
   strcat(result, "[");
   int count = 0;
   char* token = strtok(timestr_copy, ",");
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
      result[strlen(result)-1] = ']';
   else // under the scenario with no intersection
      strcat(result, "]");
   // printf("result is %s\n", result);
   return result;
}

