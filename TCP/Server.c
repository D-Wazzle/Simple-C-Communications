/* Server.c
Author: Dylan Wallace, Drones and Autonomous Systems Lab, University of Nevada, Las Vegas

A simple C TCP server to communicate with the client defined by Client.c. Broadcasts the defined message to the defined port.
Made with the help of Beej's Guide to Network Programming.*/

#include <stdio.h>      // Standard Input/Output
#include <stdlib.h>     // Standard library
#include <unistd.h>     // Unix API
#include <errno.h>      // Error library
#include <string.h>     // String library
#include <sys/types.h>  // Unix System types
#include <sys/socket.h> // Unix System sockets
#include <netinet/in.h> // Internet library
#include <netdb.h>      // Internet database
#include <arpa/inet.h>  // Internet library
#include <sys/wait.h>   // Wait library
#include <signal.h>     // Exception/error handler

#define PORT "4800"     // the port users will be connecting to

#define BACKLOG 10      // how many pending connections queue will hold

// Function to reap all dead processes
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// Function to get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) { // If IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr); //Return IPv4
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); //Otherwise return IPv6
}

int main(void)
{
    int sockfd, new_fd;  // Listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p; //Holds IP address info
    struct sockaddr_storage their_addr; // Connector's address information
    socklen_t sin_size; // Holds socket size
    struct sigaction sa; // Holds data for reaping dead processes
    int yes = 1; // Boolean reference
    char s[INET6_ADDRSTRLEN]; // Holds the socket IP address in string form
    int rv; // Holds error status for getaddrinfo

    memset(&hints, 0, sizeof hints); // Fill out empty addrinfo struct
    hints.ai_family = AF_UNSPEC; // Set family to either IPv4 or IPv6, we don't care
    hints.ai_socktype = SOCK_STREAM; // Set socket type to stream (TCP)
    hints.ai_flags = AI_PASSIVE; // Use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { // If error in getaddrinfo
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Print error info
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) { // Loop through all the results and bind to the first we can
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) { // If error during sock creation
            perror("server: socket"); // Give socket error
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) { // If error in getting the option
            perror("setsockopt"); // Give option error
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // If error during bind
            close(sockfd); // Close the socket
            perror("server: bind"); // Give bind error
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // Free up this structure

    if (p == NULL)  { // If no port bound to p
        fprintf(stderr, "server: failed to bind\n"); // Give failure to bind error
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) { // If listen fails
        perror("listen"); // Give listen error
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // Reap all dead processes
    sigemptyset(&sa.sa_mask); // Clear processes
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) { // If reap error
        perror("sigaction"); // Give sigaction error
        exit(1);
    }

    printf("server: waiting for connections...\n"); // Print waiting statement

    while(1) {  // Main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); // Accept the connection
        if (new_fd == -1) { // If error during accept
            perror("accept"); // Give accept error
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s); // Translate IP address into string form
        printf("server: got connection from %s\n", s); // Print client IP address

        if (!fork()) { // This is the child process
            close(sockfd); // Child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1) // If send error (Note: the message being sent can be changed here)
                perror("send"); // Give send error
            close(new_fd); // Close the connection
            exit(0);
        }
        close(new_fd);  // Close the connection
    }

    return 0;
}
