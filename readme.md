# README - Assignment 2  
**Bîrleanu Teodor Matei 324 CA**  
**TCP and UDP Client-Server Application for Message Management**

---

## Overview  
This project implements a client-server application for message management using TCP and UDP protocols. The design is based on the skeleton code provided in Laboratory 7, with enhancements for client-server communication and message handling. Two sleep-days were utilized during the development process.

---

## Protocol Design  
The communication protocol encapsulates messages into a structured format containing:  
- **Topic**: The subject of the message.  
- **Message Type**: Specifies the type of the message (e.g., subscribe, unsubscribe, etc.).  
- **Content**: The message's actual data.  
- **Client IP** and **Port**: For UDP messages.  

### Client Details  
Each client maintains:  
- A unique **ID**.  
- A **File Descriptor** for communication.  
- Subscribed **Topics**.  
- A "semaphore" indicating whether the client is connected.

---

## Client Implementation  
The client implementation is based on the Laboratory 7 skeleton code. Key functionalities include:

1. **TCP Socket Initialization**  
   - A TCP socket is created to connect to the server.  
   - Upon connection, a message containing the client ID is sent to the server.  

2. **Multiplexing**  
   - `poll` is used for handling input from:  
     - **Keyboard**: User commands.  
     - **Server**: Incoming messages.  

3. **Command Handling**  
   - **Exit**:  
     - Sends a message of type `0` to the server and terminates the client program.  
   - **Subscribe**:  
     - Sends a message of type `1` with the topic and client ID in the message structure.  
   - **Unsubscribe**:  
     - Sends a message of type `2` with the topic and client ID.  
   - **Server Messages**:  
     - Processes server responses or displays them as required.

---

## Server Implementation  
The server is also built upon the Laboratory 7 skeleton code. Key functionalities include:

1. **Socket Initialization**  
   - File descriptors are created for both TCP and UDP sockets.  

2. **Multiplexing**  
   - `poll` is used to handle input from:  
     - **Keyboard**: For administrative commands.  
     - **TCP Sockets**: Communication with clients.  
     - **UDP Sockets**: Incoming UDP messages.  

3. **Command Handling**  
   - **Exit**:  
     - Sends an "exit" message to all connected clients and shuts down the server.  
   - **New Connections**:  
     - Accepts new TCP connections, adds the client to the file descriptor list, and checks for duplicate IDs to prevent conflicts.  
   - **UDP Messages**:  
     - Parses incoming UDP messages using the `parse_udp` function, handling:  
       - **SHORT_REAL**  
       - **INT**  
       - **FLOAT**  
       - **STRING**  
     - Constructs structured messages and processes them accordingly.  
   - **Client Commands**:  
     - **Subscribe**: Adds the client to the corresponding topic matrix.  
     - **Unsubscribe**: Removes the client from the topic matrix.  
     - **Exit**: Marks the client as disconnected, removes its file descriptor, and updates the state.

4. **Additional Features**  
   - **Neagle's Algorithm**: Disabled for TCP sockets to improve performance.  
   - Ensures graceful shutdown by closing all file descriptors (TCP and UDP).

---

## Error Handling  
- Errors are handled using the `DIE` macro from Laboratory 7 to ensure proper debugging and program reliability.

---

## Summary  
This client-server application implements robust message handling, supporting subscription management, UDP message parsing, and efficient multiplexing using `poll`. The structured protocol ensures seamless communication between clients and the server, while error handling and connection management maintain system stability.

### Key Features  
- Subscription and unsubscription functionality.  
- Support for multiple message types and structured UDP parsing.  
- Graceful handling of client exits and server shutdown.  
- Efficient multiplexing for simultaneous TCP and UDP communication.
