#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

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

void writeFile(string s) {
    ofstream logFile;
    logFile.open("log.txt", ios::app);
    logFile << s << "\n";
    logFile.close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: c2c server_port" << endl;
        return 0;
    }

    // Get port number
    string sPort = argv[1];
    int port = atoi(sPort.c_str());

    char buffer[MAXBUF];
    string response, timeStr, logStr;

    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    int sd, new_sd;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "ERROR: socket() failed" << endl;
        return -1;
    }
    cout << "Socket Created" << endl;

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
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

    while(1) {
        if ((new_sd = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
            cerr << "ERROR: accept() failed" << endl;
            return -1;
        }

        memset(buffer, 0, MAXBUF);
        read(new_sd, buffer, MAXBUF);

        if (strcmp(buffer, "#JOIN") == 0) {
            logStr = "\"" + getCurrentTime() + "\": Received a \"#JOIN\" action from agent \"" + inet_ntoa(client_addr.sin_addr) + "\"";
            writeFile(logStr);
            response = "$OK";
            write(new_sd, response.c_str(), response.length());
            logStr = "\"" + getCurrentTime() + "\": Responded to agent \"" + inet_ntoa(client_addr.sin_addr) + "\" with \"" + response + "\"";
            writeFile(logStr);
        }
        else if (strcmp(buffer, "#LEAVE") == 0) {
            logStr = "\"" + getCurrentTime() + "\": Recieved a \"#LEAVE\" action from agent \"" + inet_ntoa(client_addr.sin_addr) + "\"";
            writeFile(logStr);
            response = "$OK";
            write(new_sd, response.c_str(), response.length());
            logStr = "\"" + getCurrentTime() + "\": Responded to agent \"" + inet_ntoa(client_addr.sin_addr) + "\" with \"" + response + "\"";
            writeFile(logStr);
        }
        else if (strcmp(buffer, "#LIST") == 0) {
            logStr = "\"" + getCurrentTime() + "\": Recieved a \"#LIST\" action from agent \"" + inet_ntoa(client_addr.sin_addr) + "\"";
            writeFile(logStr);
        }
        else if (strcmp(buffer, "#LOG") == 0) {
            logStr = "\"" + getCurrentTime() + "\": Recieved a \"#LOG\" action from agent \"" + inet_ntoa(client_addr.sin_addr) + "\"";
            writeFile(logStr);
        }

        close(new_sd);
    }

    close(sd);
    return 0;
}
