server: 
	g++ -std=c++11 ./src/TCPServer.cpp ./src/TCPClient.cpp chat_server.cpp  ./src/Client.cpp -o server -lpthread

client:
	g++ -std=c++11 ./src/TCPServer.cpp ./src/TCPClient.cpp chat_client.cpp  ./src/Client.cpp -o client -lpthread

all:
	g++ -std=c++11 ./src/TCPServer.cpp ./src/TCPClient.cpp ./src/Client.cpp chat_server.cpp  -o server -lpthread -Wall -Wextra
	g++ -std=c++11 ./src/TCPServer.cpp ./src/TCPClient.cpp ./src/Client.cpp chat_client.cpp ./src/chat_client/InterfaceClient.cpp -o client -lpthread -lncurses -Wall -Wextra