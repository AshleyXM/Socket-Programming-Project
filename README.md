# Socket-Programming-Project
A project for EE450 course at USC

# Problem Statement

Finding a time that works for everyone to meet is often a headache issue, especially for a group of people. People often need to exchange availability through back-and-forth emails and allow for several days to finally schedule a meeting. With the help of a meeting scheduler that gives a power to book meetings with users efficiently, users can save hours of time on unnecessary emailing and focus on more important things. There exist some survey tools online to help a group find the best meeting time by manually filling a shared table with their availability. It could however be more productive if the scheduler can maintain the availability of all users and schedule the meeting at the time that works for everyone accordingly.

In this project, we are going to develop a system to fasten the meeting scheduling process. The system receives a list of names involved in the meeting and returns the time intervals that work for every participant based on their availability. To maintain the privacy of everyone’s daily schedule, the availability of all users is stored on backend servers and cannot be accessed by any other servers or clients. Due to a large number of users, more than one backend servers are deployed to store their availability. A main server is then needed to manage which backend server a user’s information is stored in. The main server also plays a role of handling requests from clients.

When a user wants to schedule a meeting among a group of people, the user will start a client program, enter all names involved in the meeting and request the main server for the time intervals that works for every participant. Once the main server receives the request, it decides which backend server the participants’ availability is stored in and sends a request to the responsible backend server for the time intervals that works for all participants belonging to this backend server. Once the backend server receives the request from the main server, it searches in the database to get all requested users’ availability, runs an algorithm to find the intersection among them and sends the result back to the main server. The main server receives the time slots from different backend servers, runs an algorithm to get the final time slots that works for all participants, and sends the result back to the client. The scheduler then can decide the final meeting time according to the available time recommendations and schedule it in the involved users’ calendars.

Without the loss of generality, we assume there are one client, one main server and two backend servers in our project, as indicated in Figure 1. The full process can be roughly divided into four phases: **Boot-up**, **Forward**, **Schedule**, **Reply** (read details in “Application Workflow Phase Description” section).

- Client: used to access the meeting scheduling system.
- Main server (serverM): coordinate with the backend servers.
- Backend server (serverA and serverB): store the availability of all users and get the time slots that work for all meeting participants once receiving requests.

# Application Workflow Phase Description

## Phase 1: Boot-up

Please refer to the “Process Flow” section to start your programs in order of the main server, server A, server B and Client. Your programs must start in this order. Each of the servers and the client have boot-up messages which have to be printed on screen, please refer to the on-screen messages section for further information.

When two backend servers (server A and server B) are up and running, each backend server should read the corresponding input file and store the information in a certain data structure. You can choose any data structure that accommodates the needs. After storing all the data, server A and server B should then send all usernames they have to the main server via UDP over the port mentioned in the PORT NUMBER ALLOCATION section. Since the usernames are unique the main server will maintain a list of usernames corresponding to each backend server. In the following phases you have to make sure that the correct backend server is being contacted by the main server for corresponding usernames. You should print correct on screen messages onto the screen for the main server and the backend servers indicating the success of these operations as described in the “ON-SCREEN MESSAGES" section.

After the servers are booted up and the required usernames are transferred from the backend servers to the main server, the client will be started. Once the client boots up and the initial boot up messages are printed, the client waits for the usernames to be taken as inputs from the user. The user can enter up to 10 usernames (all of which are separated by a single space).

After booting up. Your client should first show a prompt:
*Please enter the usernames to check schedule availability:*

Assuming one entered:
*theodore callie*

You should store the above two usernames (*theodore* and *callie*). Once you have the usernames stored in your program, you can consider phase 1 of the project to be completed. You can proceed to phase 2.

## Phase 2: Forward

Once usernames involved in the meeting are entered in the client program, the client forms a message with these names and sends it to the main server over the established TCP connection. You can use any format for the message as long as the main server can receive the correct and intact list of usernames and can extract the information properly for the next step.

The main server first examines if all usernames exist in the database of backend servers according to the username list received from two backend servers in Phase 1. For the usernames that do not exist in the username list, it replies the client with a message saying that which usernames do not exist. For those that do exist, the main server splits them into two sublists based on where the user information is located and forwards those names to the corresponding backend server through UDP connection. Each of these servers have its unique port number specified in the “PORT NUMBER ALLOCATION” section with the source and destination IP address as “localhost” or “127.0.0.1”.

Client, main server and two backend servers are required to print out on-screen messages after executing each action as described in the “ON-SCREEN MESSAGES" section. These messages will help with grading in the event that the process does not execute successfully. Missing some of the on-screen messages might result in misinterpretation that your process failed to complete. Please follow the format when printing the on screen messages.

## Phase 3: Schedule

Once the backend servers receive the usernames, in order to help each participant schedule a common available time, in this phase, you will need to implement an algorithm to find the intersection of time availability of multiple users.

In the case of one participant, the backend server can directly send the time availability (i.e., a list of time intervals) back to the main server.

In the case of two participants, for example Alice and Bob, the backend server should first search in its database and find the time availability of them. The time availability of Alice and Bob are two lists of time intervals as stated in the “Input Files” section, denoted as

Alice_Time_List=[[t1_start,t1_end],[t2_start,t2_end],...]

Bob_Time_List=[[t1_start,t1_end],[t2_start,t2_end],...]

An algorithm should be developed with Alice_Time_List and Bob_Time_List as the input and output the intersection of time intervals of two lists. An illustration of time intervals and some example inputs and outputs are given in Figure 2, in which the intersection between time intervals [5,10] and [8,11] which is [8,10] are considered as a valid intersection (and the two time lists can have more than one valid intersections). However, the intersection between [15,17] and [17,18] which is [17,17] is **NOT** an intersection in our scenario because the definition of time intervals requires t[i]_start < t[i]_end and t[i]_end < t[i+1]_start as stated in the “Input Files” section.

![time_interval_examples](https://github.com/AshleyXM/Socket-Programming-Project/blob/main/images/time_interval_examples.png)

Figure: Illustration of time intervals and example inputs and outputs

In the case of more than two participants, you may run the algorithm for two participants iteratively, and each time find the intersection between the previously found intersection result and the time list of a new participant. You can also develop other algorithms that can directly work for multiple users. An example of the whole process is given as follows. A backend server receives the request for the time availability among Alice, Bob and Amy. The backend server first finds in its database the time availability of them obtaining

Alice_Time_List=[[1,10],[11,12]]

Bob_Time_List=[[5,9],[11,15]]

Amy_Time_List=[[4,12]]

An algorithm is then runned to output the intersection result [[5,9],[11,12]].

At the end of this phase, a backend server should have the intersection result among all participants, which is a list of time intervals, and be ready to send it to the main server. If no time interval is found, the result should be [] as shown in figure 2. The backend servers are required to print out on-screen messages after executing each action as described in the “ON-SCREEN MESSAGES" section.

## Phase 4: Reply

During this phase, the backend servers send the intersection result among all participants to the main server via UDP. Once the main server receives the results, the main server runs the algorithm for two participants developed in Phase 3 to identify the intersection between two intersection results received from two backend servers. For example, the main server receives the following intersection results which are two lists of time intervals from two backends servers

result_1= [[6,7],[10,12],[14,15]]

result_2= [[3,8],[9,15]]

Then the main server should feed these time intervals to your algorithm and have the result [[6,7],[10,12],[14,15]] ready. Then the main server sends the intersection result which is a list of time intervals back to the client via TCP.

When the client receives the result, it will print out the result and the prompt messages for a new request as follows:

*Time intervals <[time intervals]> works for <username1, username2, …>.*

*-----Start a new request-----*

*Please enter the usernames to check schedule availability:*

All servers and the client are required to print out on-screen messages after executing each action as described in the “ON-SCREEN MESSAGES" section.

## Extra Credits

After showing the meeting time recommendations (i.e., the intersection of all time intervals), a user can decide the final meeting time and schedule it in the involved users’ calendars. In this extra credit part, you should develop a functionality to schedule a meeting in the participants’ calendar. This phase happens after the previous four phases but before starting a new request.

The program should ask the user to enter the meeting time by printing the following prompt:

*Please enter the final meeting time to register an meeting:*

The user should enter an interval [t_start, t_end] chosen from the recommendations, such as [1, 2].

This entered time interval must be one of the intervals in the recommendations. For example, the recommendations are [[1,3],[8,10],[15,16],[21,23]], then the acceptable meeting times are [1,2] or [2,3] or [1,3], etc.. But [2,4] or [3,5] are not acceptable. Your program should check if the entered time period is valid or not. If it is not valid, you should print a message and ask the user to enter again:

*Time interval <[t_start, t_end]> is not valid. Please enter again:*

If it is valid, the client should pass the interval to the main server and the main server passes it to the corresponding backend server based on the entered username in Phase 1. The backend servers then remove this time interval from the involved users time availability list, indicating that this time slot is occupied by a meeting. For example, the original availability of a user is [[1,5],[7,8]]. After registering [1,2] as a meeting time, the availability in the database becomes [[2,5],[7,8]]. The backend server should print the on-screen messages showing the updates:

*Register a meeting at \<time slot\> and update the availability for the following users:*

*<username 1>: updated from \<original time availability list\> to \<updated time availability list\>*

*<username 2>: updated from \<original time availability list\> to \<updated time availability list\>*

*…*

For example:

*Register a meeting at [1,2] and update the availability for the following users:*

*Alice: updated from [[1,2],[7,10]] to [[7,10]]*

*Bob: updated from [[1,5],[7,8]] to [[2,5],[7,8]]*

After updating, the backend server sends a message to the main server indicating that the update is finished. And the main server will notify the client so that the client can start a new request.

# Illustration of the system
![image](https://user-images.githubusercontent.com/60614853/233798991-2c4fcc2b-4804-4444-88bd-9c7f434721e5.png)

# Problem Analysis

Based on the problem statement, we can conclude that for the basic part, firstly, the client sends a message to the main server via TCP, and then the main server requests server A and/or server B via UDP. After that, server A and/or server B reply to the main server via UDP, and then the main server replies to the client via TCP. And for the extra credits part, the client replies to the main server with a selected time interval via TCP, and then the main server sends it to server A and/or serve B. After that, server A and/or server B update their database and notify the main server, and then the main server notify the client that the databases have been updated successfully. The flow chart can be drawn as follow:

![basic_part_flow_chart](https://github.com/AshleyXM/Socket-Programming-Project/blob/main/images/basic_part_flow_chart.png)

![extra_credit_flow_chart](https://github.com/AshleyXM/Socket-Programming-Project/blob/main/images/extra_credit_flow_chart.png)

# Clarification

The tag **v1.0** is implemented based on the basic requirement, and the tag **v2.0** is implemented based on the extra credit part.

**Note:** All the code in this project only cares about the functions, but pays no attention to the exception aspect, which means that, I have made no effort to check invalid inputs, or in other words, there is no exception checking mechanism in this project.

# How to compile and run

![image](https://user-images.githubusercontent.com/60614853/233798970-7212a547-ad55-41a2-af60-be2e156a3799.png)

**Note:** The launching order must be like this: serverM -> serverA -> serverB -> client. And when you want to stop the processes, I would recommend to kill the processes in the reverse order (that is, client -> serverB -> serverA -> serverM), otherwise, you might get into some unexpected port-related problems.

# Screenshots



# Gains

1. I got to know the relationship between several important functions in **socket network**. I have drawn some figures shown as below based on my understanding:

   ![UDP_connection_flow_chart](https://github.com/AshleyXM/Socket-Programming-Project/blob/main/images/UDP_connection_flow_chart.png)

2. We must be really careful when we use **pointers**. As it is easy for us to forget the fact that a pointer points to a variable and when you update the value via pointers, the pointed variable will alter at the same time, which is unexpected. For example, as for the function **strtok**, the first parameter is a pointer, so when you are going to split the first parameter by function strtok, it will alter the original variable.

3. When we are writing a program which can keep running and accepting inputs, **be careful to reset the global variables**. Never reset the variables which do not need to reset!

4. For every program, the **edge values** are always the most vulnerable part. So it is a good way for programmers to write multiple test cases to test the edge values.

5. I learnt how to write **Makefile** for C programs, which is essential and helpful took when we write large C programs.

6. There is one point which is worth paying a lot of attention to, which is a lasting problem which confused me for a long time. When peers are communicating with each other with TCP connection, it is weird that the return value of function send from server side is -1, and the error code is "**Connection reset by peer**". I have searched it on Google, there are several possible reasons: It has been a long time to have this TCP connection (which can be excluded in my case because it appeared at the same position); or the other side, say the client in my case, has closed the socket before the function send, which can also excluded in my case. And I suppose it is because the buffer of the TCP connection has overflowed (but I am not sure, just an inference). So I solved this issue by establishing a new TCP connection to send the status code between the client and the serverM.

