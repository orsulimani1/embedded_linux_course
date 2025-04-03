#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

// Constants
#define MAX_EVENTS 64
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080
#define NUM_CLIENTS 10
#define MAX_REQUESTS_PER_CLIENT 5

// Client state
typedef struct {
    int socket;
    char read_buffer[BUFFER_SIZE];
    int read_size;
    char write_buffer[BUFFER_SIZE];
    int write_size;
    int write_offset;
} Client;

// Global variables
volatile sig_atomic_t running = 1;
Client* clients[MAX_CLIENTS];

// Handle signals for graceful shutdown
void signal_handler(int sig) {
    running = 0;
}

// Set a socket to non-blocking mode
int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
        return -1;
    }
    
    return 0;
}

// Create a client structure
Client* create_client(int socket_fd) {
    Client* client = (Client*)malloc(sizeof(Client));
    if (!client) {
        perror("malloc client");
        return NULL;
    }
    
    memset(client, 0, sizeof(Client));
    client->socket = socket_fd;
    
    return client;
}

// Clean up client resources
void close_client(Client* client, int epoll_fd) {
    if (!client) return;
    
    if (client->socket >= 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->socket, NULL);
        close(client->socket);
        
        // Remove from client array
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] && clients[i]->socket == client->socket) {
                clients[i] = NULL;
                break;
            }
        }
    }
    
    free(client);
}

// Handle a read event
void handle_read_event(Client* client, int epoll_fd) {
    // Read data from the client socket
    ssize_t bytes_read = read(client->socket, 
                              client->read_buffer + client->read_size, 
                              BUFFER_SIZE - client->read_size);
    
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("Client disconnected\n");
        } else {
            perror("read");
        }
        close_client(client, epoll_fd);
        return;
    }
    
    client->read_size += bytes_read;
    
    // Process the message (in a real application, you'd parse the message here)
    // For this example, we'll just echo it back
    
    // Copy read buffer to write buffer
    memcpy(client->write_buffer, client->read_buffer, client->read_size);
    client->write_size = client->read_size;
    client->write_offset = 0;
    client->read_size = 0;
    
    // Modify EPOLL to watch for write events
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.fd = client->socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->socket, &ev) == -1) {
        perror("epoll_ctl: mod");
        close_client(client, epoll_fd);
        return;
    }
}

// Handle a write event
void handle_write_event(Client* client, int epoll_fd) {
    // If we have data to write
    if (client->write_offset < client->write_size) {
        // Write data to the client socket
        ssize_t bytes_written = write(client->socket, 
                                     client->write_buffer + client->write_offset, 
                                     client->write_size - client->write_offset);
        
        if (bytes_written <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket is not ready for writing, try again later
                return;
            }
            
            perror("write");
            close_client(client, epoll_fd);
            return;
        }
        
        client->write_offset += bytes_written;
        
        // If we've written everything
        if (client->write_offset >= client->write_size) {
            client->write_size = 0;
            client->write_offset = 0;
            
            // Modify EPOLL to only watch for read events
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = client->socket;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->socket, &ev) == -1) {
                perror("epoll_ctl: mod");
                close_client(client, epoll_fd);
            }
        }
    }
}

// Server thread function
void* server_thread(void* arg) {
    int server_fd, epoll_fd;
    struct sockaddr_in server_addr;
    struct epoll_event events[MAX_EVENTS];

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return NULL;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return NULL;
    }
    
    // Make server socket non-blocking
    if (set_nonblocking(server_fd) == -1) {
        close(server_fd);
        return NULL;
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return NULL;
    }
    
    // Listen for connections
    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(server_fd);
        return NULL;
    }
    
    printf("Server started on port %d\n", SERVER_PORT);
    
    // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_fd);
        return NULL;
    }
    
    // Add server socket to epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        close(epoll_fd);
        return NULL;
    }

    // Initialize clients array
    memset(clients, 0, sizeof(clients));
    
    // Event loop
    while (running) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);  // 1 second timeout
        if (nfds == -1) {
            if (errno == EINTR) {
                // Interrupted system call (signal), just retry
                continue;
            }
            perror("epoll_wait");
            break;
        }
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // Accept new connections
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                while (running) {  // Accept all pending connections
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No more connections to accept
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }
                    
                    // Set client socket to non-blocking
                    if (set_nonblocking(client_fd) == -1) {
                        close(client_fd);
                        continue;
                    }
                    
                    // Store client information
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                    printf("New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
                    
                    // Create client structure
                    Client* client = create_client(client_fd);
                    if (!client) {
                        close(client_fd);
                        continue;
                    }
                    
                    // Add to clients array
                    int slot = -1;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j] == NULL) {
                            clients[j] = client;
                            slot = j;
                            break;
                        }
                    }
                    
                    if (slot == -1) {
                        // No available slots
                        printf("Maximum clients reached, rejecting connection\n");
                        close_client(client, epoll_fd);
                        continue;
                    }
                    
                    // Add client socket to epoll
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                        perror("epoll_ctl: client_fd");
                        close_client(client, epoll_fd);
                        continue;
                    }
                }
            } else {
                // Handle client event
                int fd = events[i].data.fd;
                
                // Find the client
                Client* client = NULL;
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j] && clients[j]->socket == fd) {
                        client = clients[j];
                        break;
                    }
                }
                
                if (!client) {
                    // This shouldn't happen, but just in case
                    printf("Unknown client socket: %d\n", fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    continue;
                }
                
                // Handle the event
                if (events[i].events & EPOLLIN) {
                    handle_read_event(client, epoll_fd);
                }
                
                if (events[i].events & EPOLLOUT) {
                    handle_write_event(client, epoll_fd);
                }
                
                if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    printf("Socket error or disconnect\n");
                    close_client(client, epoll_fd);
                }
            }
        }
    }
    
    // Cleanup
    printf("Server shutting down...\n");
    
    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            close_client(clients[i], epoll_fd);
        }
    }
    
    close(server_fd);
    close(epoll_fd);
    
    printf("Server shutdown complete\n");
    return NULL;
}

// Client thread function
void* client_thread(void* arg) {
    int client_id = *((int*)arg);
    free(arg);
    
    // Wait for server to start
    sleep(1);
    
    // Create client socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return NULL;
    }
    
    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Convert IP address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return NULL;
    }
    
    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return NULL;
    }
    
    printf("Client %d connected to server\n", client_id);
    
    // Send requests to the server
    char send_buffer[BUFFER_SIZE];
    char recv_buffer[BUFFER_SIZE];
    
    for (int request = 0; request < MAX_REQUESTS_PER_CLIENT; request++) {
        // Generate a message
        snprintf(send_buffer, BUFFER_SIZE, "Hello from client %d, request %d", 
                 client_id, request);
        
        // Send the message
        ssize_t bytes_sent = send(sockfd, send_buffer, strlen(send_buffer), 0);
        if (bytes_sent <= 0) {
            perror("send");
            break;
        }
        
        printf("Client %d sent: %s\n", client_id, send_buffer);
        
        // Receive the response
        ssize_t bytes_received = recv(sockfd, recv_buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Server closed connection\n");
            } else {
                perror("recv");
            }
            break;
        }
        
        // Null-terminate the received data
        recv_buffer[bytes_received] = '\0';
        
        printf("Client %d received: %s\n", client_id, recv_buffer);
        
        // Wait a random time before the next request
        usleep((rand() % 1000 + 500) * 1000);  // 500-1500ms
    }
    
    printf("Client %d finished\n", client_id);
    close(sockfd);
    
    return NULL;
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Seed random number generator
    srand(time(NULL));
    
    // Start server thread
    pthread_t server_tid;
    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("pthread_create: server");
        return 1;
    }
    
    // Start client threads
    pthread_t client_tids[NUM_CLIENTS];
    for (int i = 0; i < NUM_CLIENTS; i++) {
        int* client_id = malloc(sizeof(int));
        *client_id = i + 1;
        
        if (pthread_create(&client_tids[i], NULL, client_thread, client_id) != 0) {
            perror("pthread_create: client");
            free(client_id);
            continue;
        }
        
        // Stagger client connections
        usleep((rand() % 300 + 100) * 1000);  // 100-400ms
    }
    
    // Wait for client threads to finish
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(client_tids[i], NULL);
    }
    
    // Allow time for the server to process any final messages
    sleep(1);
    
    // Signal the server to stop
    running = 0;
    
    // Wait for server thread to finish
    pthread_join(server_tid, NULL);
    
    return 0;
}