#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 6543
#define BUF 1024

int create_socket;

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
        std::cout << ">> ";
        
        char input[BUF]; 
        std::string fullstring = ""; 

        //input content of email
        //TODO: input/error handling
        fgets(input, BUF, stdin);
        std::cout << "INPUT:" << input << std::endl; 
        
        if((std::string) input == "SEND\n") {
            fullstring = fullstring + input; 
            std::cout << "Sender:" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            fullstring = fullstring + input; 
            std::cout << "Receiver:" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            fullstring = fullstring + input;
            std::cout << "Subject:" << std::endl << ">> "; 
            fgets(input, BUF, stdin);
            fullstring = fullstring + input;
            std::cout << "Message:" << std::endl << ">> "; 
            
            while((std::string) fgets(input, BUF, stdin) != ".\n") {
                fullstring = fullstring + input;
            }
            
            strcpy(buffer, fullstring.c_str());
        }
        else if((std::string) input == "LIST\n"){
            fullstring = fullstring + input;
            std::cout << "Username:" << std::endl << ">> ";
            fgets(input, BUF, stdin);

            fullstring = fullstring + input;
            strcpy (buffer, fullstring.c_str());
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
            isQuit = strcmp(buffer, "quit") == 0;

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
                if (strcmp("OK", buffer) != 0)
                {
                    fprintf(stderr, "<< Server error occured, abort\n");
                    break;
                }
            }
        }
    } while (!isQuit);
}

//convert to exe:
    // g++ client.cpp -o client.exe
//run exe
    // ./client.exe 127.0.0.1 80

//EINGABEFORMAT:
//./twmailer-client <ip> <port>
int main(int argc, char **argv){
    struct sockaddr_in address; //struct to work with addresses ports etc
    int size, input;
    char buffer[BUF];

    //Socket Creation
    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) //socket of domain type IPv4(AF_INET), of type SOCK_STREAM(TCP reliable connection oriented) and automatically chosen protocol(0) ->returns -1 in case of errors
    {
        perror("Socket error");
        return EXIT_FAILURE;
    }


    //Address init
    memset(&address, 0, sizeof(address)); //sets each number from address to 0
    address.sin_family = AF_INET; //sets address "family" to IPv4 (AF_INET6 is for IPv6)
    address.sin_port = htons(PORT); //converts short integer from host byte to network byte order

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