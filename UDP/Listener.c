/* Listener.c
Author: Dylan Wallace, Drones and Autonomous Systems Lab, University of Nevada, Las Vegas

A simple C UDP listener to listen to the port defined. Receives the message sent to the defined port.
Made with the help of Beej's Guide to Network Programming.*/

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

#define MYPORT "4812"   // The port users will be connecting to

#define MAXBUFLEN 100   // The maximum receiving buffer

// Function to get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) { // If IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr); // Return IPv4
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); // Return IPv6
}

int main(void)
{
    int sockfd; // Socket descriptor
    struct addrinfo hints, *servinfo, *p; // Used to store IP address info
    int rv; // Used for error checking
    int numbytes; // Number of bytes received
    struct sockaddr_storage their_addr; // Store the talker IP address
    char buf[MAXBUFLEN]; // Receiving buffer
    socklen_t addr_len; // IP address length
    char s[INET6_ADDRSTRLEN]; // String to store IP address

    memset(&hints, 0, sizeof hints); // Fill structure with empty info
    hints.ai_family = AF_UNSPEC; // Don't care if IPv4 or IPv6 (set to AF_INET to force IPv4)
    hints.ai_socktype = SOCK_DGRAM; // UDP type
    hints.ai_flags = AI_PASSIVE; // Use my IP to listen to the socket

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) { // If getaddrinfo fails
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Give getaddrinfo error
        return 1;
    }

    // Loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) { // If socket creation error
            perror("listener: socket"); // Give socket error
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // If bind fails
            close(sockfd); // Close the socket
            perror("listener: bind"); // Give bind error
            continue;
        }

        break;
    }

    if (p == NULL) { // If nothing ever bound to the socket
        fprintf(stderr, "listener: failed to bind socket\n"); // Give failure to bind message
        return 2;
    }

    freeaddrinfo(servinfo); // Free the structure

    printf("listener: waiting to recvfrom...\n"); // Give waiting to receive message

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) { // If receiving error
        perror("recvfrom"); // Give receive error
        exit(1);
    }

    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s)); // Print received message and converted IP address
    printf("listener: packet is %d bytes long\n", numbytes); // Give number of bytes in the packet
    buf[numbytes] = '\0'; // Add null-terminator
    printf("listener: packet contains \"%s\"\n", buf); // Print packet contents

    close(sockfd); // Close the socket

    return 0;
}
