# Client_Server_Chatroom
These programs enable real-time chat between clients and a server. Clients connect, send messages, and exit using "QUIT." The server manages connections, validates usernames, and relays messages. Features include non-blocking sockets and dynamic message handling.

Criteria:		 	
  At level 1, The implemented client-server application allows anonymous users to chat. The server listens on port 25565. Clients provide a username and server name when starting, using the same port as the server. Usernames are local and not necessarily unique. 
  The server, at Level 2, handles the USER command for connection, verifying unique names and providing error messages. Clients can disconnect with a QUIT message.
  At Level 3, the server maintains a list of unique names and handles multi-user interactions. It sends messages to all clients connected, ensuring a dynamic chat environment. The USER command specifies username, hostname, servername, and real name during connection, with possible server replies for insufficient parameters or existing registrations. The application facilitates real-time communication between multiple clients without the need for channels.

Commands:		
USER Command			
  Command: USER
  Parameters: <username> <hostname> <servername> <realname>
  Example: USER guest tolmoon tolsun Ronnie Reagan
  Possible numeric server replies:				
    ERR_NEEDMOREPARAMS
    ERR_ALREADYREGISTRED				
  The USER message is used at the beginning of connection to specify the username, hostname, servername and realname of the new user. The hostname is the name of the client’s machine.	

QUIT Command					
  Command: QUIT
  Parameters: [<quit message>]
  Example: QUIT got to go, goodbye! A client session is ended with a quit message. The server must close the connection to a client which sends a QUIT message. If a "Quit Message" is given, this will be sent instead of the default message, the nickname.		
  If, for some other reason, a client connection is closed without the client issuing a QUIT command (e.g. client dies and EOF occurs on socket), the server is required to fill in the quit message with some sort of message reflecting the nature of the event which caused it to happen.

Servers:	
The server forms the backbone of Internet Relay Chat, providing a point to which clients may connect to talk to each other.

Clients:
A client is anything connecting to a server that is not another server. The server MUST have the following information about all clients:				
-A unique client identifier (a.k.a. the client "nickname", a string between 1 and 9 characters) 
-The real name of the host that the client is running on
-The username of the client on that host
-The server to which the client is connected
					
How to Run the Application:
-To begin, I ssh and log into a server where the firewall has been dropped:
  ssh aw01
-Next step is to compile the two programs:
  g++ client.cpp -o client -std=c++11
  g++ server.cpp -o server -std=c++11
-Execute the server program:
  ./server
-Execute the client program:
  ./client <server_ip> <nickname> <real_name> <username>
-Chat away
