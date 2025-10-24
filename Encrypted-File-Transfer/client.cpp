#include <cstring>
#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <arpa/inet.h>

void encrypt(unsigned char *buffer, char *keys);

void printProcess(int pktSendTimes);

int keyTimes = 0;
int realRead = 0;
unsigned char printBuffer[4];

int main() {
    int port;
    int size;
    char ip[50];
    char file[50];
    char key[50];

    // read & set parameters
    std::cout << "Connect to IP address: ";
    bzero(&ip, sizeof(ip));
    std::cin >> ip;
    std::cout << "Port #:";
    std::cin >> port;
    std::cout << "File to be sent: ";
    bzero(&file, sizeof(file));
    std::cin >> file;
    std::cout << "Pkt size: ";
    std::cin >> size;
    std::cout << "Enter encryption key: ";
    bzero(&key, sizeof(key));
    std::cin >> key;

    // size to KB *1024
    size *= 1024;

    // socket part
    int sendSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (sendSocket == -1) {
        perror("Socket Error: ");
        exit(-1);
    }

    // set address
    struct sockaddr_in sendAddress{};
    bzero(&sendAddress, sizeof(sendAddress));
    sendAddress.sin_addr.s_addr = inet_addr(ip);
    sendAddress.sin_family = AF_INET;
    sendAddress.sin_port = htons(port);

    // connect part
    if (connect(sendSocket, (struct sockaddr *) &sendAddress, sizeof(sockaddr_in)) == -1) {
        perror("Connect Error: ");
        exit(-1);
    }

    // file part
    FILE *fp = fopen(file, "r");
    if (fp == nullptr) {
        perror("File Error: ");
        exit(-1);
    }

    unsigned char partBuffer[size];
    char pktSize[100];
    int pktSendTimes = 0;

    bzero(&partBuffer, sizeof(partBuffer));
    realRead = fread(partBuffer, sizeof(char), size, fp);
    while (realRead == size) {
        // send size
        bzero(&pktSize, sizeof(pktSize));
        sprintf(pktSize, "%d", realRead);
        send(sendSocket, pktSize, sizeof(pktSize), 0);
        // send content
        encrypt(partBuffer, key);
        printProcess(pktSendTimes);
        send(sendSocket, partBuffer, realRead, 0);
        pktSendTimes++;
        // continue read
        bzero(&partBuffer, sizeof(partBuffer));
        realRead = fread(partBuffer, sizeof(char), size, fp);
    }

    if (realRead == 0) {
        bzero(&pktSize, sizeof(pktSize));
        pktSize[0] = 'e';
        pktSize[1] = 'n';
        pktSize[2] = 'd';
        pktSize[3] = '*';
        send(sendSocket, pktSize, sizeof(pktSize), 0);
    } else {
        //last time sent size
        sprintf(pktSize, "%d", realRead);
        send(sendSocket, pktSize, sizeof(pktSize), 0);
        //sent content
        encrypt(partBuffer, key);
        printProcess(pktSendTimes);
        send(sendSocket, partBuffer, realRead, 0);
    }
    fclose(fp);
    std::cout << "Send success!" << std::endl;
    std::cout << "MD5: " << std::endl;
    char command[8] = "md5sum ";
    system(strcat(command, file));
    close(sendSocket);
    return 0;
}

void printProcess(int pktSendTimes) {
    std::cout << "Sent encrypted packet# " << std::dec << pktSendTimes
              << " – encrypted as " << std::hex
              << std::setfill('0') << std::setw(2) << (int) printBuffer[0]
              << std::setfill('0') << std::setw(2) << (int) printBuffer[1] << "...."
              << std::setfill('0') << std::setw(2) << (int) printBuffer[2]
              << std::setfill('0') << std::setw(2) << (int) printBuffer[3] << std::endl;
}


void encrypt(unsigned char *buffer, char *keys) {
    unsigned char content;
    unsigned char key;
    unsigned char encrypted;
    for (int i = 0; i < realRead; i++) {
        content = buffer[i];
        key = keys[keyTimes % strlen(keys)];
        keyTimes++;
        encrypted = content ^ key;
        if (i == 0) {
            printBuffer[0] = encrypted;
        } else if (i == 1) {
            printBuffer[1] = encrypted;
        } else if (i == realRead - 2) {
            printBuffer[2] = encrypted;
        } else if (i == realRead - 1) {
            printBuffer[3] = encrypted;
        }
        buffer[i] = encrypted;
    }
}




