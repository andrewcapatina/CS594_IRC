/*
 *
 * Andrew Capatina
 * 5/8/2018
 *
 * Contains process of server.
 * Will serve client requests based off 
 * of RFC.
 *
 */

#include "header.h"

static int BUF_SIZE = 1023;

using namespace std;
int main()
{
  server s;                                     // Object for server class.
  int rc = 0;                                   // Return code variable.
  struct sockaddr_in6 my_addr, peer_addr;       // Pointer to sock address struct.
  int sock_fd = 0;                              // Socket descriptor.
  int peer_sock = 0;                            // Socket descriptor.
  socklen_t size_peer_addr;                     // Holds size of peer address structure size.
  fd_set rfd;                                   // Receive file descriptors, set of open sockets.
  fd_set c_rfd;                         // Receive file descriptors, set of open sockets.
  int fdsize;                           // Size of file descriptor table.
  int ns = 0;                           // New socket descriptor.

  sock_fd = s.server_create(my_addr);  // Calling function to create server.
  
  /*
   * Mark sockfd as passive, wait for requests.
   */
  rc = listen(sock_fd, MAX_CLIENTS);
  if(rc == -1)                          // Checking if listen function call failed.
    perror("Listening Failed");

  size_peer_addr = sizeof(peer_addr);

  fdsize = getdtablesize();             // Get size of file descriptor table.

  FD_ZERO(&rfd);                        // Initializing set of file descriptors.
  FD_SET(sock_fd,&rfd);                 // Adding server socket to set.
  int sel_rc = -1;
  int option = 0;                       // Used for server option selections.
  int flag = 0;                         // Used for prompting user.
  int prompt_flag = 0;                  // Used for prompting user.
  struct timeval tim = {0,10000};       // Populating poll structure.
  
  while(1)
  {

    c_rfd = rfd;

    sel_rc = select(fdsize+1,&c_rfd,NULL,NULL,&tim);    // Monitoring socket until operation is ready.
    rc = sel_rc;
    if(rc == -1)        // Error checking.
      perror("select");

    
    if(FD_ISSET(sock_fd,&c_rfd))        // Checking if server socket is part of set.
    {
      ns = accept(sock_fd,(struct sockaddr *)&peer_addr,&size_peer_addr); // Creating new socket after listening.
      if(ns <0) // Error check.
      {
        cout << "ns < 0" << endl;
      }
      if(ns < 0)
        continue;

      FD_SET(ns,&rfd);  // Adding new socket to set.

      continue;
    }

    for(int i = 0; i < fdsize; ++i)     // Loop through set of socket descriptors.
    {
      if(i != sock_fd && FD_ISSET(i,&c_rfd))    // Test if any of the sockets are ready for I/O operation.
      {
        rc = s.parse_request(i);        // Call function to parse information
        if(rc == 3 || rc == 15)         // If the client requests to disconnect or the connection is lost:
        {
          cout << "closing peer" << endl;       // Close the connection.
          close(i);
          FD_CLR(i,&rfd);
          rc = 0;
        }
      }
    }
      option = s.display_option(flag,prompt_flag);      // Calling function to take server admin input for server options.
      if(option == 1)   // Option to disconnect all clients.
      {
        for(int i = 0; i <= fdsize; ++i)        // Loop through set of descriptors.
        {
          if(i != sock_fd && i > 3) // Close all connections except listening socket.
          {
            if(s.find_client(i) != NULL) // Check if socket number is member of IRC
            {
              char buf[BUF_SIZE] = {0};         // Buffer to send to user.
              char op_code[6] = "11111";        // Operation code.
              strncpy(buf,op_code,6);           // Copy code to buffer.
              strcat(buf,"/");                  // Add app delimiter.
              cout << "Sending shutdown message to peer: " << i << endl;
              rc = send(i,buf,BUF_SIZE,0);      // Send acknowledgement to peer.
              if(rc < 0)                        // Error check.
                perror("Failed to send shutdown message to peer.");
              close(i);                 // Disconnect peer.
              FD_CLR(i,&rfd);

              }
            }
        }
        s.~server();
     }
  }


  cout << "exit" << endl;
  close(peer_sock);
  close(sock_fd);

  return 0;



}
