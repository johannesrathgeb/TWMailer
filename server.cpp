#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 6543
#define BUF 1024
bool abortRequested = false;
int new_socket = -1;
int create_socket = -1;

void *clientCommunication(void *data)
{
    char buffer[BUF];
    int size;
    int *current_socket = (int *)data;

    strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
    if (send(*current_socket, buffer, strlen(buffer), 0) == -1)
    {
        perror("send failed");
        return NULL;
    }

    return NULL;
}

void signalHandler(int sig){
    if(sig == SIGINT){

    printf("abort Requested... "); // ignore error
      abortRequested = true;

      if (new_socket != -1)
      {
         if (shutdown(new_socket, SHUT_RDWR) == -1)
         {
            perror("shutdown new_socket");
         }
         if (close(new_socket) == -1)
         {
            perror("close new_socket");
         }
         new_socket = -1;
      }

      if (create_socket != -1)
      {
         if (shutdown(create_socket, SHUT_RDWR) == -1)
         {
            perror("shutdown create_socket");
         }
         if (close(create_socket) == -1)
         {
            perror("close create_socket");
         }
         create_socket = -1;
      }
   }
   else
   {
      exit(sig);
   }
}

//convert to exe:
    // g++ server.cpp -o server.exe
//run exe
    // ./server.exe 80 ...

//EINGABEFORMAT:
//./twmailer-server <port> <mail-spool-directoryname>
int main(void){
    //Testausgabe   
    std::cout << "Server" << std::endl;
    
    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;
    int reuseValue = 1;

    //interacative attention signal tested on errors
    if (signal(SIGINT, signalHandler) == SIG_ERR)
    {
        perror("signal can not be registered");
        return EXIT_FAILURE;
    }

    //Create socket
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) //socket of domain type IPv4(AF_INET), of type SOCK_STREAM(TCP reliable connection oriented) and automatically chosen protocol(0) ->returns -1 in case of errors
    {
        perror("Socket error");
        return EXIT_FAILURE;
    }

    //Socket options
    if (setsockopt(create_socket,
                    SOL_SOCKET, //Protocol Level SOL_SOCKET
                    SO_REUSEADDR, //Address thats already in use
                    &reuseValue,
                    sizeof(reuseValue)) == -1)
        {
        perror("set socket options - reuseAddr");
        return EXIT_FAILURE;
    }

   if (setsockopt(create_socket,
                    SOL_SOCKET, //Protocol Level SOL_SOCKET
                    SO_REUSEPORT, //Port thats already in use
                    &reuseValue,
                    sizeof(reuseValue)) == -1)
    {
        perror("set socket options - reusePort");
        return EXIT_FAILURE;
    }

    //set address options
    memset(&address, 0, sizeof(address)); //sets all bytes of address to 0
    address.sin_family = AF_INET; //sets address family to IPv4
    address.sin_addr.s_addr = INADDR_ANY; //binds to default IP address
    address.sin_port = htons(PORT); // sets Port in network byte order

    //makes port visible from outside
    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind error");
        return EXIT_FAILURE;
    }

    //Allow client connection checks if queue is full, return connection refused to client
    if (listen(create_socket, 5) == -1) // listen(create_socket, x)) x == count of queued connections allowed to wait for an accept
    {
        perror("listen error");
        return EXIT_FAILURE;
    }

    while (!abortRequested)
    {
        printf("Waiting for connections...\n");

        //accept connection from client address
        addrlen = sizeof(struct sockaddr_in);
        if ((new_socket = accept(create_socket,
                                (struct sockaddr *)&cliaddress,
                                &addrlen)) == -1)
        {
            if (abortRequested)
            {
                perror("accept error after aborted");
            }
            else
            {
                perror("accept error");
            }
            break;
        }

        printf("Client connected from %s:%d...\n",
            inet_ntoa(cliaddress.sin_addr), //client address in ASCII
            ntohs(cliaddress.sin_port)); //port in host byte order
        
        clientCommunication(&new_socket); // returnValue can be ignored
        new_socket = -1;
    }
}