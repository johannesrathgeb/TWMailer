#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

#define PORT 6543

//convert to exe:
    // g++ client.cpp -o client.exe
//run exe
    // ./client.exe 196.0.0.1 80

//EINGABEFORMAT:
//./twmailer-client <ip> <port>
int main(int argc, char **argv){
    //Testausgabe
    std::cout << "Client" << std::endl;
    std::cout << argv[1] << std::endl;
    std::cout << argv[2] << std::endl;

    struct sockaddr_in address; //struct to work with addresses ports etc
    int create_socket;

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

      printf("Connection with server (%s) established\n",
          inet_ntoa(address.sin_addr));

}