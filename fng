Chat Application
Overview
This client-server application enables users to chat in real-time. It operates with a client-server architecture where the server listens on port 25565. Clients are required to provide a username and server name when initiating a connection on the same port as the server. Usernames are local and not necessarily unique.

Level 1
At Level 1, the application allows anonymous users to chat.

Level 2
At Level 2, the server handles the USER command for connection. It verifies unique names and provides error messages. Clients can disconnect with a QUIT message.

Level 3
At Level 3, the server maintains a list of unique names and handles multi-user interactions. It sends messages to all connected clients, ensuring a dynamic chat environment. The USER command specifies username, hostname, servername, and real name during connection, with possible server replies for insufficient parameters or existing registrations. The application facilitates real-time communication between multiple clients without the need for channels.

Commands
USER Command
Command: USER
Parameters: <username> <hostname> <servername> <realname>
Example: USER guest tolmoon tolsun Ronnie Reagan
Possible Numeric Server Replies:
ERR_NEEDMOREPARAMS
ERR_ALREADYREGISTRED
The USER command is used at the beginning of the connection to specify the username, hostname, servername, and real name of the new user. The hostname is the name of the client’s machine.

QUIT Command
Command: QUIT
Parameters: [<quit message>]
Example: QUIT got to go, goodbye!
A client session is ended with a quit message. The server must close the connection to a client that sends a QUIT message. If a "Quit Message" is given, it will be sent instead of the default message, which is the nickname. If a client connection is closed without issuing a QUIT command (e.g., client dies and EOF occurs on the socket), the server fills in the quit message with some sort of message reflecting the nature of the event.

Servers
The server forms the backbone of Internet Relay Chat, providing a point to which clients may connect to talk to each other.

Clients
A client is anything connecting to a server that is not another server. The server must have the following information about all clients:

A unique client identifier (a.k.a. the client "nickname," a string between 1 and 9 characters)
The real name of the host that the client is running on
The username of the client on that host
The server to which the client is connected
