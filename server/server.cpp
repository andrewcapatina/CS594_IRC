/*
 *
 * Andrew Capatina
 * CS594
 * 5/8/2018
 *
 *
 * Contains functions for server side of IRC application.
 *
 */

#include "header.h"

static int BUF_SIZE = 1023;
static int BUF_MEM_SIZE = 1023;

using namespace std;

// Class constructor.
server::server()
{
  chat_head = NULL;
  chat_tail = NULL;
  client_head = NULL;
  client_tail = NULL;
  client_ip6[50] = {};

}
// Class deconstructor.
server::~server()
{
  chat_room * temp = chat_head;
  while(chat_head != NULL)
  {
    chat_head = chat_head -> next;
    delete temp;
    temp = NULL;
    temp = chat_head;
  }
  client_info * temp_2 = client_head;

  while(client_head != NULL)
  {
    client_head = client_head -> next;
    delete temp_2;
    temp_2 = NULL;
    temp_2 = client_head;
  }


}

/*
 * Function to create server socket.
 */
int server::server_create(sockaddr_in6 & my_addr)
{
  int sock_fd = 0;      // Socket descriptor.
  int rc = -1;           // Return code variable.
  /*
   * Creating socket using IPv6, TCP, and IP protocol. TCP6 socket.
   */
  sock_fd = socket(AF_INET6,SOCK_STREAM,IP_PROTOCOL);

  if(sock_fd == -1)     // Checking if socket creation failed.
    perror("Server socket creation failed.");

  int buf_size = 1023;
  int enable = 1;

  if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int))< 0)        // Allows reuse of local addresses.
    perror("setsockopt");
  if(setsockopt(sock_fd,SOL_SOCKET,SO_SNDBUF,&buf_size,sizeof(buf_size)) < 0)   // Setting send buffer size.
    perror("setsockopt");
  if(setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,&buf_size,sizeof(buf_size))<0)     // Setting receive buffer size.
    perror("setsockopt");

  memset(&my_addr, 0, sizeof(struct sockaddr_in6));      // Setting structure members to zero. 
  my_addr.sin6_family = AF_INET6;       // Specifying IPv6 protocol.
  my_addr.sin6_port = htons(PORT);      // Assigning port, convert to network byte order.
  my_addr.sin6_addr = in6addr_any;      // Copying local address.

  /*
   * Assigning a name to the socket.
   */
  rc = bind(sock_fd,(struct sockaddr*) &my_addr,
      sizeof(my_addr));

  if(rc == -1)    // Checking if bind failed.
    perror("Server bind failed");

   return sock_fd;       // Returning socket descriptor for thread.

}

/*
 * Function to get IP address of peer.
 */
char * server::get_client_address(int client_socket)
{
  struct sockaddr_in6 addr;             // Pointer to sock address struct.
  socklen_t size_addr = sizeof(addr);   // Getting size of address structure.
  char * client_ip6= new char[50];      // Contains IP address.
  int rc = 0;                           // Return code variable.

  /*
   * Getting peer IP address. Need to store for identification.
   */
  rc = getpeername(client_socket,(struct sockaddr*) &addr,&size_addr);  // Populating addr structure with ipv6 address.
  if(rc < 0)
  {
    perror("Peer name");
    return 0;
  }

  inet_ntop(AF_INET6, &addr,client_ip6, 50);    // Converting ip address to binary.

  if(client_ip6 == NULL)       // Error checking.
  {
    perror("Null");
      return 0;
  }

  return client_ip6;    // Return ip address.
}

/*
 * 
 * Function to populate a client_info structure and add it to a linked list.
 */
int server::create_client_info(char * address, int socket,char * buf)
{
  client_info * temp = new client_info; // Create temporary pointer.

  strncpy(temp->username,buf,17);               // Copy username to temporary structure.
  int len = strlen(address);
  strncpy(temp->peer_address,address,len+1);    // Copying address of peer to temporary structure.
  temp->peer_socket = socket;                   // Copy socket number to structure.

  add_client(temp);                             // Add client to linked list.


  return 0;
}

// Function which creates linked list of clients.
// Node is passed in as argument.
int server::add_client(client_info * source)
{
  if(!client_head)      // Checking if linked list has been created.
  {
    client_head = source;       // Create first node.
    client_tail = source;
    client_tail->next = NULL;
  }
  else if(client_head)  // Add to end of list if list is created already.
  {
    client_tail->next = source; 
    client_tail = source;
  }

  cout << "Adding user: " << source->username << "socket number: " << source->peer_socket << endl;

  return 0;
}

/*
 * Function to remove a client from this list.
 */
int server::remove_client(int socket)
{
  client_info * current = client_head;  // For tracking pointers.
  client_info * previous = current;     // For tracking pointers.
  while(current != NULL)        // While we are not at the end of the list:
  {

    if(current->peer_socket == socket) // If match is found:
    {
      if(current != client_head && current != client_tail) // If we are somewhere in between the beginning and end of the list:
      {
        previous->next = current->next; // Set previous' pointer to the item after deleted object.
        cout << "deleting: " << current->peer_socket << " user: " << current->peer_address << endl;
        delete current; // Delete objects.
        current = NULL;
        return 0;
      }

      if(current == client_head)        // If we are at the beginning of the list:
      {
        client_head = current -> next;  // Set head to whatever is next in list.
        cout << "deleting: " << current->peer_socket << " user: " << current->peer_address << endl;
        delete current;         // Delete objects.
        current = NULL;
        delete previous;
        return 0;
      }
      
      if(current == client_tail)        // If we are at end of list:
      {
        previous -> next = NULL;        // Set the ends next pointer to null.
        cout << "deleting: " << current->peer_socket << " user: " << current->peer_address << endl;
        delete current;         // Delete objects.
        current = NULL;
        client_tail = previous;
        return 0;
      }

    }

    previous = current;                 // Set previous before current.
    current = current -> next;          // Traverse list.
  }

  return 1;  
}

/*
 * Function to check linked list of clients for new user.
 */
int server::new_user_check(int socket, char * username)
{
  if(!client_head)      // Check if list empty.
    return 0;   // Return fail.

  client_info * temp = client_head;     // Temporary pointer to list.

  while(temp != NULL)   // While we aren't at end of list:
  {
    if(temp->peer_socket == socket || strcmp(temp->username,username) == 0)     // Check if paramater matche 
    {
      return 1; // Return success.
    }

    temp = temp -> next;        // Traverse list.
  }
  return 0;     // Return fail.
}

/* 
 * Function that calls necessary functions to add user to system.
 * Acknowledges completion to user.
 */
int server::register_user(int socket_fd, char * uname)
{
  int socket = socket_fd;       // Socket to be serviced.
  int rc = 0;                   // Return code variable.
  char conf_code[1023] = "550";     // Operation code.

  if(new_user_check(socket,uname) == 1) // Check if user already exists.
  {
    cout << "Username: " << uname << " already connected." << endl;
    return 1;   // Return fail if true.
  }

  char * result = get_client_address(socket);      // Getting client IP address.
  if(result == NULL)    // Error checking.
  {
    cout << "Failed to get client address." << endl;
    return 0;
  }

  rc = create_client_info(result,socket,uname);       // Calling function to insert client in appropriate data structure.
  delete []result;

  rc = send(socket_fd,conf_code,BUF_SIZE,0);  // Sending confirmation message to client.
  if(rc < 0)
  {
    perror("Couldn't send user registration confirmation.");
    return 1;
  }

  return 0;
}

/*
 * Function which handles all client requests to the server.
 */
int server::parse_request(int socket)
{
  int rc = 0;                   // Return code variable.
  char buffer[1024] = {0};      // Buffer for sending data on network.
  char * temp = NULL;           // Temp pointer.
  char * params[12] = {NULL};   // Used to pass paramaters to functions.
  char * operation_temp = NULL; // Holds operation string.
  int operation;                // Holds numberical operation number.
  int i = 0;

  rc = recv(socket,buffer,BUF_SIZE,MSG_WAITALL); // Get client request.
  if(rc < 0)
  {
    perror("Couldn't read user request.");
    return 1;
  }

  if(rc == 0)              // Check if any data was sent.
  {
    perror("Client has lost connection");
    chat_room * temp = chat_head;
    while(temp != NULL)                 // Removing disconnectd socket from all rooms. 
    {
      for(int i = 0; i < MAX_USERS; ++i)
      {
        if(temp->members[i] == socket)
          temp->members[i] = 0;
      }
      temp = temp -> next;
    }
    remove_client(socket);
    return 15;

  }
  temp = strtok(buffer,"/");    // Parsing up to delimiter. 

  operation_temp = temp;        // Hold operation number to be converted.
  while(temp != NULL)           // Get all paramaters.
  {
    temp = strtok(NULL,"/");
    params[i] = temp;
    ++i;

  }

  operation = atoi(operation_temp);     // Convert operation into int.

  cout << "OPERATION: " << operation << endl;
  switch(operation)     // Select function based off of operation value.
  {
    case 500:   register_user(socket,params[0]);        // Call function to register user.
                break;
    case 10000: if(create_room(socket,params[0])==0)    // Call function to create room.
                  return 1;
                break;
    case 20000: if(list_rooms(socket) == 0) // Call function to list rooms.
                  return 2;  
                //else IMPLEMENT CASE WHERE THERE ARE NO ROOMS

                break;
    case 30000: if(client_disconnect(socket) == 0) return 3;    // Call function to disconnect client. 
                break;
    case 40000: connect_to_room(socket,params[0]);      // Call function to connect a user to a room.
                break;
    case 50000: relay_message(socket,params[0],params[1]);      // Call function to relay the client message.
                break;
    case 50500: relay_m_private(socket,params[0],params[1],params[2],params[3]);
                break;
    case 60000: remove_from_room(socket,params[0]);     // Call function to remove a chat room.
                break;
    case 80000: list_users(socket,params[0]);           // Call function to list all users of a room.
                break;
  }
  
  return 0;
}

/*
 * Function to create a new room. Populates chat room 
 * structure and adds the structure to linked list.
 */
int server::create_room(int socket, char * payload)
{
  int rc = 0;
  int fd = socket;      // socket to be serviced.
  char conf_code[6] = "10500";
  char buf[1023] = {0};
  chat_room * temp = new chat_room;     // Create new chat room structure.
  strncpy(temp->name,payload,17);       // Copy name of chat room.

  temp->members[0] = fd;
  cout << "Adding socket number to chat: " << temp -> members[0] << endl;

  add_chat(temp);       // Add to linked list.
  strncpy(buf,conf_code,6);
  strcat(buf,"/");
  rc = send(socket,buf,BUF_SIZE,0);   // Send success message to user.
  if(rc < 0)
  {
    perror("Failed to send room creation message to peer.");
    return 1;
  }

  return 0;
}

// Function which creates linked list of chat rooms.
// Node is passed in as argument.
int server::add_chat(chat_room * source)
{
  if(!chat_head)        // Checking if a chat has been created.
  {     
    chat_head = source; // If not, create the first item in list.
    chat_tail = source;
    chat_tail->next = NULL;
    chat_head->next = NULL;
  }
  else if(chat_head)    // Add to end of the list if there already is an item.
  {
    chat_tail->next = source; 
    chat_tail = source;
  }

  cout << "Adding chat room: " << source->name << endl;

  return 0;
}

/*
 * Function to find a clients structure 
 * based on socket number.
 */
client_info* server::find_client(int socket)
{
  if(!client_head)      // If the list is empty, there's nothing to find.
    return NULL;
  client_info * temp = client_head;     // Temporary pointer to list.
  while(temp != NULL)   // Search list.
  {
    if(temp->peer_socket == socket)     // If match found:
      return temp;                      // Return address.

    temp = temp->next;          // Traverse to next item.
  }
  
  return NULL;  // Return null if nothing found.
}

/*
 * Function to disconnect a client from the server.
 */
int server::client_disconnect(int socket)
{
  int rc = 0;
  char conf_code[1024] = "30500"; // Operation ACK code.

  rc = send(socket,conf_code,BUF_SIZE,0);  // Send acknowledgement to user.
  if(rc < 0)    // Error check.
  {
    perror("Failed to send disconnect acknowledgement to client");
    return 1;
  }

  cout << "Closing connection." << endl;

  close(socket);        // Close descriptor.

  remove_client(socket);    // Remove client from data structure.

  return 0;
}

/*
 * Function to traverse linked list of chat rooms
 * and send messages of all chat room names.
 */
int server::list_rooms(int socket)
{
  int rc = 0;                   // Return code variable.
  char stop[6] = "20250";       // Code which ends operation.
  char no_rooms_code[6] = "20300";
  char buf[1024] = {0};         // Buffer to use for networking.
  if(!chat_head)        // No rooms to list if there are none.
  {
    strncpy(buf,no_rooms_code,6);  // Copy stop message to buffer.
    strcat(buf,"/");      // Add delimiter.
    rc = send(socket,buf,BUF_SIZE,0);     // Send stop message to client.
    if(rc < 0)    // Error check.
    {
      perror("Failed to send room list acknowledgement");
      return 1;
    }
    return 0;
  }
  chat_room * temp = chat_head; // Temporary pointer to list.
  char conf_code[6] = "20500";    // ACK code.
  
  while(temp != NULL)   // Traverse the entire list.
  {

    strncpy(buf,conf_code,6);     // Copy operation code.
    strcat(buf,"/");            // Add delimiter.
    strcat(buf,temp->name);     // Add chat name to message.
    strcat(buf,"/");            // Add delimiter.
    rc = send(socket,buf,BUF_SIZE,0);   // Send room name to client.
    if(rc < 0)  // Error check.
    {
      perror("Failed to send room name to client.");
      return 1;
    }
    memset(buf,0,BUF_MEM_SIZE);     // Reset buffer.
    temp = temp -> next;        // Traverse list.
  }
  strncpy(buf,stop,6);  // Copy stop message to buffer.
  strcat(buf,"/");      // Add delimiter.
  rc = send(socket,buf,BUF_SIZE,0);     // Send stop message to client.
  if(rc < 0)    // Error check.
  {
    perror("Failed to send room list acknowledgement");
    return 1;
  }

  return 0;

}

/*
 * Function to connnect a client to any 
 * available room.
 */
int server::connect_to_room(int socket, char * room)
{
  int rc = 0;   // Return code variable.
  char conf_code[6] = "40500";    // Operation code.
  char fail_code[6] = "40250";    // Operation code.
  char buf[1024] = {0};         // Buffer for networking.
  chat_room * temp = NULL;      // Temporary pointer 
  temp = find_room(room);       // Find room requested by user.
  if(temp == NULL)              // If room isn't found, return.
  {
    cout << "Failed to connect client to room" << endl;
    return 1;
  }
  
  for(int i = 0; i < MAX_USERS; ++i)
  {
    if(temp->members[i] == socket)    // If match is found, user is already a member.
    {
      strncpy(buf,fail_code,6);
      strcat(buf,"/");
      rc = send(socket,buf,BUF_SIZE,0);
      if(rc < 0)
      {
        perror("Failed to send room member name to client");
        return 1;
      }

      return 1;
    }
    else if(temp->members[i] == 0)
    {
      temp->members[i] = socket;
      cout << "Connecting socket to room: " << temp->members[i] << endl;
      break;
    }
  }
  strncpy(buf,conf_code,6);       // Create acknowledgement message.
  strcat(buf,"/");
  rc = send(socket,buf,BUF_SIZE,0);     // Send acknowledgement message to client.
  if(rc < 0)
  {
    perror("Failed to send confirmation message to client."); 
    return 1;
  }


  return 0;

}

/*
 * Function to find room requested.
 * */
chat_room* server::find_room(char * room_name)
{
  if(!chat_head)        // If there's no list, there's no rooms.
    return NULL;
  chat_room * temp = chat_head; // Temporary pointer to list.
  while(temp != NULL)   // Traverse entire list.
  {
    if(strcmp(temp->name,room_name) == 0)       // Check list until same room name found.
      return temp;      // Return address of match.

    temp = temp -> next;        // Traverse list.
  }
  
  return NULL;  // Return null if nothing found.
}

/*
 * Function to relay client messages
 * between a given chat room.
 */
int server::relay_message(int socket, char * room,char*msg)
{
  if(!msg || !room)             // Checking if paramaters are not null.
    return 1;
  int rc = 0;                   // Return code variable.
  char buf[1023] = {0};         // Buffer for networking.
  char conf_code[6] = "50500";    // Operation code.
  char fail_code[6] = "50250";    // Operation code.
  client_info * sender = NULL;          // Pointer for sender. 
  chat_room * to_service = NULL;        // Pointer for room to send messages.
  to_service = find_room(room);         // Find the appropriate room.
  if(to_service == NULL)                // Error check.
    return 1;
 
  sender = find_client(socket);         // Find original sender.
  if(sender == NULL)            // Error checking.
  {     
    cout << "SENDER NULL" << endl;
    return 1;
  }

  create_message(buf,conf_code,sender->username,room,msg);      // Calling function to create a message for the user.

  for(int i =0; i < MAX_USERS; ++i)       // Loop through number of members.
  {
    if(to_service->members[i] != socket && to_service->members[i] > 3)  // Send message to all besides sender.
    {
      int fd = to_service->members[i];
      cout <<  "Sending to descriptor: " << fd << endl;
      rc = send(fd,buf,BUF_SIZE,0);
      if(rc < 0)        // Error checking.
      {
        memset(buf,0,BUF_MEM_SIZE);       // Letting sender know the send failed.
        strncpy(buf,fail_code,6);
        strcat(buf,"/");
        rc = send(socket,buf,BUF_SIZE,0);
        if(rc < 0)    // Error checking.
        {
          perror("Couldn't send message acknowledgement to client");
          return 1;
        }
        perror("Couldn't relay message to client");
      }
      
    }

  }
  memset(buf,0,BUF_MEM_SIZE);       // Creating acknowledgement message and sending to client.
  strncpy(buf,conf_code,6);
  strcat(buf,"/");
  rc = send(socket,buf,BUF_SIZE,0);
  if(rc < 0)    // Error checking.
  {
    perror("Couldn't send message acknowledgement to client");
    return 1;
  }
  
  return 0;
}

/*
 * Function to remove user from room.
 */
int server::remove_from_room(int socket, char * current_room)
{
  char buf[1024] = {0};         // Buffer for networking.
  char conf_code[6] = "60500";  // Operation code.
  int rc = 0;                   // Return code variable.

  chat_room * temp = find_room(current_room);   // Temp pointer for room.
  if(temp == NULL)              // Checking if room found.
    return 1;

  for(int i = 0; i < MAX_USERS; ++i)    // Loop through array containing socket numbers.
  {
    if(temp->members[i] == socket)
    {
      temp->members[i] = 0;             // If match is found, set the array index to zero to remove user.
      strncpy(buf,conf_code,6);       // Send acknowledgement message to client.
      strcat(buf,"/");
      rc = send(socket,buf,BUF_SIZE,0);
      if(rc < 0)    // Error check.
      {
        perror("Couldn't send room removal acknowledgement to peer.");
        return 1;
      }
  
      return 0;
    }
  }
 
  return 1;
}

/*
 * Function to send all users of a room
 * to the client.
 */
int server::list_users(int socket,char * room)
{
  int rc = 0;                           // Return code variable.
  char buf[1024] = {0};                 // Buffer for networking.
  char uname_temp[17] = {0};            // Username temp array.
  char conf_code[6] = "80500";          // ACK code.
  char stop_code[6] = "80501";          // ACK code.
  chat_room * temp_room = NULL;         // Poiner to match of room.
  client_info * temp_client = NULL;     // Pointer to client of a room.
  int fd_to_find = 0;                   // Socket descriptor client to find.

  temp_room = find_room(room);          // find room requested.  
  if(temp_room == NULL)                 // Error check.
    return 1;

  for(int i =0; i < MAX_USERS; ++i)       // Loop through number of members.
  {
    if(temp_room->members[i] != 0)      // Send all members of the room to client requesting it.
    {
      strncpy(buf,conf_code,6);           // Format message.
      strcat(buf,"/");
      fd_to_find = temp_room->members[i];
      temp_client = find_client(fd_to_find);            // Find client of room.
      memcpy(uname_temp,temp_client->username,17);      // Copy username.
      strcat(buf,uname_temp);                           // Add to buffer.
      rc = send(socket,buf,BUF_SIZE,MSG_CONFIRM);       // Send username to user.
      if(rc < 0)
      {
        perror("send");
        return 1;
      }
      memset(buf,0,BUF_MEM_SIZE);           // Reset buffer for next iteration.
    }
  }
  strncpy(buf,stop_code,6);           // Format message.
  strcat(buf,"/");
  rc = send(socket,buf,BUF_SIZE,MSG_CONFIRM);       // Send username to user.
    if(rc < 0)
    {
      perror("send");
      return 1;
    }
  return 0;
}

/*
 * Function to display user options.
 * Function will accept input from STDIN 
 * as well.
 */
int server::display_option(int & flag, int & prompt_flag)
{
  struct pollfd pfd = {STDIN_FILENO,POLLIN,0};  // Populating poll structure for monitoring events.   
  int rc = 0;           // Return code variable.
  int response = 0;     // Records user response.
  int idle = 50;        // Return code number for no activity.
  if(flag == 0)         // If flag is 0, display all server function options.
  {
    cout << "-----------SERVER OPTIONS----------" << endl;      // Display server options.
    cout << "Press 1 to disconnect from all clients." << endl;
    rc = poll(&pfd,1,1000);     // Check if input from STDIN is waiting to be read.
    if(rc == 1) // If true:
    {
      cin >> response;  // Read response.
      cin.ignore(100,'\n');
      prompt_flag = 1;  
    }
    if(rc == 0) // If not:
    {
      flag = 1;
      prompt_flag = 0;

      return idle;      // Remain idle.

    }
    flag = 1;           // Set flag so options aren't constantly displayed.
    prompt_flag = 0;   
  }
  else if(flag == 1)    // Menu without spamming user.
  {
    if(prompt_flag == 1)        // Let server admin know commands can be entered.
      cout << "Ready for server to accept commands. Enter 10 to display all commands." << endl;

    rc = poll(&pfd,1,1000);     // Check if input is waiting at STDIN.
    if(rc == 1)         // If so:
    {
      cin >> response;  // Read input.
      cin.ignore(100,'\n');
      prompt_flag = 1;
    }
    else if(rc == 0) // If no activity:
    {
      prompt_flag = 0;
      return idle;      // Remain idle.
    }

    if(response == 10)  
    {
      flag = 0;
      return 10;
    }
  }
  return response;      // Return admin response.
}
/*
 * Function to remove a client from this list.
 */
int server::remove_room(chat_room * to_delete)
{
  chat_room * current = chat_head;  // For tracking pointers.
  chat_room * previous = current;     // For tracking pointers.
  while(current != NULL)        // While we are not at the end of the list:
  {

    if(current == to_delete) // If match is found:
    {
      if(current != chat_head && current != chat_tail) // If we are somewhere in between the beginning and end of the list:
      {
        previous->next = current->next; // Set previous' pointer to the item after deleted object.
        cout << "deleting: " << current->name << endl;
        delete current; // Delete objects.
        current = NULL;
        return 0;
      }

      if(current == chat_head)        // If we are at the beginning of the list:
      {
        chat_head = current -> next;  // Set head to whatever is next in list.
        cout << "deleting: " << current->name << endl;
        delete current;         // Delete objects.
        current = NULL;
        delete previous;
        return 0;
      }
      
      if(current == chat_tail)        // If we are at end of list:
      {
        previous -> next = NULL;        // Set the ends next pointer to null.
        cout << "deleting: " << current-> name << " " << endl;
        delete current;         // Delete objects.
        current = NULL;
        return 0;
      }

    }

    previous = current;                 // Set previous before current.
    current = current -> next;          // Traverse list.
  }

  return 1;  
}

/*
 * Function to relay a message privately. Part of extra credit.
 *
 */
int server::relay_m_private(int socket,char * room_temp,char * sender ,char * to_receive,char * msg)
{
  if(!room_temp || !to_receive || !msg) // Check if the paramaters are null.
    return 1;
  int rc = 0;                   // Return code variable.
  char * room = room_temp;      // Temporary pointer to room.
  char buf[1023] = {0};         // Buffer for networking.
  char op_code[6] = "50505";    // Operation code.
  char conf_code[6] = "51500";  // Acknowledgement code variable.
  int fd = 0;                   // Socket descriptor.
  chat_room * room_ptr = NULL;  // Temporary chat room pointer.
  client_info * client = NULL;  // Temporary client information structure.

  create_message(buf,op_code,room_temp,sender,msg);

  room_ptr = find_room(room);   // Find room currently requested.

  for(int i = 0; i < MAX_USERS; ++i)       // Look through members of room.
  {     
    fd = room_ptr->members[i];          // Store socket descriptor from struct in local variable.
    if(i != 0 || i != socket)           // Send to intended receiver.
    {
      client = find_client(fd);         // Find client struct.
      if(strcmp(to_receive,client->username) == 0)      // Check that the username is the user we are looking for.
      {
        rc = send(fd,buf,BUF_SIZE,0);
        if(rc > 0)
        {
          memset(buf,0,BUF_MEM_SIZE);
          strncpy(buf,conf_code,6);       // Creating message format for receiver.
          strcat(buf,"/");
          rc = send(socket,buf,BUF_SIZE,0);
        }

        return 0;
      }

    }


  }
  

  return 1;
}

/*
 * Function to put together the appropriate message format for the client
 * application.
 */
int server::create_message(char *buf, char * op_code, char * room, char * sender,char * msg)
{

  strncpy(buf,op_code,6);       // Adding operation code to buffer.
  strcat(buf,"/");              // Adding delimiter.
  strcat(buf,room);             // Adding current chat room to buffer.
  strcat(buf,"/");              
  strcat(buf,sender);           // Adding who sent message to buffer.
  strcat(buf,"/");
  strcat(buf,msg);              // Adding message contents to buffer.
  strcat(buf,"/");

  return 0;
}
