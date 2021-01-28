# UDP-Chatroom
This repository stores code of a simple chatroom program based on the application of UDP sockets in C++.

## Features
The program sets a simple chatroom within a server machine so that the diffrent clients can access through the specified port to chat with each other. Once the server will receive the message once a client gets into the room, speaks and leaves the room, and broadcast the message to all other clients.



## Installation
+ Clone this repository with 
```
git clone https://github.com/charleschang213/UDP-Chatroom.git
```
+ Get into the reposotory then use ```make```
+ Deploy the executive ```server``` to the server and ```client``` to the client machines.

## Usage
+ The server is started by the command
```
./server <port>
```
So that the chatroom hill be hosted on specific port.
+ The client is started by the command
```
./client <server-address> <server-port> <username>
```
And into will get into the chatroom. In the chatroom the client user can just type to speak.