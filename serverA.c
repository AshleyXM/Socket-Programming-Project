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
#define MYPORT 21015
#define SERVERMPORT "23015"
#define MAXBUFLEN 1000
#define MAXSLOTLEN 100

typedef struct user{
	char username[25]; // a maximum length of 20
	int slots[MAXSLOTLEN]; // store the available time info for the current user, range 0-100
} user;

user users[200]; // up to 200 lines in each file
int usernum = 0; // the total number of users

/* get the intersection of time slots of given usernames */
/* the names in parameter username are separated with comma */
/* parameter src_num is the number of users given in the first paramter*/
int *findIntersection(char *usernames, int src_num);
/*
   parameter time is the final time slots, like 001033330201...(length equals MAXSLOTLEN)
   parameter src_num is the number of users used to calculate the intersection
   return value:
*/
char *displayIntersection(int time[MAXSLOTLEN], int src_num);

int main(){
	printf("Server A is up and running using UDP on port %d.\n", MYPORT);
	FILE *fp;
	if((fp = fopen("a.txt","r")) == NULL){
		puts("Fail to open file a.txt!");
		exit(-1);
	}
	char left[MAXBUFLEN], right[MAXBUFLEN];
	char temp[MAXBUFLEN*2] = {0};
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
		temp[strlen(temp)-1] = '\0'; // remove the last \n symbol at the end
		// dump left data
		int pos_semicolon = 0; // the position of semicolon
		while(temp[pos_semicolon] != ';')
			pos_semicolon++;
		memcpy(left, temp, pos_semicolon * sizeof(char)); // dump the content before the semicolon into left
		strcpy(users[usernum].username, left); // set username
		// dump right data
		strcpy(right, temp + pos_semicolon + 1);
		int inputtime[20]; // store all the numbers input, a maximum of 10 time intervals
		int count = 0; // the number of input numbers
		char *token = strtok(right, ",[]"); // split the content by three symbols: , [ ]
		while(token != NULL) {
			inputtime[count] = atoi(token); // atoi: convert string into int
			token = strtok(NULL, ",[]");
			count++;
		}

		// dump the inputtime into struct user
		for(int i = 0; i < count; i += 2) { // count must be a odd number
		   for(int j = inputtime[i]; j <= inputtime[i+1]-1; j++) {
            users[usernum].slots[j] = 1;
         }
		}

		// clear temporary variable
		memset(temp, 0, sizeof(temp));
		memset(left, 0, sizeof(left));
		memset(right, 0, sizeof(right));
		usernum++; // move to the next user
	}

	fclose(fp); // close the fd

	// define the username info to be sent to the main server: "aa bb cc dd "
	char usernameInfo[MAXBUFLEN] = {0};
	for(int i = 0; i <= 200; i++) {
		if(strlen(users[i].username) != 0) {
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
   // send the usernames in serverA to the main server
   // usernameInfo form: "aa bb cc dd "
	sendto(sockfd, usernameInfo, MAXBUFLEN-1, 0, res->ai_addr, res->ai_addrlen);

	printf("Server A finished sending a list of usernames to Main Server.\n");

	while(1){
		struct sockaddr_in src_addr; // the address of the sender, to be got
		socklen_t addr_len = sizeof(src_addr);
		char recvnames[MAXBUFLEN];
		memset(recvnames, 0, sizeof(recvnames));
		// receive usernames from the main server
		// recvnames form: "aa, bb, cc, dd"
		recvfrom(sockfd, recvnames, MAXBUFLEN-1, 0, NULL, NULL);
      printf("Server A received the usernames from Main Server using UDP over port %d.\n", MYPORT);
		// printf("serverA recvnames is %s\n",recvnames);
		int currentnum = 1; // the number of users in the current request
		for(int i = 0; i < strlen(recvnames); i++) {
		   if(recvnames[i] == ',')
		      currentnum++;
		}

		int *intersection = NULL; // store the final intersection in the current request
		char displayres[MAXBUFLEN]; // display the final intersection in the current request
		memset(displayres, 0, sizeof(displayres));
		intersection = findIntersection(recvnames, currentnum);
		strcpy(displayres, displayIntersection(intersection, currentnum));
		printf("Found the intersection result: %s for %s.\n", displayres, recvnames);
		// send the intersection to the main server
		// intersection form: 0012000333330000...
		sendto(sockfd, displayres, MAXBUFLEN-1, 0, res->ai_addr, res->ai_addrlen);
		printf("Server A finished sending the response to Main Server.\n");
	}

	freeaddrinfo(res);
	close(sockfd);
}

/* func: get the intersection of time slots of given usernames
   the names in parameter username are separated with comma
   parameter src_num is the number of users given in the first paramter
   return value: an int array which shows the intersection with numbers
*/
int *findIntersection(char *usernames, int src_num) {
   static int time[MAXSLOTLEN];
   memset(time, 0, sizeof(time)); // initiate/clear the time array
   char usernames_copy[MAXBUFLEN];
   strcpy(usernames_copy, usernames);
   char* token = strtok(usernames_copy, ", ");
   while(token != NULL) {
      // printf("token is %s\n",token);
      // fill the time array with time slots
      for(int i = 0; i < usernum; i++) { // traverse all the users: usernum
         if(strcmp(token, users[i].username) == 0) { // token is users[i]
            for(int j = 0; j < MAXSLOTLEN; j++) {
               time[j] = time[j] + users[i].slots[j];
            }
         }
      }
      token = strtok(NULL, ", ");
   }

   /*
   printf("time is ");
   for(int i = 0; i < MAXSLOTLEN; i++)
      printf("%d ", time[i]);
   printf("\n");
   */

   return time;
}

/*
   parameter time is the final time slots, like 001033330201...(length equals MAXSLOTLEN)
   parameter src_num is the number of users used to calculate the intersection
   return value:
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
