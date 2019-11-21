#include <iostream>
#include <fstream>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAXBUF 1024

using namespace std;

void printToLog(string s) {
    ofstream logFile("log.txt", ios::app);

    time_t tm = time(NULL);
    char* timeStamp = ctime(&tm);
    timeStamp[strlen(timeStamp)-1] = '\0';

    logFile << "\"" << timeStamp << "\": " << s << "\n";

    logFile.close();
}

bool isInList(string ip, string list[],int size) {
    bool found = false;
    for(int i = 0; i < size; i++) {
        if(list[i] == ip)
            found = true;
    }
    return found;
}

void removeFromList(string ip, string list1[], time_t list2[], int &size) {
    int index = 0;
    bool found = false;
    while(!found && (index < size)) {
        if(list1[index] == ip)
            found = true;
        else
            index++;
    }
    for(int i = index; i < size - 1; i++) {
        list1[i] = list1[i+1];
        list2[i] = list2[i+1];
    }
    size--;
}

int main(int argc, char* argv[]) {
    // Check for correct # of arguments
    if(argc < 2) {
        cerr << "Usage: c2c port_number" << endl;
        return -1;
    }

    // Define buffer and other variables to be used
    char buffer[MAXBUF];
    int numAgents = 0;
    string agents[15];
    time_t timeStamps[15];

    int port = atoi(argv[1]);
    struct sockaddr_in my_addr;
    struct sockaddr_in agent_addr;
    int addrlen = sizeof(agent_addr);
    string ip_addr, response;

    //Create a new socket
    int sd, new_sd;
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "ERROR: socket() failed" << endl;
        return -1;
    }
    cout << "Socket Created" << endl;

    // Bind the socket to my IP and desired port number
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if((bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr))) < 0) {
        cerr << "ERROR: bind() failed" << endl;
        return -1;
    }

    // Prepare to accept connections and set 10 as total requests queued
    if((listen(sd, 10)) < 0) {
        cerr << "ERROR: listen() failed" << endl;
        return -1;
    }

    // Cycle through loop until server is stopped with Cntl+C
    while(1) {
        // Wait until a new connection is established
        if((new_sd = accept(sd, (struct sockaddr*)&agent_addr, (socklen_t*)&addrlen)) < 0) {
            cerr << "ERROR: accept() failed" << endl;
            return -1;
        }
        ip_addr = inet_ntoa(agent_addr.sin_addr);

        // Read the action sent by the agent
        memset(buffer, 0, MAXBUF);
        read(new_sd, buffer, MAXBUF);

        // Determine which action was sent
        if(strcmp(buffer, "#JOIN") == 0) {
            printToLog("Received a \"#JOIN\" action from agent \"" + ip_addr + "\"");

            if(isInList(ip_addr, agents, numAgents)) {
                response = "$ALREADY MEMBER";
                write(new_sd, response.c_str(), response.length());
                printToLog("Responded to agent \"" + ip_addr + "\" with \"" + response + "\"");
            }
            else {
                agents[numAgents] = ip_addr;
                timeStamps[numAgents] = time(NULL);
                numAgents++;
                response = "$OK";
                write(new_sd, response.c_str(), response.length());
                printToLog("Responded to agent \"" + ip_addr + "\" with \"" + response + "\"");
            }
        }
        else if(strcmp(buffer, "#LEAVE") == 0) {
            printToLog("Received a \"#LEAVE\" action from agent \"" + ip_addr + "\"");

            if(isInList(ip_addr, agents, numAgents)) {
                removeFromList(ip_addr, agents, timeStamps, numAgents);
                response = "$OK";
                write(new_sd, response.c_str(), response.length());
                printToLog("Responded to agent \"" + ip_addr + "\" with \"" + response + "\"");
            }
            else {
                response = "$NOT MEMBER";
                write(new_sd, response.c_str(), response.length());
                printToLog("Responded to agent \"" + ip_addr + "\" with \"" + response + "\"");
            }
        }
        else if(strcmp(buffer, "#LIST") == 0) {
            printToLog("Received a \"#LIST\" action from agent \"" + ip_addr + "\"");

            if(isInList(ip_addr, agents, numAgents)) {
                cout << "Active Agents" << endl;
                for(int i = 0; i < numAgents; i++) {
                    cout << agents[i] << "\t";
                    cout << difftime(time(NULL), timeStamps[i]) << " seconds ago" << endl;
                }
            }
            else {
                printToLog("No response is supplied to agent \"" + ip_addr + "\" (agent not active)");
            }
        }
        else if(strcmp(buffer, "#LOG") == 0) {
            printToLog("Received a \"#LOG\" action from agent \"" + ip_addr + "\"");
        }

        close(new_sd);
    }

    close(sd);
    return 0;
}