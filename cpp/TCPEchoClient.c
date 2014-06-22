#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const size_t BUFSIZE = 4096;

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

// 和服务器端建立连接
int connectServer(const char* servIP, in_port_t servPort);

// 主动发送消息到服务器，等待服务器回显
int chat(int serverSocket);

// 将消息发送到服务器
int sendMessage(int serverSocket, const char* msg, size_t len);

// 从服务器接收消息。这里最多只接收BUFSIZE大小的消息
int recieveMessage(int serverSocket, char* msgBuffer);

int main(int argc, const char *argv[]) {
	if (argc < 1) {
		printf("Usage: %s <Server IP> [<Server Port>]\n", argv[0]);
		return -1;
	}
	
	const char *servIP = argc > 1 ? argv[1] : "127.0.0.1";
	in_port_t servPort = argc > 2 ? atoi(argv[2]) : 5000;
	
	int sock = connectServer(servIP, servPort);
	if (sock < 0) {
		return sock;
	}
	
	int ret = chat(sock);
	
	close(sock);
	
	return ret;
}

// 和服务器端建立连接
int connectServer(const char* servIP, in_port_t servPort) {
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		printf("socket() failed\n");
		return -1;
	}

	// Construct the server address structure
	struct sockaddr_in servAddr;
	// Zero out structure
	memset(&servAddr, 0, sizeof(servAddr)); 

	servAddr.sin_family = AF_INET;
	
	int rtnVal = convertIPAddress(servIP, &servAddr);
	if (rtnVal != 0) {
		return rtnVal;
	}
	
	// Server port
	servAddr.sin_port = htons(servPort);
  
	// Establish the connection to the echo server
	if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		printf("connect() failed\n");
		return -1;
	}
	
	return sock;	
}

// 主动发送消息到服务器，等待服务器回显
int chat(int serverSocket) {
	char msg[BUFSIZE] = { 0 };
	while (1) {
		printf("client>> ");
		if (gets(msg) == NULL) {
			printf("\n");
			continue;
		}
		
		// 当输入quit或者exit的时候退出客户端
		if (strcmp(msg, "quit") == 0 || strcmp(msg, "exit") == 0) {
			return 0;
		}
		
		// 必须以一个NULL结束消息，因为服务器端需要根据这个来判断接收消息是否结束
		int ret = sendMessage(serverSocket, msg, strlen(msg) + 1);
		if (ret != 0) {
			continue;
		}
		
		recieveMessage(serverSocket, msg);
		
		printf("server>> %s\n", msg);
	}
}

// 将消息发送到服务器
int sendMessage(int serverSocket, const char* msg, size_t len) {
	size_t totalBytes = 0;
	size_t startPos = 0;
	while (totalBytes < len) {
		ssize_t numBytes = send(serverSocket, msg + startPos, len - totalBytes, 0);
		if (numBytes < 0) {
			printf("send() failed\n");
			return -1;
		}
		
		totalBytes += numBytes;
		startPos += numBytes;
	}	
	return 0;
}

// 从服务器接收消息。这里最多只接收BUFSIZE大小的消息
int recieveMessage(int serverSocket, char* msgBuffer) {
	size_t totalBytesRcvd = 0;
	while (totalBytesRcvd < (BUFSIZE - 1)) {
		ssize_t numBytes = recv(serverSocket, msgBuffer, BUFSIZE - 1, 0);
		if (numBytes < 0) {
			printf("recv() failed\n");
			return -1;
		} else if (numBytes == 0) {
			printf("connection closed prematurely\n");
		}
		
		totalBytesRcvd += numBytes; // Keep tally of total bytes
		if (msgBuffer[totalBytesRcvd - 1] == '\0') {
			break;
		}
		if (numBytes == 0) {
			break;
		}
	}
	
	return totalBytesRcvd;
}
