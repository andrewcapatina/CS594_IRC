/*
 *
 * Andrew Capatina
 * 5/8/2018
 *
 * Main for IRC program.
 *
 */

#include "header.h"

static int thr_fd = 0;
using namespace std;
int main()
{
  pthread_t thr_id = 0;       // Thread ID for io thread.
  bool hi = true;
  int fd = 0;
  int rc = 0;
  int option = 0;
  int flag = 0;
 char buf[1024]={0};
  int rc_2 = 0;
  fd_set rfd;
  fd_set c_rfd;
  int dsize;
  int cs =0;
  FD_ZERO(&rfd);
  FD_SET(thr_fd,&rfd);
  dsize = getdtablesize();


  // ******************NEED TO IMPLEMENT A RECEIVE THREAD*******************

  //create_server();
  //pthread_create(&thr_id,NULL,server_thread, NULL);
  //usleep(500);
  fd = connect_to_server();
  thr_fd = fd;
//  pthread_create(&thr_id,NULL,io_thread,NULL);

  struct pollfd pfd = {fd,POLLIN,0};

 while(hi == true)
  {
      option = client_menu_begin(flag);
      if(option != 10)
        rc = select_init_function(fd,option);
      if(option != 50)
      {
        if(rc == 1) 
          return 0;
        else if(rc == 2) 
          cout << "You have not registered yet!" << endl;
        else if(rc == 3)
          cout << "You have already registered!" << endl;
      }
      rc = poll(&pfd,1,1000); 
      if(rc > 0)
      {
        int i = 0;
        int operation = 0;
        char * temp = NULL;
        char * operation_temp = NULL;
        char * params[10];
        char val[1024] = {0};
        socklen_t op_len = 1023;
        rc = recv(fd,buf,1023,MSG_WAITALL);
        if(rc ==  0)
        {
          perror("Server lost connection. Exiting application.");
          sleep(2);
          return 0;
        }

        temp = strtok(buf,"/");
        operation_temp = temp;
        while(temp != NULL)
        {
          temp = strtok(NULL,"/");
          params[i] = temp;
          ++i;
        }
        operation = atoi(operation_temp);

        switch(operation)
        {
          case 11111 : cout << "Server requested shutdown. Shuting down in 5 seconds" << endl; 
                       sleep(5);
                       close(fd);
                       return 0;
                       break;
          case 50500 : display_chat_message(params[0],params[1],params[2]);
                       break;
          case 50505 : display_priv_message(params[0],params[1],params[2]);
                       break;

        }
        memset(buf,0,1023);
        operation = 0;
        operation_temp = 0;
      
      }
  
  }

  return 0;

}

/*
 * Function that displays public messages in a readable format.
 */
int display_chat_message(char*sender,char*room,char*msg)
{

  cout << ">>" << room << "/" << sender << ":" << endl;
  cout << "  >>" << msg << endl;

  return 0;

}

/*
 * Function that displays private messages in a readable format.
 */
int display_priv_message(char*sender,char*room,char*msg)
{

  cout << ">>" << room << "/" << sender << ":" << endl;
  cout << "  >> [private]: " << msg << endl;

  return 0;

}
