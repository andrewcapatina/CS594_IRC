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


// Function with int return and arguments.
extern int select_init_function(int fd, int operation);

// Function with int return and int pass by reference.
extern int client_menu_begin(int & flag);

// Function with int return and character pointer argument.
extern int search_str(char * to_search);

// Function with int return and no arguments.
extern int connect_to_server();

// Functon with int return and no arguments.
extern int create_room();

// Function with int return and character pointer argument.
extern int send_msg(char*buf);

// Function with int return and argument.
extern int register_uname(int fd);

// Function with integer return and character array arguments.
extern int display_chat_message(char*sender,char*room,char*msg); 

// Function with integer return and character array arguments.
extern int display_priv_message(char*sender,char*room,char*msg);

// Integer return with integer argument.
int disconnect(int socket);

// Integer return with integer argument.
int get_rooms(int socket);

// Integer return with integer argument.
int join_room(int socket);

// Integer return with integer argument.
int send_msg_to_server(int socket);

// Integer return with integer argument.
int leave_room(int socket);

// Integer return type.
int switch_rooms();

// Function with integer return and integer argument.
int list_members(int socket);
// Integer return with integer argument.
int private_message(int socket);

// Function to eliminate spaces in character array argument.
int eliminate_spaces(char * str);
