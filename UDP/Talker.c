/* Talker.c
Author: Dylan Wallace, Drones and Autonomous Systems Lab, University of Nevada, Las Vegas

A simple C UDP talker to broadcast a message to the port defined. Allows the user to input the IP address, and connects to the defined port.
The user can input the desired message to send. Made with the help of Beej's Guide to Network Programming.*/

#include <stdio.h>      // Standard Input/Output library
#include <stdlib.h>     // Standard library
#include <unistd.h>     // Unix API
#include <errno.h>      // Error library
#include <string.h>     // String library
#include <sys/types.h>  // System types library
#include <sys/socket.h> // Sockets library
#include <netinet/in.h> // Main internet library
#include <arpa/inet.h>  // The Internet library
#include <netdb.h>      // Internet database

#define SERVERPORT "4812" // The port users will be connecting to

int main(int argc, char *argv[]) // Uses command line arguments
{
    int sockfd; // Socket descriptor
    struct addrinfo hints, *servinfo, *p; // Structs to store IP address info
    int rv; // Error checking
    int numbytes; // Number of bytes to send

    if (argc != 3) { // Check command line arguments
        fprintf(stderr,"usage: talker hostname message\n"); // Print usage if needed
        exit(1);
    }

    memset(&hints, 0, sizeof hints); // Fill out empty structure
    hints.ai_family = AF_UNSPEC; // We don't care if IPv4 or IPv6 (set to AF_INET to force IPv4)
    hints.ai_socktype = SOCK_DGRAM; // UDP type

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) { // If getaddrinfo fails
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Give getaddrinfo error
        return 1;
    }

    // Loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) { // If socket create fails
            perror("talker: socket"); // Give socket error
            continue;
        }

        break;
    }

    if (p == NULL) { // Check if anything actually ever was created
        fprintf(stderr, "talker: failed to create socket\n"); // Give failure to create message
        return 2;
    }

    if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) { // If sending error
        perror("talker: sendto"); // Give sending error
        exit(1);
    }

    freeaddrinfo(servinfo); // Free the IP address structure

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]); // Print number of bytes sent to the IP address
    close(sockfd); // Close the socket

    return 0;
}
