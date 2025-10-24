#include <arpa/inet.h>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <cstring>

void decrypt(unsigned char *buffer, char *keys);

void printProcess(int pktReceiveTimes);

int pktSize = 0;
int keyTimes = 0;
unsigned char printBuffer[4];

int main() {
    int port;
    char ip[50];
    char file[50];
    char key[50];

    std::cout << "Connect to IP address: ";
    std::cin >> ip;
    std::cout << "Port #:";
    std::cin >> port;
    std::cout << "Save file to: ";
    std::cin >> file;
    std::cout << "Enter encryption key: ";
    std::cin >> key;

    // socket part
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        perror("Socket Error: ");
        exit(-1);
    }

    // set address
    struct sockaddr_in listenAddress{};
    bzero(&listenAddress, sizeof(listenAddress));
    listenAddress.sin_family = AF_INET;
    listenAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddress.sin_port = htons(port);

    // bind part
    if (bind(listenSocket, (struct sockaddr *) &listenAddress, sizeof(sockaddr_in)) == -1) {
        perror("Bind Error: ");
        exit(-1);
    }

    // listen part
    if (listen(listenSocket, 5) == -1) {
        perror("Listen Error: ");
        exit(-1);
    }


    struct sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(struct sockaddr_in);

    // get client socket
    int clientSocket;
    while (true) {
        bzero(&clientAddress, sizeof(clientAddress));
        clientSocket = accept(listenSocket, (struct sockaddr *) (&clientAddress), &clientAddressLength);
        if (clientSocket == -1) {
            perror("Client Socket Error: ");
            exit(-1);
        }
        if (clientAddress.sin_addr.s_addr != inet_addr(ip)) {
            std::cout << "Incorrect Client IP Want to Connect! Refuse! " << std::endl;
            std::cout << "Still Waiting Client" << std::endl;
            close(clientSocket);
        } else {
            break;
        }
    }


    int pktReceiveTimes = 0;
    FILE *fp = fopen(file, "w");
    int firstPktSize;
    while (true) {
        char pktSizeBuffer[100];
        bzero(&pktSizeBuffer, sizeof(pktSizeBuffer));
        recv(clientSocket, pktSizeBuffer, sizeof(pktSizeBuffer), 0);
        if (pktSizeBuffer[0] == 'e' &&
            pktSizeBuffer[1] == 'n' &&
            pktSizeBuffer[2] == 'd' &&
            pktSizeBuffer[3] == '*') {
            break;
        }

        pktSize = atoi(pktSizeBuffer);
        if (pktReceiveTimes == 0) {
            firstPktSize = pktSize;
        }
        unsigned char receiveBuffer[pktSize];
        bzero(&receiveBuffer, sizeof(receiveBuffer));
        int receivedLength;
        int shouldReceivedLength = pktSize;
        unsigned char *startPoint = receiveBuffer;
        while (true) {
            receivedLength = recv(clientSocket, startPoint, shouldReceivedLength, 0);
            if(receivedLength != shouldReceivedLength){
                // not end
                startPoint = startPoint + receivedLength;
                shouldReceivedLength = shouldReceivedLength - receivedLength;
            }else{
                // end
                break;
            }
        }
        decrypt(receiveBuffer, key);
        printProcess(pktReceiveTimes);
        fwrite(receiveBuffer, sizeof(char), pktSize, fp);
        if (pktSize == firstPktSize) {
            pktReceiveTimes++;
        } else {
            break;
        }
    }
    fclose(fp);
    std::cout << "Receive success!" << std::endl;
    std::cout << "MD5: " << std::endl;
    char command[8] = "md5sum ";
    system(strcat(command, file));
    close(clientSocket);
    close(listenSocket);
    return 0;
}

void printProcess(int pktReceiveTimes) {
    std::cout << "Rec packet# " << std::dec << pktReceiveTimes
              << " – encrypted as " << std::hex
              << std::setfill('0') << std::setw(2) << (int) printBuffer[0]
              << std::setfill('0') << std::setw(2) << (int) printBuffer[1] << "...."
              << std::setfill('0') << std::setw(2) << (int) printBuffer[2]
              << std::setfill('0') << std::setw(2) << (int) printBuffer[3] << std::endl;

}


void decrypt(unsigned char *buffer, char *keys) {
    unsigned char encrypted;
    unsigned char key;
    unsigned char content;
    for (int i = 0; i < pktSize; i++) {

        encrypted = buffer[i];
        key = keys[keyTimes % strlen(keys)];
        keyTimes++;
        content = encrypted ^ key;
        if (i == 0) {
            printBuffer[0] = encrypted;
        } else if (i == 1) {
            printBuffer[1] = encrypted;
        } else if (i == pktSize - 2) {
            printBuffer[2] = encrypted;
        } else if (i == pktSize - 1) {
            printBuffer[3] = encrypted;
        }
        buffer[i] = content;
    }
}




