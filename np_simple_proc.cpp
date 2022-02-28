
// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <stdlib.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <string.h> 
#include <cstring>
#include <string>
#include <iostream>

#include "npshell.h"
#include "UserList.h"
#define BUFSIZE      4096

std::string convertToString(char* a, int size) 
{ 
    int i; 
    std::string s = ""; 
    for (i = 0; i < size; i++) { 
        s = s + a[i]; 
    } 
    return s; 
} 

void sendWelcomeMsg(int fd)
{
	std::string msg = "****************************************\n** Welcome to the information server. **\n****************************************\n";
	write(fd, msg.c_str(), strlen(msg.c_str()));

	return;
}

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    int stdoutCopy = dup(STDOUT_FILENO), stderrCopy = dup(STDERR_FILENO);
    UserList userList;
    npshell np;
    int port = atoi(argv[1]);

    //Single-Process, Concurrent Servers 
    fd_set  rfds;       /* read file descriptor set */
    fd_set  afds;       /* active file descriptor set   */
    int nfds;

    setenv("PATH", "bin:.", 1);
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 100) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    nfds = getdtablesize();
    FD_ZERO(&afds);
    FD_SET(server_fd, &afds);

    while (true) 
    {
        memcpy(&rfds, &afds, sizeof(rfds));

        if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
            std::cerr << "select: " << strerror(errno);
        if (FD_ISSET(server_fd, &rfds)) 
        {
            int ssock;
            new_socket = accept(server_fd, (struct sockaddr *)&address,  (socklen_t*)&addrlen);
            if (new_socket < 0)
                std::cerr << "accept: " << strerror(errno);
            else
            {
            	//Get client ip
    			struct sockaddr_in clientAddr;
    			socklen_t addr_size = sizeof(struct sockaddr_in);
    			int res = getpeername(new_socket, (struct sockaddr *)&clientAddr, &addr_size);
    			
    			//Get client port
    			int clientPort =(int) ntohs(clientAddr.sin_port);
    			char *clientip = new char[20];
    			strcpy(clientip, inet_ntoa(clientAddr.sin_addr));

    			//Add client to client list
            	User *user = new User(new_socket, clientip, clientPort);
            	userList.add(user);

            	//Welecom
            	sendWelcomeMsg(new_socket);
            	//Tell everyone new user
            	userList.tellNewUser(new_socket, clientip, clientPort);
            	write(new_socket, "% ", 2);	
            	FD_SET(new_socket, &afds);
            }
        }

        for (int fd=0; fd<nfds; ++fd)
        {
            if (fd != server_fd && FD_ISSET(fd, &rfds))
            {
                int cc;
                std::string input = "";
                do
                {
                    if((cc = read( fd , buffer, 1024))<0)
                    	std::cerr << "Read error: " << strerror(errno) << std::endl;
                    else
                    	input = input + convertToString(buffer, cc);
                	//std::cout << fd << " " << input << std::endl;
                }
                while( buffer[cc-1] != '\n');
				
				if (cc < 0)
				    std::cerr << "echo read: " << strerror(errno);
				else if(cc ==0 || input.substr(0, 4) == "exit")
				{
					PipeRecord* tmp; 
                    while((tmp=np.userPipeList.findAndDeleteWithFd(fd))!=NULL)
                    {
                    	close(tmp->pip[0]);
                    	close(tmp->pip[1]);
                        free(tmp);
                    }
                    while((tmp=np.pipeRecordList.findAndDeleteWithOwner(fd))!=NULL)
                    {   
                    	close(tmp->pip[0]);
                    	close(tmp->pip[1]); 
                    	free(tmp);
                    }
                    userList.remove(fd);
				    close(fd);
                    FD_CLR(fd, &afds);
                }
                else
                {
                	std::string tmpString = input;
                	std::string cmd = np.getNextToken(tmpString, " ");
                	std::string arg = np.getNextToken(tmpString, "\n");

                	if(cmd.substr(0, 3) == "who")
                	{
                		userList.who(fd);
                	}
                	else if(cmd.substr(0, 4) == "tell")
                	{
						int reciveId = stoi(np.getNextToken(arg, " "));
						userList.tell(fd, reciveId, arg);
                	}
                	else if(cmd.substr(0, 4) == "yell")
                	{
                		userList.yell(fd, arg);	
                	}
                	else if(cmd.substr(0, 4) == "name")
                	{
                		userList.changeName(fd, arg);
                	}
                	else
                	{
                		dup2(fd, STDOUT_FILENO);
                    	dup2(fd, STDERR_FILENO);
                		np.getInput(input, fd, userList);
                		dup2(stdoutCopy, STDOUT_FILENO);
                    	dup2(stderrCopy, STDERR_FILENO);
                	}

                	write(fd, "% ", 2);	
                }                

           	}
        }
    }

    return 0; 
} 


