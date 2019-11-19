/**************************
 * Name: Steven Hernandez
 * CS 4310
 * Project 2
 * Due Date: 12-4-2019
 * ************************/

// List of includes
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

#define MAXBUF 1024

using namespace std;

string getCurrentTime() {
   time_t current_time;
   char timeBuf[256];
   current_time = time(NULL);
   strcpy(timeBuf, ctime(&current_time));
   timeBuf[strlen(timeBuf)-1] = '\0';
   string s(timeBuf);
   return s;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: c2c server_port" << endl;
        return 0;
    }

    time_t current_time;
    char timeBuf[256];

    string sPort = argv[1];
    int nPort = atoi(sPort.c_str());

    struct sockaddr_in my_addr;
    struct sockaddr_in agent_addr;
    int addrlen = sizeof(agent_addr);

    // Create a socket to use for communication
    int sd, new_sd;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "ERROR: socket() failed" << endl;
        return -1;
    }
    cout << "Socket Created" << endl;

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(nPort);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to IP address and port #
    if ((bind(sd, (struct sockaddr *)&my_addr, sizeof(my_addr))) < 0) {
        cerr << "ERROR: bind() failed" << endl;
        return -1;
    }

    // Create a queue of size 10 to listen to connectinos
    if ((listen(sd, 10)) < 0) {
        cerr << "ERROR: listen() failed" << endl;
        return -1;
    }

    char buffer[MAXBUF];
    string response;

    while(1) {
        if ((new_sd = accept(sd, (struct sockaddr *)&my_addr, (socklen_t *)&addrlen)) < 0) {
            cerr << "ERROR: accept() failed" << endl;
            return -1;
        }
        //current_time = time(NULL);
        //strcpy(timeBuf, ctime(&current_time));
        //timeBuf[strlen(timeBuf)-1] = '\0';

        memset(buffer, 0, MAXBUF);
        read(new_sd, buffer, MAXBUF);
        cout << "Buffer: " << buffer << endl;

        if(strcmp(buffer, "#JOIN") == 0) {
            cout << "\"" << getCurrentTime() << "\": Received a \"#JOIN\" action from agent " << inet_ntoa(my_addr.sin_addr) << "\n";
            response = "$OK";
            write(new_sd, response.c_str(), response.length());
        }
        else if(strcmp(buffer, "#LEAVE") == 0) {
            response = "$OK";
            write(new_sd, response.c_str(), response.length());
        }
        else if(strcmp(buffer, "#LIST") == 0) {
            response = "Returning list of active active agents...";
            write(new_sd, response.c_str(), response.length());
        }
        else if(strcmp(buffer, "#LOG") == 0) {
           response = "Returning a file called log.txt";
           write(new_sd, response.c_str(), response.length());
        }

        close(new_sd);
    }

    close(sd);
    return 0;
}
