#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF 1024

int create_socket;
bool loggedin = false; 

bool charValidation(char input[BUF]) {

    for(int i = 0; i < (int)strlen(input) - 1; i++) {
        
        if(islower(input[i]) || ((int) input[i] - '0' >= 0 && (int) input[i] - '0' <= 9 )) {
        } else {
            return false; 
        }
    }
    return true; 
}

bool checkIfNumber(char input[BUF]) {
    
    for(int i = 0; i < (int)strlen(input) - 1; i++) {

        if((int) input[i] - '0' >= 0 && (int) input[i] - '0' <= 9 ) {
        } else {
            return false; 
        }
    }
    return true; 
}



void communicateWithServer(){
    int size;
    char buffer[BUF];
    bool isQuit;

    //Recieve data from socket
    size = recv(create_socket, buffer, BUF - 1, 0); //reads BUF-1 bytes from socket to BUF
    if (size == -1)
    {
        perror("recv error");
    }
    else if (size == 0)
    {
        std::cout << "Server closed remote socket" << std::endl;
    }
    else
    {
        buffer[size] = '\0';
        std::cout << buffer;
    }

        //communication with server
    do
    {
        if(loggedin){
            std::cout << "<SEND|LIST|READ|DEL|QUIT|LOGIN>" << std::endl; 
        }
        else{
            std::cout << "<LOGIN|QUIT>" << std::endl; 
        }
        //TEST
        std::cout << ">> ";
        
        char input[BUF]; 
        std::string input_as_string; 
        std::string fullstring = ""; 

        //input content of email
        //TODO: input/error handling
        
        bool exitloop = false; 
        bool waitForOk = false;

        do {
            fgets(input, BUF, stdin);
            std::cout << "INPUT:" << input << std::endl; 
            input_as_string = (std::string) input; 
            if((input_as_string == "SEND\n" || input_as_string == "LIST\n" || input_as_string == "READ\n" || input_as_string == "DEL\n" || input_as_string == "QUIT\n" || input_as_string == "LOGIN\n") && loggedin == true) {
                exitloop = true; 
            } else if((input_as_string == "LOGIN\n" || input_as_string == "QUIT\n") && loggedin == false) {
                exitloop = true; 
            } else {
                std::cout << "Invalid Input. <SEND|LIST|READ|DEL|QUIT|LOGIN>" << std::endl; 
                std::cout << "if not logged in: <LOGIN|QUIT>" << std::endl; 
            }

        } while(exitloop == false);
        waitForOk = true;
        if((std::string) input == "SEND\n") {
            fullstring = fullstring + input; 

            do {
                std::cout << "Sender(max. 8 characters [a-z][0-9]):" << std::endl << ">> "; 
                fgets(input, BUF, stdin);
            } while(strlen(input) > 9 || !charValidation(input)); //anker
            fullstring = fullstring + input; 

            do {
                std::cout << "Receiver(max. 8 characters [a-z][0-9]):" << std::endl << ">> "; 
                fgets(input, BUF, stdin);
            } while(strlen(input) > 9 || !charValidation(input));
            fullstring = fullstring + input;

            do {
                std::cout << "Subject(max. 80 characters):" << std::endl << ">> "; 
                fgets(input, BUF, stdin);
            } while(strlen(input) > 81);
            fullstring = fullstring + input;

            std::cout << "Message:" << std::endl << ">> "; 
            while((std::string) fgets(input, BUF, stdin) != ".\n") {
                fullstring = fullstring + input;
            }
            
            strcpy(buffer, fullstring.c_str());
        }
        else if((std::string) input == "READ\n"){
            fullstring = fullstring + input;
            std::cout << "Username:" << std::endl << ">> ";
            fgets(input, BUF, stdin);
            fullstring = fullstring + input;
            std::cout << "Message Number:" << std::endl << ">> ";
            fgets(input, BUF, stdin);
            fullstring = fullstring + input;
            strcpy (buffer, fullstring.c_str());
            waitForOk = false;
        }
        else if((std::string) input == "LIST\n"){
            fullstring = fullstring + input;

            
            std::cout << "Username:" << std::endl << ">> ";
            fgets(input, BUF, stdin);

            fullstring = fullstring + input;
            strcpy (buffer, fullstring.c_str());
            waitForOk = false;
        } 
        else if((std::string) input == "DEL\n") { 
            fullstring = fullstring + input; 
            std::cout << "Username:" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            fullstring = fullstring + input; 
            
            do {
            std::cout << "Message Number:" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            } while(!checkIfNumber(input));

            fullstring = fullstring + input; 
            
            strcpy(buffer, fullstring.c_str());
            waitForOk = false;
        }
        else if((std::string) input == "QUIT\n"){
            strcpy(buffer, input);
        } else if((std::string) input == "LOGIN\n"){
           fullstring = fullstring + "POGIN\n"; 

            std::cout << "username" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            fullstring = fullstring + input; 

            std::cout << "password" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            fullstring = fullstring + input; 

            std::cout << fullstring << std::endl; 
            strcpy(buffer, fullstring.c_str());
            waitForOk = false;
        }



        if (buffer != NULL) //input, saved to buffer with maximum length of BUF
        {
            int size = strlen(buffer);
            
            // remove new-line signs from string at the end
            if (buffer[size - 2] == '\r' && buffer[size - 1] == '\n')
            {
                size -= 2;
                buffer[size] = 0;
            }
            else if (buffer[size - 1] == '\n')
            {
                --size;
                buffer[size] = 0;
            }
            isQuit = strcmp(buffer, "QUIT") == 0;

            if ((send(create_socket, buffer, size, 0)) == -1) 
            {
                perror("send error");
                break;
            }

            size = recv(create_socket, buffer, BUF - 1, 0);
            if (size == -1)
            {
                perror("recv error");
                break;
            }
            else if (size == 0)
            {
                std::cout << "Server closed remote socket" << std::endl;
                break;
            }
            else
            {
                buffer[size] = '\0';
                std::cout << "<< " << buffer << std::endl;
                if(waitForOk){
                    if (strcmp("OK", buffer) != 0)
                    {
                        fprintf(stderr, "<< Server error occured, abort\n");
                        break;
                    }
                }
                else if(strcmp("LOGIN SUCCESSFUL", buffer) == 0){
                    loggedin = true;
                }
                
            }
            memset(buffer, 0, strlen(buffer));
        }
    } while (!isQuit);
}

// "make" to compile the program

//run file: 
// ./client 127.0.0.1 80


int main(int argc, char **argv){
    struct sockaddr_in address; //struct to work with addresses ports etc
    int port =  atoi(argv[2]);

    //Socket Creation
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) //socket of domain type IPv4(AF_INET), of type SOCK_STREAM(TCP reliable connection oriented) and automatically chosen protocol(0) ->returns -1 in case of errors
    {
        perror("Socket error");
        return EXIT_FAILURE;
    }


    //Address init
    memset(&address, 0, sizeof(address)); //sets each number from address to 0
    address.sin_family = AF_INET; //sets address "family" to IPv4 (AF_INET6 is for IPv6)
    address.sin_port = htons(port); //converts short integer from host byte to network byte order

    if (argc < 2)
    {
        inet_aton("127.0.0.1", &address.sin_addr); //converts integer and dot notation to binary data (no address passed, so default ip set)
    }
    else
    {
        inet_aton(argv[1], &address.sin_addr); //converts integer and dot notation to binary data
    }
    

    //Create connection
    if (connect(create_socket,(struct sockaddr *)&address, sizeof(address)) == -1) //Opens connection on socket with address
   {
      perror("Connect error - no server available");
      return EXIT_FAILURE;
   }

    std::cout << "Connection with server " << inet_ntoa(address.sin_addr) <<" established" << std::endl;

    communicateWithServer();

       // CLOSES THE DESCRIPTOR
    if (create_socket != -1)
    {
        if (shutdown(create_socket, SHUT_RDWR) == -1)
        {
            // invalid in case the server is gone already
            perror("shutdown create_socket"); 
        }
        if (close(create_socket) == -1)
        {
            perror("close create_socket");
        }
        create_socket = -1;
    }
}
