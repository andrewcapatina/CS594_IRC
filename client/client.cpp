/*
 *
 * Andrew Capatina
 * 5/8/2018
 *
 * Client code for IRC program.
 *
 *
 */

#include "header.h"

static int BUF_SIZE = 1023;     // Size of buffer for sending and receiving.
static int BUF_MEM_SIZE = 1023;     // Size of buffer for sending and receiving.

using namespace std;

static int sock_fd = 0;
static int registered = -1;
char current_room[16] = {0};
static int prompt_flag = 0;
char * rooms_joined[10] = {NULL};
char gl_username[17];

int list_current_rooms();

/*
 * Function to select a function based off user input.
 */
int select_init_function(int fd, int operation)
{
  sock_fd = fd;         // Socket to perform operation on.
  if(registered == -1 && operation > 0) // If user isn't registered and their request requires registration, tell them.
    return 2;
  else if(registered == 0 && operation == 0)   // If they have registered already and try again, prompt them so. 
    return 3;
  if(operation == 5 && current_room == NULL)    // Check user is in a room before sending a message.
    return 4;
     
  switch(operation) // Switch statement for app functions. 
  {
    case 0 : registered = register_uname(sock_fd);      // Register username with server.
              break;
    case 1 : create_room();                             // Create a room hosted by server.
              break;
    case 2 : get_rooms(sock_fd); return 0;           // Get all rooms currently hosted.
              break; 
    case 20: list_current_rooms();
              break;
    case 3 : if(disconnect(sock_fd) == 0) return 1;  // Disconnect from server.
              break;
    case 4 : join_room(sock_fd);                     // Join any room.
              break;
    case 5 : send_msg_to_server(sock_fd);            // Send message to current room.
              break;
    case 51 :private_message(sock_fd);
              break;
    case 6 : leave_room(sock_fd);                    // Leave any room.
              break;
    case 7 : switch_rooms();                         // Switch current room.
              break; 
    case 8 : list_members(sock_fd);                  // List members of a current room.
              break;
  }

  operation = -1;

  return 0;

}

/*
 * Function to prompt and getuser menu options.
 * Input is polled.
 */
int client_menu_begin(int & flag)
{

  int response_int = 0; // Records response.
  int rc = 0;           // Return code variable.
  int idle = 50;        // Variable for idle.
  struct pollfd pfd = {STDIN_FILENO,POLLIN,0};  // Populating poll struct to monitor input events on STDIN.
  
  if(flag == 0)
  {
    cout << "//////////////////////////////////" << endl;
    cout << "'//~' denotes that the following options below are ready to be chosen." << endl;
    cout << "'^^.' denotes server messages." << endl;
    cout << "Message format: >>room/sender: " << endl;
    cout << "                  >>'msg' " << endl;
    cout << "----------------------------------" << endl;               // Display all options to users.
    cout << "Press 0 to register your username." << endl;
    cout << "Press 1 to create a room" << endl;
    cout << "Press 2 to list all rooms" << endl;
    cout << "Press 20 to list all currently joined rooms." << endl;
    cout << "Press 3 to disconnect" << endl;
    cout << "Press 4 to join a room." << endl;
    cout << "Press 5 to send a message to currently joined room." << endl;
    cout << "Press 51 to send a message to a specific user." << endl;
    cout << "      - User must be in the SAME room." << endl;
    cout << "Press 6 to leave any currently connected room." << endl;
    cout << "Press 7 to switch rooms for sending messages. Must be member of room." << endl;
    cout << "Press 8 to list all members of the current room." << endl;
    cout << "----------------------------------" << endl;
    printf("//~");
    fflush(stdout);
    rc = poll(&pfd,1,1000);
    if(rc == 1)         // If input is waiting to be read:
    {
      cin >> response_int;      // Read input.
      cin.ignore(100,'\n');
      prompt_flag = 1;
    }
    if(rc == 0)         // If input isn't ready:
    {
      prompt_flag = 0;
      flag = 1;         // Set flag so we don't spam user.
      return idle;      // Return idle state.

    }
    flag = 1;
  }
  else
  {
    if(prompt_flag == 1)        // Prompt to remind user they can enter input.
    {
      printf("//~");
      fflush(stdout);
    }


    rc = poll(&pfd,1,1000);     // Checking status of descriptor.
    if(rc == 1)
    {
      cin >> response_int;      // Get user input if available.
      cin.ignore(100,'\n');
      prompt_flag = 1;
    }
    if(rc == 0)
    {
      prompt_flag = 0;  // It no input available, remain idle.
      return idle;

    }

    if(response_int == 10)      // If user needs to see all commands again:
    {
      flag = 0;         // Set flag back to zero.
      return 10;
    }
  }

  return response_int;          // Return user response.
}

/*
 * Function to connect to server.
 */
int connect_to_server()
{

  int connect_rtn = -1;         // Holds return value of bind function call.
  int buf_size = 1023;
  //int rc = 0;                 // General return code variable.

  struct sockaddr_in6 server_addr;      // Pointer to socket address structure.

  /*
   * Creating socket using IPv6, TCP, and IP protocol. TCP6 socket.
   */
  sock_fd = socket(AF_INET6,SOCK_STREAM,IP_PROTOCOL);
  if(sock_fd == -1)     // Checking return value.
    perror("Client failed to create socket.");

  if(setsockopt(sock_fd,SOL_SOCKET,SO_SNDBUF,&buf_size,sizeof(buf_size)) < 0)   // Setting receive buffer size.
    perror("setsockopt");
  if(setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,&buf_size,sizeof(buf_size))<0)     // Setting send buffer size.
    perror("setsockopt");

  server_addr.sin6_family = AF_INET6;                   // Specifying IPv6 protocol.
  inet_pton(AF_INET6,"::1", &server_addr.sin6_addr);    // Looback address.
  server_addr.sin6_port = htons(PORT);                  // Assigning server port, convert to network byte order. 

  connect_rtn = connect(sock_fd,(struct sockaddr*) &server_addr, sizeof(server_addr));  // Establishing connection with server.

  if(connect_rtn == -1)         // Checking return value.
  {
    perror("Client failed to connect");
    return -1;                  // return < 0
  }
  else
    cout << "Connected to server." << endl;
  

  return sock_fd;       // Return descriptor.
}

/*
 * Function to assist user in creating chat room.
 */
int create_room()
{
  int rc = 0;                           // Return code variable.
  char room_name[16] = {0};             // Buffer for room name.
  char buf[1024] = {0};                 // Buffer for networking.
  char buffer[1024] = {0};              // "      "     " 
  char op_code[6] = "10000";         // Operation number.
  char conf_code[6] = "10500";         // Operation number.
  char * code;

  do
  {
    memset(room_name,0,16);
    cout << "Please enter a name for the chat room" << endl;      
    cout << "A maximum of 16 characters can be used, including spaces." << endl;
    cin.getline(room_name,16,'\n');       // Getting room name user desires.
    rc = search_str(room_name);
  }while(rc != 0);
  //cin.ignore(100,'\n');

  strcpy(buf,op_code);       // Create message format for server.
  strcat(buf,"/");
  strcat(buf,room_name);
  strcat(buf,"/");
  rc = send_msg(buf);           // Send request to server.
  rc = recv(sock_fd,buffer,BUF_SIZE, MSG_WAITALL);       // Read server response.
  if(rc < 0)
    perror("read");

  code = strtok(buffer,"/");   // Split input and check that the operation is correct.
 

  if(strcmp(code,conf_code) == 0)    // If the correct code is returned, room was created successfully.
  {
    cout << "Room successfully created!" << endl;
    memset(buffer,0,BUF_MEM_SIZE);
    memset(current_room,0,strlen(current_room));
    memcpy(current_room,room_name,16);    // Set newly created room to current room for user.
    for(int i = 0; i < 10; ++i)           // Add room name to total number of rooms joined.
    {
      if(rooms_joined[i] == NULL)         // Add to empty index.
      {
        rooms_joined[i] = new char [16];          
        memcpy(rooms_joined[i],room_name,16);
        cout << "JOINED:" << rooms_joined[i] << endl;
        return 0;
      }
    }
  }
  else
  {
    cout << "Room creation failed!" << endl;
    memset(current_room,0,16);    // Set newly created room to current room for user.
    return 1;

  }
  return 0;
}


/*
 * Function to simply send a message to a socket.
 */
int send_msg(char * buf)
{
  int rc = 0;   // Return code variable.
  rc = send(sock_fd,buf,BUF_SIZE, 0);   // Send paramater to server.
  if(rc < 0)    // Error checking.
  {
    perror("Failed to send message");
    return rc;
  }
 
  return rc;
}

/*
 * Function to register assist in 
 * user registration.
 */
int register_uname(int fd)
{
  int rc = 0;           // Return code variable.
  sock_fd = fd;         // Socket descriptor to send to.
  //char buf[1024] = "00002";   //
  char buf[1024] = {0};         // Buffer for networking.
  char op_code[4] = "500";     // Operation code.
  char conf_code[4] = "550";     // Acknowledgement code.
  char temp_name[17] = {0};     // Temporary array for username.
  
  strncpy(buf,op_code,4);      // Create format for message beginning with operation code.
  strcat(buf,"/");              // Add delimiter.

  do
  {
    memset(temp_name,0,17);
    cout << "Please enter a username." << endl;                           // Prompt user for username choice.
    cout << "The username must be 16 characters at most with no spaces." << endl;
    cin.getline(temp_name,16,'\n');       // Get input.
    //cin.ignore(100,'\n');
    rc = search_str(temp_name);
  }while(rc != 0);
  strcat(buf,temp_name);                // Add request to message for server.
  strcat(buf,"/");                      // Add delimiter.
  memcpy(gl_username,temp_name,16); 
  eliminate_spaces(buf);                // Calling function to eliminate white spaces from string.
  send_msg(buf);                        // Send request to server.

  rc = recv(sock_fd,buf,BUF_SIZE,MSG_WAITALL);    // Wait for server acknowledgement.
  if(rc ==  0)          // Error checking.
  {
    perror("Server lost connection. Exiting application.");
    sleep(2);
    return 0;
  }


  if(strcmp(buf,conf_code) == 0)          // If buffer matches operation code, operation was a success.
  {
    cout << "Username successfully created." << endl;
    return 0;
  }

  return 1;     // Return fail.
}

/*
 * Function to disconnect the client from the server.
 */
int disconnect(int socket)
{
  int rc = 0;                           // Return code variable.
  char op_code[1024] = "30000";        // Acknowledgement code.
  char conf_code[1024] = "30500";        // Acknowledgement code.
  char buf[1024]={0};                   // Buffer for networking.
  strcpy(buf,op_code);                 // Copy operation code to buffer. 
  strcat(buf,"/");                      // Add delimiter.
  send_msg(buf);                        // Send message to server.

  rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL);          // Wait for server acknowledgement.
  if(rc ==  0)
  {
    perror("Server lost connection. Exiting application.");
    sleep(2);
    return 0;
  }

  if(strcmp(buf,conf_code) == 0)          // Checking if server was able to process request. Closing socket in both cases.
  {
    cout << "Disconnected" << endl;
    close(socket);
  }
  else
  {
    cout << "Fail to disconnect." << endl;
    cout << "Closing socket anyways." << endl;
    close(socket);
  }

  return 0;

}

/*
 * Function to have server send rooms of all
 * rooms currently available. 
 */
int get_rooms(int socket)
{
  int rc = 0;                   // Return code variable.
  char op_code[6] = "20000";    // Operation code.
  char conf_code[6] = "20500";    // ACK code.
  char fail_code[6] = "20250";    // ACK code.
  char no_rooms_code[6] = "20300"; // Code indicating there are no rooms.
  char buf[1024] = {0};         // Buffer for networking.
  char * chat_name = NULL;      // Holds chat room name.
  char * operation = NULL;      // Holds current operation.
  char * temp = NULL;           // Temporary array for splitting input.
  
  strncpy(buf,op_code,6);       // Populate buffer with operation code.

  //send(socket,buf,BUF_SIZE,0);  // Send request to server.
  rc = send_msg(buf);           // Send request to server.
  if(rc < 0)    // Error check.
  {
    perror("send");
    return 1;
  }
 
  int i = 1;    // Counter for loop below.
  do            // Rooms will be sent one by one. Loop until finished.
  {
    rc = 0;
    rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL); // Get message from server.
    if(rc ==  0)        // Error checking.
    {
      perror("Server lost connection. Exiting application.");
      sleep(2);
      return 0;
    }
    if(rc > 0)
    {
      temp = strtok(buf,"/");   // Split input and check that the operation is correct.
      operation = temp;
      if(strcmp(operation,conf_code) == 0)
      {
        temp = strtok(NULL,"/");        // Split string up to delimiter.
        chat_name = temp;               // Store chat name.
        cout << "ROOM " << "( " << i << "): " << chat_name << endl;
        ++i;
      }
      else if(strcmp(operation,fail_code) == 0)
      {
        rc = 0;
      }
      else if(strcmp(operation,no_rooms_code) == 0)
      {
        rc = 0;
        cout << "There are no rooms available." << endl;
      }
    }
      memset(buf,0,BUF_MEM_SIZE);   // Reset buffer for next iteration.

  }while(rc > 0);


  return 0;
}

/*
 * Function that allows the client to 
 * join any room they AREN'T a member
 * of.
 */
int join_room(int socket)
{
  char op_code[6] = "40000";            // Operation code.
  char conf_code[6] = "40500";          // Confirmation code.          
  char fail_code[6] = "40250";          // Fail code.
  char room_name[17] = {0};             // Array for room name.
  char buf[1024] = {0};                 // Buffer for networking.
  char * temp;                  
  int rc = 0;                           // Return code variable.

  do
  {
    memset(room_name,0,17);
    cout << "Enter the room name to be joined." << endl;  // Prompt user.
    cin.getline(room_name,16,'\n');                       // Get room they desire to join.
    //cin.ignore(100,'\n');
    rc = search_str(room_name);
  }while(rc != 0);

  strncpy(buf,op_code,16);      // Create message format for server.
  strcat(buf,"/");
  strcat(buf,room_name);
  strcat(buf,"/");
  
  rc = send_msg(buf);                // Send message to server.

  rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL);      // Wait for response acknowledging request.
  if(rc ==  0)          // Error checking.
  {
    perror("Server lost connection. Exiting application.");
    sleep(2);
    return 0;
  }
  temp = strtok(buf,"/");       // Parse buffer up to delimiter.
  if(strcmp(temp,conf_code) == 0) // Inspect that operation code is correct.
  {
    memcpy(current_room,room_name,16);  // Set current room to the room requested.
    cout << "Successfully joined room: " << room_name << endl;
  }
  else if(strcmp(temp,fail_code) == 0)    // Else check to see if they are already a member.
    cout << "Error joining room. May already be a member." << endl;

  for(int i = 0; i < 10; ++i)           // Add room name to total number of rooms joined.
  {
    if(rooms_joined[i] == NULL)         // Add to empty index.
    {
      rooms_joined[i] = new char [16];          
      memcpy(rooms_joined[i],room_name,16);
      cout << "JOINED:" << rooms_joined[i] << endl;
      return 0;
    }
  }

  return 0;
}

/*
 * Function that creates the message format for 
 * the server.
 */
int send_msg_to_server(int socket)
{

  char buf[1024];                       // Buffer for networking.
  char msg_temp[900];                   // buffer for message.
  char * msg_arg;                  
  char * temp;                  
  char op_code[6] = "50000";            // Operation code.
  char conf_code[6] = "50500";            // Operation code.
  //char fail_code[6] = "50250";            // Operation code.
  int rc =0;                            // Return code variable.

  do
  {
    memset(buf,0,BUF_MEM_SIZE);
    cout << "Please enter a message up to 900 characters. Can't enter '/'." << endl;        // Prompt user to enter messages.
    cin.ignore(0,'\n');
    cin.getline(msg_temp,900,'\n');       // Populate buffer with input from user.
    rc = search_str(msg_temp);
  }while(rc != 0);
  
  msg_arg = msg_temp;

  strncpy(buf,op_code,6);               // Create message format for server.
  strcat(buf,"/");
  strcat(buf,current_room);
  strcat(buf,"/");
  strcat(buf,msg_temp);
  strcat(buf,"/");

  rc = send_msg(buf);                // Send message to server.

  rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL);      // Wait for response acknowledging request.
  if(rc ==  0)          // Error checking.
  {
    perror("Server lost connection. Exiting application.");
    sleep(2);
    return 0;
  }
  temp = strtok(buf,"/");          // Split buffer sent from server.
  if(strcmp(temp,conf_code) == 0)         // Check for correct acknowledgement code.
    display_chat_message(gl_username, current_room,msg_arg);
  else
    cout << "Failed to send message" << endl;


  return 0;
}

/*
 * Function that allows the client to 
 * leave any room they are a member of.
 */
int leave_room(int socket)
{
  int rc = 0;                   // Return code variable.
  char room_name[17] = {0};     // array for room name.
  char * temp;
  char buf[1024] = {0};         // Buffer for networking.
  char op_code[6] = "60000";    // Operation code.
  char conf_code[6] = "60500";    // ACK code.
  
  do
  {
    memset(room_name,0,17);
    cout << "Your current room is: " << current_room << endl;             // Prompt user.
    cout << "Enter the name of the room you wish to leave. 16 characters max." << endl;
    cin.getline(room_name,16,'\n');       // Get name of room they desire to leave.
    rc = search_str(room_name);
  }while(rc != 0);

  strncpy(buf,op_code,6);       // Create message format for server.
  strcat(buf,"/");
  strcat(buf,room_name);
  strcat(buf,"/");
  
  send_msg(buf);                // Send message to server.
  memset(buf,0,BUF_MEM_SIZE);       // Reset buffer contents.

  rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL);      // Wait for response acknowledging request.
  if(rc ==  0)                  // Error checking.
  {
    perror("Server lost connection. Exiting application.");
    sleep(2);
    return 0;
  }
  temp = strtok(buf,"/");
  if(strcmp(temp,conf_code) == 0)         // Check that server acknowledgement code is correct.
    cout << "Successfully left room." << endl;
  else
    cout << "Failed to leave room." << endl;

  for(int i = 0; i < 10; ++ i)
  {
    if(rooms_joined[i] != NULL)
    {
      if(strcmp(room_name,rooms_joined[i]) == 0)
      {
        delete rooms_joined[i];
        rooms_joined[i] = NULL;
        return 0;
      }
    }
  }

  return 0;

}

/*
 * Function to switch rooms for client.
 * Server is not consulted for this function.
 */
int switch_rooms()
{
  char to_join[17] = {0};       // Buffer for input.
  int rc = 0;                   // Return code variable.

  do
  {
    memset(to_join,0,17);
    cout << "Please enter a room to switch to." << endl;  // Prompt user and get option.
    cin.getline(to_join,16,'\n');
    rc = search_str(to_join);
  }while(rc != 0);

  if(to_join == NULL)   // Check if input was given.
  {
    cout << "Null parameter for switching rooms." << endl;
    return 1;
  }
  for(int i = 0; i < 10; ++i)   // Loop through number of rooms one can join.
  {
    if(strcmp(to_join,rooms_joined[i]) == 0)    // If the requested room is found:
    {
      memset(current_room,0,16);
      memcpy(current_room,rooms_joined[i],16);     // Switch to room requested.
      cout << "Switched to room: " << current_room << endl;             // Let user know it was success.
      return 0;
    }

  }

      cout << "Couldn't join requested room. Check rooms joined and spelling." << endl;


  return 0;
}
  
/*
 * Function to list members of a currently joined room.
 */
int list_members(int socket)
{

  int rc = 0;                   // Return code variable.
  int i = 0;                    // Counter variable.
  char * temp = NULL;
  char * operation = NULL;      // Holds current operation.
  char * member_name = NULL;    // Holds name of member.
  char buf[1024] = {0};         // Buffer for networking.
  char op_code[6] = "80000";    // Operation code.
  char conf_code[6] = "80500";    // Operation code.
  char stop_code[6] = "80501";    // Operation code.

  strncpy(buf,op_code,6);       // Create message format for server.
  strcat(buf,"/");
  strcat(buf,current_room);
  send_msg(buf);
  memset(buf,0,BUF_MEM_SIZE);       // Reset buffer contents.

  do    // Receive each of the members one by one.
  {
    rc = 0;
    rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL); // Get message from server.
    if(rc ==  0)                                // Error checking.
    {
      perror("Server lost connection. Exiting application.");
      sleep(2);
      return 0;
    }
    
    if(rc <0)
    {
      return 1;
      perror("read");
    }
    temp = strtok(buf,"/");   // Split buffer.
    operation = temp;         
    if(rc > 0)          // If data was given:
    {
     if(strcmp(operation,conf_code) == 0)        // Check if server ACK is correct.
      {
        temp = strtok(NULL,"/");                // Split string to get member name.
        member_name = temp;
        ++i;
        cout << "MEMBER (" << i << "): " << member_name << endl;

      }
      else if(strcmp(operation,stop_code) == 0) 
        rc = 0;
    }
      memset(buf,0,BUF_MEM_SIZE);   // Reset buffer contents.

  }while(rc > 0);


  return 0;
}
/*
 * Function to list the rooms currently joined 
 * by the client.
 */
int list_current_rooms()
{

  for(int i =0; i < 10; ++i)    // Loop through number of rooms joined and display it.
  {
    if(rooms_joined[i] != NULL)
      cout << "member of:" << rooms_joined[i] << endl;

  }
  return 0;
}

/*
 * Function to create a private message for server
 * to send.
 */
int private_message(int socket)
{
  int rc = 0;                   // Return code variable.
  char buf[1023] = {0};         // Buffer for networking.
  char msg_temp[800] = {0};     // Contains message typed by user.        
  char to_receive[18] = {0};    // Username of user to receive message.
  char op_code[6] = "50500";    // Operation code for server.
  char conf_code[6] = "51500";  // Acknowledgement code.
  char * to_check = NULL;       // Variable to store acknowledge return.

  do
  {
    memset(to_receive,0,17);
    cout << "Enter users name." << endl;
    cin.getline(to_receive,17,'\n');       // Getting room name user desires.
   rc = search_str(to_receive);
  }while(rc != 0);
  do
  {
    memset(msg_temp,0,800);
    cout << "Please enter a message up to 800 characters" << endl;        // Prompt user to enter messages.
    cin.ignore(0,'\n');
    cin.getline(msg_temp,800,'\n');       // Populate buffer with input from user.
    //cin.ignore(100,'\n');
    rc = search_str(msg_temp);
  }while(rc != 0);

  strncpy(buf,op_code,6);               // Create message format for server.
  strcat(buf,"/");
  strcat(buf,current_room);
  strcat(buf,"/");
  strcat(buf,gl_username);
  strcat(buf,"/");
  strcat(buf,to_receive);
  eliminate_spaces(buf);
  strcat(buf,"/");
  strcat(buf,msg_temp);
  strcat(buf,"/");

  send_msg(buf);

  memset(buf,0,BUF_MEM_SIZE);
  rc = recv(socket,buf,BUF_SIZE,MSG_WAITALL);   // Get code returned from server.
  to_check = strtok(buf,"/");       // Parse buffer up to delimiter.

  if(rc ==  0)          // Error checking.
  {
    perror("Server lost connection. Exiting application.");
    sleep(2);
    return 1;
  }
  else if(rc < 0)
  {
    perror("Failed sending private message to user");
    return 1;
  }

  if(strcmp(to_check,conf_code) == 0)        // Display private message if success.
  {
    cout << ">>" << current_room << "/" << gl_username << ":" << endl;
    cout << "  >> [private]:" << msg_temp << endl;

  }
  else          // Prompt if fail.
  {
    cout << "Server failed sending message to user." << endl;
    return 1;
  }
  return 0;
}

int search_str(char * to_search)
{
  int len = strlen(to_search);

  for(int i = 0; i < len; ++i)
  {
    if(to_search[i] == '/')
      return 1;
  }

  return 0;
}

/*
 * FOLLOWING CODE REFERENCED FROM:
 * ://www.geeksforgeeks.org/remove-spaces-from-a-given-string/
 *
 * Function to remove whitespaces in a given string.
 *
 */
int eliminate_spaces(char * str)
{
  if(!str)              // Checking paramater value.
    return 1;

  int numel = strlen(str);
  int count = 0;

  for(int i =0; i < numel; ++i)    // Loop through string and ignore indexes containing space.
    if(str[i] != ' ')
    {
      str[count++] = str[i];
    }

  return 0;
}
