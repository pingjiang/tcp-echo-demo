#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const ssize_t BUFSIZE = 4096;
static const int MAXPENDING = 5;

// 将IP转化为结构体
int convertIPAddress(const char* ip, struct sockaddr_in *addr) {
	int rtnVal = inet_pton(AF_INET, ip, &((*addr).sin_addr.s_addr));
	if (rtnVal == 0) {
		printf("inet_pton() failed, invalid address string\n");
		return -1;
	} else if (rtnVal < 0) {
		printf("inet_pton() failed\n");
		return -1;
	}
	
	return 0;
}

void ipAddressToString(struct sockaddr_in *clntAddr, char clntName[INET_ADDRSTRLEN]) {
	if (inet_ntop(AF_INET, &(clntAddr->sin_addr.s_addr), clntName, INET_ADDRSTRLEN) != NULL) {
		printf("Handling client %s:%d\n", clntName, ntohs(clntAddr->sin_port));
	} else {
		puts("Unable to get client address\n");
	}
}

// 建立服务器端套接字
int createServerSocket(const char* servIP, in_port_t servPort);

// 处理客户端连接
void serverProcess(int clientSocket);

// 接收请求消息
int receiveRequest(int clientSocket, char* buffer);

// 发送响应消息
int sendResponse(int clientSocket, const char* buffer, ssize_t len);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("no port argument provide, using default port %d\n", 5000);
	}
	const char* servIP = (argc > 1 ? argv[1] : "127.0.0.1");
	in_port_t servPort = (argc > 2 ? atoi(argv[2]) : 5000);

	int servSock = createServerSocket(servIP, servPort);
	if (servSock < 0) {
		printf("create server socket failed\n");
		return -1;
	}
	
	// printf("create server socket success, server socket=%d\n", servSock);
	
	for (;;) { // Run forever
		struct sockaddr_in clntAddr; // Client address
		// Set length of client address structure (in-out parameter)
		socklen_t clntAddrLen = sizeof(clntAddr);
		// Wait for a client to connect
		int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
		if (clntSock < 0) {
			printf("error: accept() failed, errno=%d\n", errno);perror("accept() failed");
			close(servSock);
			close(clntSock);
			return -1;
		}
			
		// clntSock is connected to a client!
		char clntName[INET_ADDRSTRLEN]; // String to contain client address
		ipAddressToString(&clntAddr, clntName);
		
		serverProcess(clntSock);
	}
	
	close(servSock);
	
	return 0;
}

// 建立服务器端套接字
int createServerSocket(const char* servIP, in_port_t servPort) {
	// Create socket for incoming connections
	int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Socket descriptor for server
	if (servSock < 0) {
		printf("error: socket() failed\n");
		return -1;
	}
		
	// Construct local address structure
	struct sockaddr_in servAddr;			//Local address
	memset(&servAddr, 0, sizeof(servAddr));		//Zero out structure
	servAddr.sin_family = AF_INET;			//IPv4 address family
	//servAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//Any incoming interface
	servAddr.sin_port = htons(servPort);		//Local port
	
	int ret = convertIPAddress(servIP, &servAddr);
	if (ret != 0) {
		return ret;
	}

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0) {
		printf("error: bind() failed\n");
		return -1;
	}
		
	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0) {
		printf("error: listen() failed\n");
		return -1;
	}

	printf("server running on %s:%d, socket=%d ...\n", servIP, servPort, servSock);
	
	return servSock;
}

// 处理客户端连接
void serverProcess(int clientSocket) {
	printf("Handle client socket=%d\n", clientSocket);
	char buffer[BUFSIZE] = { 0 };
	while (1) {
		printf("Handle client socket=%d, receiving request ...\n", clientSocket);
		ssize_t len = receiveRequest(clientSocket, buffer);
		if (len < 0) {
			break;
		}
		
		if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "exit") == 0) {
			printf("Handle client socket=%d quit\n", clientSocket);
			break;
		}
		
		sendResponse(clientSocket, buffer, len);
	}
	
	close(clientSocket);
	printf("Handle client socket=%d finished\n", clientSocket);
}

// 接收请求消息
int receiveRequest(int clientSocket, char* buffer) {
	ssize_t totalBytes = 0;
	while (totalBytes < BUFSIZE - 1) {
		ssize_t numBytesRcvd = recv(clientSocket, buffer, BUFSIZE, 0);
		if (numBytesRcvd < 0) {
			printf("error: recv() failed\n");
			return -1;
		}
		if (numBytesRcvd == 0) {
			return -1;
		}
		
		totalBytes += numBytesRcvd;
		if (numBytesRcvd == 0 || buffer[totalBytes - 1] == '\0') {
			break;
		}
	}
	
	return totalBytes;
}

// 发送响应消息
int sendResponse(int clientSocket, const char* buffer, ssize_t len) {
	ssize_t totalBytes = 0;
	ssize_t totalSent = 0;
	while(totalBytes < len) {
		ssize_t numBytesSent = send(clientSocket, buffer + totalSent, len - totalSent, 0);
		if (numBytesSent < 0) {
			printf("error: send() failed\n");
			return totalSent;
		}
		
		totalBytes += numBytesSent;
		totalSent += numBytesSent;
	}
	
	return totalSent;
}
