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
#include <sys/wait.h>
#include <ldap.h>

#define BUF 1024

bool abortRequested = false;
int new_socket = -1;
int create_socket = -1;
char buffer[BUF];
std::string dirName;
std::string loggedinname = ""; 

std::string ldapCommand(char buffer[BUF], void *data) {
    
    //int *current_socket = (int *) data;

    std::string fullstring = (std::string) buffer;

    fullstring.erase(0, 6); //erase SEND because unnecessary in email
    
    std::stringstream fullstringstream (fullstring); //as stringstream for getline


    std::string uname;
    getline(fullstringstream, uname, '\n'); 
    
    std::string pw;
    getline(fullstringstream, pw, '\n'); 
    
            ////////////////////////////////////////////////////////////////////////////
   // LDAP config
   // anonymous bind with user and pw empty
   const char *ldapUri = "ldap://ldap.technikum-wien.at:389";
   const int ldapVersion = LDAP_VERSION3;

   // recv username
   char ldapBindUser[256];
   char rawLdapUser[128];
   
    /*FETTY!!!
    Hier musst du den User ("if20bxxx") übergeben oder empfangen und mit "strcpy" auf den "rawLdapUser" kopieren
    */

    strcpy(rawLdapUser,uname.c_str());

    sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", rawLdapUser);
    printf("user set to: %s\n", ldapBindUser);

   
   char ldapBindPassword[256];
    /*FETTY!!!
    Hier musst du das Password übergeben oder empfangen und mit "strcpy" auf "ldapBindPassword" kopieren
    */
   strcpy(ldapBindPassword,pw.c_str());

   // general
   int rc = 0; // return code

   ////////////////////////////////////////////////////////////////////////////
   // setup LDAP connection
   // https://linux.die.net/man/3/ldap_initialize
   LDAP *ldapHandle;
   rc = ldap_initialize(&ldapHandle, ldapUri);
   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr, "ldap_init failed\n");
      return "error";
   }
   printf("connected to LDAP server %s\n", ldapUri);

   ////////////////////////////////////////////////////////////////////////////
   // set verison options
   // https://linux.die.net/man/3/ldap_set_option
   rc = ldap_set_option(
       ldapHandle,
       LDAP_OPT_PROTOCOL_VERSION, // OPTION
       &ldapVersion);             // IN-Value
   if (rc != LDAP_OPT_SUCCESS)
   {
      // https://www.openldap.org/software/man.cgi?query=ldap_err2string&sektion=3&apropos=0&manpath=OpenLDAP+2.4-Release
      fprintf(stderr, "ldap_set_option(PROTOCOL_VERSION): %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      return "error";
   }

   ////////////////////////////////////////////////////////////////////////////
   // start connection secure (initialize TLS)
   // https://linux.die.net/man/3/ldap_start_tls_s
   // int ldap_start_tls_s(LDAP *ld,
   //                      LDAPControl **serverctrls,
   //                      LDAPControl **clientctrls);
   // https://linux.die.net/man/3/ldap
   // https://docs.oracle.com/cd/E19957-01/817-6707/controls.html
   //    The LDAPv3, as documented in RFC 2251 - Lightweight Directory Access
   //    Protocol (v3) (http://www.faqs.org/rfcs/rfc2251.html), allows clients
   //    and servers to use controls as a mechanism for extending an LDAP
   //    operation. A control is a way to specify additional information as
   //    part of a request and a response. For example, a client can send a
   //    control to a server as part of a search request to indicate that the
   //    server should sort the search results before sending the results back
   //    to the client.
   rc = ldap_start_tls_s(
       ldapHandle,
       NULL,
       NULL);
   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr, "ldap_start_tls_s(): %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      return "error";
   }

   ////////////////////////////////////////////////////////////////////////////
   // bind credentials
   // https://linux.die.net/man/3/lber-types
   // SASL (Simple Authentication and Security Layer)
   // https://linux.die.net/man/3/ldap_sasl_bind_s
   // int ldap_sasl_bind_s(
   //       LDAP *ld,
   //       const char *dn,
   //       const char *mechanism,
   //       struct berval *cred,
   //       LDAPControl *sctrls[],
   //       LDAPControl *cctrls[],
   //       struct berval **servercredp);

   BerValue bindCredentials;
   bindCredentials.bv_val = (char *)ldapBindPassword;
   bindCredentials.bv_len = strlen(ldapBindPassword);
   BerValue *servercredp; // server's credentials
   rc = ldap_sasl_bind_s(
       ldapHandle,
       ldapBindUser,
       LDAP_SASL_SIMPLE,
       &bindCredentials,
       NULL,
       NULL,
       &servercredp);
   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr, "LDAP bind error: %s\n", ldap_err2string(rc));
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      return "error";
   } 

   std::cout << "RICHTIGE DATEN" << std::endl; 
   return uname; 
   /*FETTY!!!
   wenn du hier angekommen bist dann war dein LDAP bind erfolgreich und du kannst den Login abschließen und den User speichern für den "SEND" command
   */
  
}

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
        if (send(*current_socket, "0 message(s)", 13, 0) == -1) //send recieved message to socket
        {
            perror("send answer failed");
        }
        return;
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
            //remove .txt
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
        if (send(*current_socket, "ERR\n", 5, 0) == -1) //send recieved message to socket
        {
            perror("send answer failed");
        }
        return;
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
    else{
        std::cout << "Message not found" << std::endl;
        if (send(*current_socket, "ERR\n", 5, 0) == -1) //send recieved message to socket
        {
            perror("send answer failed");
        }
        return;
    }


    if (send(*current_socket, fullMessage.c_str(), strlen(fullMessage.c_str()), 0) == -1) //send recieved message to socket
    {
        perror("send answer failed");
        //return NULL;
    }
    memset(buffer, 0, strlen(buffer)); 
}

void deleteCommand(char buffer[BUF], void *data){ 
    int *current_socket = (int *) data;
    DIR* dir; 
    struct dirent *entry; 

    bool deleted = false; 

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
                    
                    if(remove(cmessageidpath) != 0) { //remove selected email
                        std::cout << "This message ID does not exist" << std::endl; 
                    } else {
                        std::cout << "File successfully deleted" << std::endl; 
                        deleted = true;
                    }
                } 
            }
        }
    }
    if(deleted == true){
        if (send(*current_socket, "OK", 3, 0) == -1) //send recieved message to socket
        {
            perror("send answer failed");
            return;
        }
    }
    else{
        if (send(*current_socket, "ERR", strlen("ERR"), 0) == -1) //send recieved message to socket
        {
            perror("send answer failed");
            return;
        }
    }
    memset(buffer, 0, strlen(buffer)); 
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
    bool isQuit = false;
    std::string ldapretval; 
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
                deleteCommand(buffer, current_socket);
                break;
            case 'P':
                ldapretval = ldapCommand(buffer, current_socket); 

                if(ldapretval == "error") {
                    if (send(*current_socket, "ERR", 4, 0) == -1) //send recieved message to socket
                    {
                        perror("send answer failed");
                        return NULL;
                    }
                } else {
                    loggedinname = ldapretval; 
                    if (send(*current_socket, "LOGIN SUCCESSFUL", 17, 0) == -1) //send recieved message to socket
                    {
                        perror("send answer failed");
                        return NULL;
                    }
                }
                std::cout << "Testoutput" << std::endl; 
                break;
            case 'Q': 
                isQuit = true;
                break; 
            default:
                std::cout << "INPUT ERROR" << std::endl; //should never happen due to clientside input validation
                break; 
        }
        memset(buffer, 0, strlen(buffer));
    } while (!isQuit);
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
//./server 5432 messages

int main(int argc, char **argv){    
    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;
    int reuseValue = 1;
    int port = atoi(argv[1]);
    dirName = argv[2];
    int parentid;
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
    address.sin_port = htons(port); // sets Port in network byte order

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
    parentid = getpid();
            //while parentID
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

        if(getpid() != parentid){
            continue;
        }
        switch(fork())	{
	    case -1: 
		    printf("Child konnte nicht gestartet werden.");
		    exit(EXIT_FAILURE);
		    break;
	    case 0:
            //                                      client address in ASCII                     port in host byte order
            std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << "..." << std::endl;
            std::cout << "CreateSocket: " << create_socket << " NewSocket: " << new_socket << std::endl;
            clientCommunication(&new_socket); // returnValue can be ignored
            new_socket = -1;
            abortRequested=true;
		    break;
	    default:
		    break;
	    }    
    }
}


        
        
        
       
                                                                                                                                               //TODO abortRequested evtl sinnlos