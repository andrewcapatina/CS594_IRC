/*
 *
 *
 * Andrew Capatina 
 * 5/1/2018
 *
 * Header file for IRC program.
 *
 *
 */

// Library includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h> 
#include <arpa/inet.h>  
#include <poll.h>

#define IP_PROTOCOL     0       // Specifying protocol for socket.
//#define PORT            22855    // Port number to bind.
#define PORT            5060    // Port number to bind.
#define MAX_CLIENTS     20      // Max number of clients waiting.
#define MAX_USERS       25

// Function with no paramaters and int return.
extern int create_server();

/*
 * Structure for chat room information
 */
struct chat_room
{
  char name[17];        // Name of chat room.
  int  members[MAX_USERS];       // Contains members of a chat room.
  chat_room * next;     // Pointer for linked list of rooms.
};

/*
 * Structure containing client information for server.
 */
struct client_info
{
  char username[17];            // Contains username for client.
  char peer_address[30];          // Contains IP address of peer.
  int peer_socket;              // Contains socket descriptor of peer.
  client_info * next;           // Pointer for linked list of peers.          
};

class server
{
  public:

    server();
    ~server();

    // Function prototype with struct argument. Integer return.
    int server_create(sockaddr_in6 & my_addr);

    // Function prototype with integer argument. Char pointer return.
    char * get_client_address(int client_socket);

    // Function protype with no arguments. Integer return.
    int create_client_info(char * address, int socket,char*buf);

    // Function with integer and char pointer paramaters. Integer return.
    int new_user_check(int socket, char * username);

    // Function with integer and char pointer paramaters. Integer return.
    int register_user(int socket, char * uname);

    // Function with integer and char pointer paramaters. Integer return.
    int create_room(int socket,char *payload);

    // Function with integer paramater. Integer return.
    int parse_request(int socket);

    // Function with integer paramater. Integer return.
    int client_disconnect(int socket);
    
    // Function with integer paramater. Integer return.
    int list_rooms(int socket);

    // Function with integer and char pointer paramaters. Integer return.
    int connect_to_room(int socket,char * room);

    // Function with integer and char pointer paramaters. Integer return.
    int relay_message(int socket, char* room,char * msg);

    int relay_m_private(int socket,char * room, char * sender,char * to_receive,char * msg);

    // Function with integer and char pointer paramaters. Integer return.
    int list_users(int socket,char* room);
    
    // Function with integer paramaters passed by reference. Integer return.
    int display_option(int & flag,int & prompt_flag);
    
    // Function with integer return. Pointer of structure chat_room type paramater.
    int remove_room(chat_room * to_delete);

    // Function to create a message for send function.
    int create_message(char * buf, char * op_code, char * room, char * sender,char * msg);
    /*
     * Data Structure Functions
     */
    // Function with integer return. Pointer of structure client_info as paramater.
    int add_client(client_info * source);

    // Function with integer return. Takes integer as paramter.
    int remove_client(int socket);

    // Function with integer return. Takes pointer of structure chat_room type as paramater.
    int add_chat(chat_room * source);

    // Return type is a pointer to client_info structure. Integer paramater.
    client_info * find_client(int socket);

    // Return type is a pointer to chat_room structure. Char pointer paramater.
    chat_room * find_room(char * room_name);

    // Integer return type. Integer paramater and char pointer arguments.
    int remove_from_room(int socket, char * current_room);

  private:

    chat_room * chat_head;      // Contains linked list of chat rooms.
    chat_room * chat_tail;      // Contains linked list of chat rooms.
    client_info * client_head;  // Contains linked list of clients.
    client_info * client_tail;  // Contains linked list of clients.
    char client_ip6[50];        // Temporarily contains client ip adress info.
};

