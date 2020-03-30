#pragma once
#include <iostream>
#include <vector>
#include "../clsocket/src/ActiveSocket.h" 
#pragma comment(lib, "ws2_32.lib")

class Client
{
public:
	Client(std::string ip,int port);
	~Client();
	bool init();
	bool send(std::vector<uint8_t> &buffer);
	bool send(uint8_t* buffer, size_t length);
	std::pair<bool, std::vector<uint8_t>> receive();
private:
	unsigned int checksum(uint8_t* data, size_t length);
	CActiveSocket socket_;
	std::string server_ip_;
	int server_port_;
};