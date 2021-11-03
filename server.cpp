#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>

#include <dirent.h>
#include <filesystem>
#include <sys/stat.h>

#define PORT 6543
#define BUF 1024

bool abortRequested = false;
int new_socket = -1;
int create_socket = -1;
//char buffer[BUF];

void saveToFile(char buffer[BUF]){

    DIR* dir; 
    struct dirent *entry; 
    bool folderexists = false; 

    std::cout << "HIER: " << buffer << std::endl;  

    std::string path = "messages";
    char* cpath = const_cast<char*>(path.c_str());
    
    char actualpath[PATH_MAX];

    realpath(cpath, actualpath);

    //std::cout << "actual path: " << actualpath << std::endl; 

    dir = opendir(actualpath);
    if(!dir) {
        std::cout << "Directory not found" << std::endl;
    }

    std::string receiver; 

    std::string fullstring = (std::string) buffer; 
    
    std::stringstream fullstringstream (fullstring); 


    getline(fullstringstream, receiver, '\n'); 
    getline(fullstringstream, receiver, '\n'); 
    getline(fullstringstream, receiver, '\n'); 

    fullstring.erase(0, 5);
    strcpy(buffer, fullstring.c_str());


    char* creceiver = const_cast<char*>(receiver.c_str());
    
    std::string userpath = (std::string) actualpath + "/" + receiver;
    char* cuserpath = const_cast<char*>(userpath.c_str());


    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if(entry->d_type == 4) {
                if(strcmp(entry->d_name,creceiver) == 0) {
                    //std::cout << "folder exists" << std::endl; 
                    folderexists = true; 
                    std::ofstream outfile (userpath + "/text.txt");
                    outfile << buffer << std::endl;
                    outfile.close();   
                } 
            }
        }
    }

    if(folderexists == false) {
        mkdir(cuserpath, 0777);

        std::ofstream outfile (userpath + "/index.txt");
        outfile << 0 << std::endl;
        outfile.close();   
    }

    closedir(dir);

   

    /*
    std::ofstream myFile;
    myFile.open("test.txt", std::ios::app);
    myFile << buffer << '\n';
    myFile.close();
    
    std::cout << "!!Saved to file!!" << std::endl;
    */
}

/*
char* recieveMessage(void *data){
    int size;
    int *current_socket = (int *)data;
    //char buffer[BUF];
    size = recv(*current_socket, buffer, BUF - 1, 0); //recieve message from socket and safe it to buffer
    if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
    {
        size -= 2;
    }
    else if (buffer[size - 1] == '\n')
    {
        --size;
    }

    if (send(*current_socket, "OK", 3, 0) == -1) //send recieved message to socket
    {
        perror("send answer failed");
        //return NULL;
    }
    return buffer;
}
*/
void clientSEND(void *data){
    int size;
    int *current_socket = (int *)data;
    //char buffer[BUF];
    char *returnValue;


    if (send(*current_socket, "OK", 3, 0) == -1) //send recieved message to socket
    {
        perror("send failed");
        //return NULL;
    }
    //returnValue = recieveMessage(current_socket);
    size = strlen(returnValue);
    returnValue[size] = '\0';
    std::string sender = returnValue;
    //saveToFile(buffer);
      
}

void *clientCommunication(void *data)
{
    char buffer[BUF];
    int size;
    int *current_socket = (int *)data;

    strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
    if (send(*current_socket, buffer, strlen(buffer), 0) == -1) // send message to socket
    {
        perror("send failed");
        return NULL;
    }

    do
    {
        size = recv(*current_socket, buffer, BUF - 1, 0); //recieve message from socket and safe it to buffer

        std::cout << "SERVERSIDE: " << buffer << std::endl;  

        if (size == -1)
        {
            if (abortRequested)
            {
                perror("recv error after aborted");
            }
            else
            {
                perror("recv error");
            }
            break;
        }

        if (size == 0)
        {
            std::cout << "Client closed remote socket" << std::endl;
            break;
        }

        // remove ugly debug message, because of the sent newline of client
        if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
        {
            size -= 2;
        }
        else if (buffer[size - 1] == '\n')
        {
            --size;
        }

        //anker
        //std::cout << "SERVERSIDE BUFFER: " << buffer << std::endl;

        buffer[size] = '\0';


        std::string command = (std::string) buffer; 
        char temp[BUF]; 
        command = command.substr(0,command.find('\n'));
        strcpy(temp, command.c_str());

        //std::cout << "test---- " << buffer[0] << " " << selection << std::endl; 

        switch(buffer[0]) {
            case 'S':
                std::cout << "SEND COMMAND" << std::endl; 
                saveToFile(buffer); 
                break; 
            case 'Q': 
                std::cout << "QUIT COMMAND" << std::endl; 
                break; 
            default:
                std::cout << "INPUT ERROR" << std::endl; 
                break; 
        }

        if(strcmp(temp, "SEND") == 0){
            clientSEND(current_socket);
        }
        else if(strcmp(buffer, "QUIT") == 0){
            abortRequested = true;
        }



        
        //std::cout << "Message received: " << buffer << std::endl;
        //saveToFile(buffer);

        if (send(*current_socket, "OK", 3, 0) == -1) //send recieved message to socket
        {
            perror("send answer failed");
            return NULL;
        }
    } while (strcmp(buffer, "QUIT") != 0 && !abortRequested);

    // closes/frees the descriptor if not already
    if (*current_socket != -1)
    {
        if (shutdown(*current_socket, SHUT_RDWR) == -1)
        {
            perror("shutdown new_socket");
        }
        if (close(*current_socket) == -1)
        {
            perror("close new_socket");
        }
        *current_socket = -1;
    }

    return NULL;
}

void signalHandler(int sig){
    if(sig == SIGINT){
    std::cout << "abort Requested..." << std::endl;
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

//?????????EINGABEFORMAT:
//./twmailer-server <port> <mail-spool-directoryname>
int main(void){    
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
        std::cout << "Waiting for connections..." << std::endl;

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
    //                                      client address in ASCII                     port in host byte order
        std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << "..." << std::endl;
        clientCommunication(&new_socket); // returnValue can be ignored
        new_socket = -1;
    }
}