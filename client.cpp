#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <string>
#include <fcntl.h>
#include <sys/select.h>

// Constant for the server port
const int PORT = 25565;

// Constants for error messages
const char* ERR_NEEDMOREPARAMS = "431 ERR_NEEDMOREPARAMS: Not enough parameters provided";
const char* ERR_ALREADYREGISTRED = "462 ERR_ALREADYREGISTRED: Username is already registered";

// Function to set a socket to non-blocking mode
void set_socket_nonblocking(int socket_fd) {
    // Get current socket flags
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
    // Set the O_NONBLOCK flag
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

// Function to send the initial user command to the server
void send_user_command(int client_socket, const std::string& username, const std::string& hostname, const std::string& servername, const std::string& realname);

// Function to handle the chat interaction
void chat(int client_socket, const std::string& nickname, const std::string& real_name, const std::string& username);

// Function to handle non-blocking input from the user
void input_thread(int client_socket) {
    std::string message;
    while (true) {
        // Read user input from the console
        std::getline(std::cin, message);
        if (!message.empty()) {
            // Send the user's message to the server
            if (send(client_socket, message.c_str(), message.length(), 0) == -1) {
                std::cerr << "Error sending message to the server" << std::endl;
                break;
            }
            // Check if the user wants to quit the chat
            if (strncasecmp(message.c_str(), "QUIT", 4) == 0) {
                break;
            }
        }
    }
}

int main(int argc, char** argv) {
    // Display welcome message and usage instructions
    std::cout << "Welcome to the chatroom, Type in your desired message followed by the return key" << std::endl;
    std::cout << "Type in 'QUIT' as the first word in your chat to exit the chatroom" << std::endl;
    std::cout << "EX: 'QUIT' 'quit' 'Quit this' 'Quitting the session' etc" << std::endl << "Start conversing" << std::endl << std::endl; 

    // Check the command-line arguments
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <nickname> <real_name> <username>" << std::endl;
        return EXIT_FAILURE;
    }

    // Extract command-line arguments
    const char* server_ip = argv[1];
    const std::string nickname = argv[2];
    const std::string real_name = argv[3];
    const std::string username = argv[4];

    // Create a socket for communication with the server
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return EXIT_FAILURE;
    }

    // Set up server address structure
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        close(client_socket);
        return EXIT_FAILURE;
    }

    // Set client socket to non-blocking mode
    set_socket_nonblocking(client_socket);

    // Send the initial user command to the server
    send_user_command(client_socket, username, "localhost", "server", real_name);

    // Start the input thread for user messages
    std::thread input(input_thread, client_socket);

    // Main loop for receiving and printing messages from the server
    char message[1024];
    int bytes_received;
    while (true) {
        // Receive messages from the server
        bytes_received = recv(client_socket, message, sizeof(message), 0);
        if (bytes_received > 0) {
            message[bytes_received] = '\0';

            // Print the message only if it is not a server notification
            if (strncmp(message, "Received from server:", strlen("Received from server:")) != 0) {
                std::cout << message << std::endl;
            }
        } else if (bytes_received == 0) {
            std::cerr << "Server disconnected" << std::endl;
            break;
        }

        // Sleep for a short duration to avoid high CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Wait for the input thread to finish
    input.join();

    // Close the client socket
    close(client_socket);

    return EXIT_SUCCESS;
}

// Function to send the initial user command to the server
void send_user_command(int client_socket, const std::string& username, const std::string& hostname, const std::string& servername, const std::string& realname) {
    // Create the USER command
    std::string user_command = "USER " + username + " " + hostname + " " + servername + " " + realname;
    // Send the USER command to the server
    send(client_socket, user_command.c_str(), user_command.length(), 0);

    // Receive and handle the server's response
    char response[1024];
    int bytes_received = recv(client_socket, response, sizeof(response), 0);
    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        // Print the server's response
        std::cout << "Server response: " << response << std::endl;
        // Check for error conditions in the response
        if (strstr(response, ERR_NEEDMOREPARAMS) != nullptr || strstr(response, ERR_ALREADYREGISTRED) != nullptr) {
            std::cerr << "Error: " << response << std::endl;
            // Close the client socket and exit with failure
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }
}

