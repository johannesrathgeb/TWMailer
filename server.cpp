#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <filesystem>
#include <sys/stat.h>
#include <algorithm>

#define PORT 6543
#define BUF 1024

bool abortRequested = false;
int new_socket = -1;
int create_socket = -1;
char buffer[BUF];
std::string dirName;

void listCommand(char buffer[BUF], void *data){
    int *current_socket = (int *) data;
    int fileCounter = 0;
    std::string fullList;
    DIR* dir;
    struct dirent *entry;
    char* cpath = const_cast<char*>(dirName.c_str()); //convert path to char* in order to work for realpath()
    
    char actualpath[PATH_MAX];

    realpath(cpath, actualpath); //absolute path for creation of user folders

    std::string user; //receiver of msg

    std::string fullstring = (std::string) buffer; //buffer with full message in string version 
    
    std::stringstream fullstringstream (fullstring); //as stringstream for getline
    getline(fullstringstream, user, '\n'); 
    getline(fullstringstream, user, '\n'); 

    fullstring.erase(0, 5); //erase LIST because unnecessary in email
    strcpy(buffer, fullstring.c_str()); //update buffer
    
    std::string userpath = (std::string) actualpath + "/" + user;
    char* cuserpath = const_cast<char*>(userpath.c_str());

    dir = opendir(cuserpath);
    if(!dir) {
        std::cout << "Directory not found" << std::endl;
    }


    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            fileCounter++;
        }
    }
    fileCounter--;
    fullList = std::to_string(fileCounter) + " message(s)"+ '\n';
    rewinddir(dir);

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp (entry->d_name, "index.txt") !=0){
            std::string name = entry->d_name;
            name.erase(std::remove(name.begin(), name.end(), '.'), name.end());
            name.erase(std::remove(name.begin(), name.end(), 't'), name.end());
            name.erase(std::remove(name.begin(), name.end(), 'x'), name.end());
            std::fstream file(userpath + "/" + entry->d_name);
            std::string str; 
            std::getline(file, str);
            std::getline(file, str);
            std::getline(file, str);
            fullList.append(name + ": " + str + '\n');
            file.close(); 
        }
    }
    if (send(*current_socket, fullList.c_str(), strlen(fullList.c_str()), 0) == -1) //send recieved message to socket
    {
        perror("send answer failed");
        //return NULL;
    }
    memset(buffer, 0, strlen(buffer));
}

void readCommand(char buffer[BUF], void *data){
    int *current_socket = (int *) data;
    DIR* dir;
    std::string fullMessage;
    char* cpath = const_cast<char*>(dirName.c_str()); //convert path to char* in order to work for realpath()
    
    char actualpath[PATH_MAX];

    realpath(cpath, actualpath); //absolute path for creation of user folders

    std::string user, messageNumber; //receiver of msg

    std::string fullstring = (std::string) buffer; //buffer with full message in string version 
    
    std::stringstream fullstringstream (fullstring); //as stringstream for getline
    getline(fullstringstream, user, '\n'); 
    getline(fullstringstream, user, '\n');
    getline(fullstringstream, messageNumber, '\n'); 

    fullstring.erase(0, 5); //erase LIST because unnecessary in email
    strcpy(buffer, fullstring.c_str()); //update buffer
    
    std::string userpath = (std::string) actualpath + "/" + user;
    char* cuserpath = const_cast<char*>(userpath.c_str());

    dir = opendir(cuserpath);
    if(!dir) {
        std::cout << "Directory not found" << std::endl;
    }

    std::fstream file(userpath + "/" + messageNumber + ".txt");
    if(file.is_open()){
        std::string sender, receiver, subject, message, tmp; 
        std::getline(file, sender);
        std::getline(file, receiver);
        std::getline(file, subject);
        while(std::getline(file, tmp)){
            message.append(tmp + '\n');
        }

        //fullMessage = "OK" + '\n';
        fullMessage.append("Sender: "+sender + '\n'+  "Reciever: "+ receiver + '\n'+ "Subject: " + subject + '\n'+ "Message: " + message + '\n');
        file.close(); 
    }



    if (send(*current_socket, fullMessage.c_str(), strlen(fullMessage.c_str()), 0) == -1) //send recieved message to socket
    {
        perror("send answer failed");
        //return NULL;
    }
    memset(buffer, 0, strlen(buffer)); 
}

void deleteCommand(char buffer[BUF]){ 
    DIR* dir; 
    struct dirent *entry; 

    bool folderexists = false; 

    std::string username;
    std::string messageid; 
    char* cpath = const_cast<char*>(dirName.c_str()); //convert path to char* in order to work for realpath()
    
    char actualpath[PATH_MAX];

    realpath(cpath, actualpath); //absolute path


    dir = opendir(actualpath);
    if(!dir) {
        std::cout << "Directory not found" << std::endl;
    }

    std::string fullstring = (std::string) buffer; //buffer with full message in string version 
    
    std::stringstream fullstringstream (fullstring); //as stringstream for getline

    getline(fullstringstream, username, '\n'); 
    getline(fullstringstream, username, '\n'); //gets username

    getline(fullstringstream, messageid, '\n'); //gets message id

    fullstring.erase(0, 4); //erase DEL because not needed any more
    strcpy(buffer, fullstring.c_str()); //update buffer

    char* cusername = const_cast<char*>(username.c_str());

    std::string usernamepath = (std::string) actualpath + "/" + username;

    std::string messageidpath = usernamepath + "/" + messageid + ".txt"; 
    char* cmessageidpath = const_cast<char*>(messageidpath.c_str());
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if(entry->d_type == 4) { //check if file is folder
                if(strcmp(entry->d_name,cusername) == 0) { //check if folder exists
                    folderexists = true; 
                    
                    if(remove(cmessageidpath) != 0) { //remove selected email
                        std::cout << "This message ID does not exist" << std::endl; 
                    } else {
                        std::cout << "File successfully deleted" << std::endl; 
                    }
                } 
            }
        }
    }
    if(folderexists == false) { 
        std::cout << "This user does not exist. The file cannot be deleted" << std::endl; 
    }
}

void sendCommand(char buffer[BUF]){

    DIR* dir; 
    struct dirent *entry; 

    bool folderexists = false; 

    char* cpath = const_cast<char*>(dirName.c_str()); //convert path to char* in order to work for realpath()
    
    char actualpath[PATH_MAX];

    realpath(cpath, actualpath); //absolute path for creation of user folders


    dir = opendir(actualpath);
    if(!dir) {
        std::cout << "Directory not found" << std::endl;
    }

    std::string receiver; //receiver of msg

    std::string fullstring = (std::string) buffer; //buffer with full message in string version 
    
    std::stringstream fullstringstream (fullstring); //as stringstream for getline


    getline(fullstringstream, receiver, '\n'); 
    getline(fullstringstream, receiver, '\n'); 
    getline(fullstringstream, receiver, '\n'); 

    fullstring.erase(0, 5); //erase SEND because unnecessary in email
    strcpy(buffer, fullstring.c_str()); //update buffer


    char* creceiver = const_cast<char*>(receiver.c_str());
    
    std::string receiverpath = (std::string) actualpath + "/" + receiver;
    char* creceiverpath = const_cast<char*>(receiverpath.c_str());


    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if(entry->d_type == 4) { //check if file is folder
                if(strcmp(entry->d_name,creceiver) == 0) { //check if folder exists
                    folderexists = true; 
                    
                    //increment int of index file by one to keep track of received emails
                    std::fstream file(receiverpath + "/index.txt");
                    std::string str; 
                    std::getline(file, str);
                    int msgnumber = std::stoi(str);
                    msgnumber += 1; 
                    str = std::to_string(msgnumber);
                    file.close(); 

                    std::ofstream file2(receiverpath + "/index.txt", std::ofstream::trunc);
                    file2 << str << '\n';
                    file2.close(); 

                    //create file for new email
                    std::ofstream outfile (receiverpath + "/" + str + ".txt");
                    outfile << buffer << std::endl;
                    outfile.close();   
                } 
            }
        }
    }

    if(folderexists == false) {
        
        mkdir(creceiverpath, 0777); //create directory (0777 = file permission in linux)

        //create new index
        std::ofstream outfile (receiverpath + "/index.txt");
        outfile << 0 << std::endl;
        outfile.close();   

        //create first file
        std::ofstream outfile2 (receiverpath + "/0.txt");
        outfile2 << buffer << std::endl;
        outfile2.close();   
    }

    closedir(dir);

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


        buffer[size] = '\0';


        std::string command = (std::string) buffer; 
        char temp[BUF]; 

        //stringsplit to get command (SEND, LIST, etc.)
        command = command.substr(0,command.find('\n'));
        strcpy(temp, command.c_str());
        
        //switch for commands
        switch(buffer[0]) {
            case 'S':
                sendCommand(buffer); 
                if (send(*current_socket, "OK", 3, 0) == -1) //send recieved message to socket
                {
                    perror("send answer failed");
                    return NULL;
                }
                break; 
            case 'R':
                readCommand(buffer, current_socket);
                break;
            case 'L':
                listCommand(buffer, current_socket);
                break;
            case 'D':
                deleteCommand(buffer);
                if (send(*current_socket, "OK", 3, 0) == -1) //send recieved message to socket
                {
                    perror("send answer failed");
                    return NULL;
                }
                break;
            case 'Q': 
                abortRequested = true;
                break; 
            default:
                std::cout << "INPUT ERROR" << std::endl; //should never happen due to clientside input validation
                break; 
        }
        memset(buffer, 0, strlen(buffer));
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

// "make" to compile the program
    
//run file: 
//./server 80 messages

int main(int argc, char **argv){    
    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;
    int reuseValue = 1;
    dirName = argv[2];

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