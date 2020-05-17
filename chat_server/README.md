andy velazquez



Simple chat server that can broadcast, list all who are in the server, message, and exit

usage
%m name message
%l 
%e
%b message

%m can send a messge up to 10 clients
%l will list all clients currently on the server
%e will exit
%b will send a message to everyclient on the server

server uses a hash table to store all the names of the clients. the client must have a unique name

run insturctions
run make

for client:
./cclient name server_name portname

for server:
./server [port number]
