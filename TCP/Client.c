/* Client.c
Author: Dylan Wallace, Drones and Autonomous Systems Lab, University of Nevada, Las Vegas

A simple C TCP client to connect to the server defined by Server.c. Allows the user to input the IP address, and connects to the defined port.
Receives the message defined by Server.c. Made with the help of Beej's Guide to Network Programming.*/

#include <stdio.h>      // Standard Input/Output library
#include <stdlib.h>     // Standard library
#include <unistd.h>     // Unix API
#include <errno.h>      // Error library
#include <string.h>     // String library
#include <netdb.h>      // Internet database
#include <sys/types.h>  // System types library
#include <netinet/in.h> // Main internet library
#include <sys/socket.h> // Sockets library

#include <arpa/inet.h>  // The Internet library

#define PORT "4800"     // The port client will be connecting to

#define MAXDATASIZE 100 // Max number of bytes we can get at once

// Function to get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) { // If IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr); // Return IPv4
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); // Otherwise return IPv6
}

int main(int argc, char *argv[]) // Get command line arguments
{
    int sockfd, numbytes; // Stores our socket descriptor, and number of bytes read
    char buf[MAXDATASIZE]; // Data buffer for reading the message
    struct addrinfo hints, *servinfo, *p; // Stores our IP address info
    int rv; // Used for error checking
    char s[INET6_ADDRSTRLEN]; // Used to store IP address of server

    if (argc != 2) { // Check CL argument count
        fprintf(stderr,"usage: client hostname\n"); // Give usage if wrong
        exit(1);
    }

    memset(&hints, 0, sizeof hints); // Fill out addrinfo with empty data
    hints.ai_family = AF_UNSPEC; // Don't care if IPv4 or IPv6 (set to AF_INET to force IPv4)
    hints.ai_socktype = SOCK_STREAM; // TCP type

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) { // If getaddrinfo fails
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Display error if failed
        return 1;
    }

    // Loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) { // If the socket fails to create properly
            perror("client: socket"); // Give socket error
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // If fails to connect
            close(sockfd); // Close the socket
            perror("client: connect"); // Give connection error
            continue;
        }

        break;
    }

    if (p == NULL) { // If nothing ever connected
        fprintf(stderr, "client: failed to connect\n"); // Print connection failure
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s); // Convert IP address into a string
    printf("client: connecting to %s\n", s); // Print connection message and IP address

    freeaddrinfo(servinfo); // Free the servinfo structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) { // If receiving the message fails
        perror("recv"); // Give receive error
        exit(1);
    }

    buf[numbytes] = '\0'; // Add null termination

    printf("client: received '%s'\n",buf); // Print received string

    close(sockfd); //Close the socket

    return 0;
}
