#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <map>
#include <sstream>

// Constants for server configuration
const int PORT = 25565; // Port on which the server will listen for incoming connections
const int MAX_CLIENTS = 10; // Maximum number of clients that the server can handle

// Error messages
const char* ERR_NEEDMOREPARAMS = "431 ERR_NEEDMOREPARAMS: Not enough parameters provided";
const char* ERR_ALREADYREGISTRED = "462 ERR_ALREADYREGISTRED: Username is already registered";

// Data structures to store client information
std::map<int, std::string> connected_clients; // Map to store connected clients (socket to username)
std::map<std::string, int> nickname_to_socket; // Map to store the mapping of usernames to sockets
std::vector<std::thread> client_threads; // Vector to store threads for handling client connections

// Function prototypes
void handle_user_command(int client_socket, const std::string& username, const std::string& hostname, const std::string& servername, const std::string& realname);
void handle_client(int client_socket);

int main() {
    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return EXIT_FAILURE;
    }

    // Set up server address structure
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        std::cerr << "Error listening for connections" << std::endl;
        close(server_socket);
        return EXIT_FAILURE;
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    while (true) {
        // Accept a connection from a client
        sockaddr_in client_address{};
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);

        if (client_socket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        // Convert client IP address to a human-readable form
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "Client connected: " << client_ip << std::endl;

        // Handle the client in a separate thread
        std::thread client_thread(handle_client, client_socket);
        client_threads.push_back(std::move(client_thread));
    }

    // Close the server socket (this part will never be reached in the current implementation)
    close(server_socket);

    return EXIT_SUCCESS;
}

// Function to handle the USER command from the client
void handle_user_command(int client_socket, const std::string& username, const std::string& hostname, const std::string& servername, const std::string& realname) {
    // Check if the username is already registered
    if (nickname_to_socket.find(username) != nickname_to_socket.end()) {
        // Send an error message and close the client connection
        send(client_socket, ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0);
        close(client_socket);
        return;
    }

    // Check if enough parameters are provided
    if (username.empty() || hostname.empty() || servername.empty() || realname.empty()) {
        // Send an error message and close the client connection
        send(client_socket, ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        close(client_socket);
        return;
    }

    // Store client information
    connected_clients[client_socket] = username;
    nickname_to_socket[username] = client_socket;

    // Notify other clients about the new connection
    std::string join_message = "New client joined: " + username;
    for (const auto& pair : connected_clients) {
        int other_client_socket = pair.first;
        if (other_client_socket != client_socket) {
            send(other_client_socket, join_message.c_str(), join_message.length(), 0);
        }
    }
}

// Function to handle the main logic for each connected client
void handle_client(int client_socket) {
    char buffer[1024]; // Buffer to store received data
    int bytes_received;

    // Receive the USER command from the client
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::string user_command(buffer);

        // Parse USER command
        std::istringstream iss(user_command);
        std::string command, username, hostname, servername, realname;
        iss >> command >> username >> hostname >> servername >> realname;

        // Handle the USER command
        if (command == "USER") {
            handle_user_command(client_socket, username, hostname, servername, realname);
        } else {
            // Handle other commands...
        }
    }

    while (true) {
        // Receive data from the client
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cout << "Client disconnected" << std::endl;

            // Broadcast a message indicating the client has left
            std::string quit_message = "Client disconnected: " + connected_clients[client_socket];
            std::cout << quit_message << std::endl;

            for (const auto& pair : connected_clients) {
                int other_client_socket = pair.first;
                if (other_client_socket != client_socket) {
                    send(other_client_socket, quit_message.c_str(), quit_message.length(), 0);
                }
            }

            // Remove the client from the list of connected clients
            nickname_to_socket.erase(connected_clients[client_socket]);
            connected_clients.erase(client_socket);

            break;
        }

        // Print the message received from the client on a new line
        buffer[bytes_received] = '\0';

        // Broadcast the message to all clients
        for (const auto& pair : connected_clients) {
            int other_client_socket = pair.first;
            std::string sender = connected_clients[client_socket];
            std::string message = "Received from " + sender + ": " + buffer;

            // Skip sending the message back to the sender
            if (other_client_socket != client_socket) {
                std::cout << message << std::endl;
                send(other_client_socket, message.c_str(), message.length(), 0);
            }
        }

        // Check if the client sent a QUIT command (case-insensitive)
        if (strncasecmp(buffer, "QUIT", 4) == 0) {
            const char* quit_message = "Goodbye!";
            std::cout << "Sending QUIT message to " << connected_clients[client_socket] << std::endl;
            send(client_socket, quit_message, strlen(quit_message), 0);

            // Broadcast a message indicating the client has left
            std::string quit_message_broadcast = "Client disconnected: " + connected_clients[client_socket];
            std::cout << quit_message_broadcast << std::endl;

            for (const auto& pair : connected_clients) {
                int other_client_socket = pair.first;
                if (other_client_socket != client_socket) {
                    send(other_client_socket, quit_message_broadcast.c_str(), quit_message_broadcast.length(), 0);
                }
            }

            // Remove the client from the list of connected clients
            nickname_to_socket.erase(connected_clients[client_socket]);
            connected_clients.erase(client_socket);

            break;
        }
    }

    // Close the client socket
    close(client_socket);
}
