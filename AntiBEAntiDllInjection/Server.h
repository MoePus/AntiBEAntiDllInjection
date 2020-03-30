#pragma once
#include <iostream>
#include <vector>
#include "../clsocket/src/PassiveSocket.h"
#pragma comment(lib, "ws2_32.lib")

class Server
{
public:
	class remote {
	public:
		remote(CActiveSocket* remote);
		~remote();
		void Close()
		{
			if (remote_)
			{
				remote_->Close();
				delete remote_;
				remote_ = 0;
			}
		}
		bool remote_vaild();
		std::pair<bool, std::vector<uint8_t>> receive();
		bool send(std::vector<uint8_t> &buffer);
		bool send(uint8_t* buffer, size_t length);
	private:
		unsigned int checksum(uint8_t* data, size_t length);
		CActiveSocket* remote_;

	};
	Server(std::string ip,int port);
	~Server();
	bool init();
	Server::remote wait_remote();
private:
	std::string server_ip_;
	int server_port_;
	CPassiveSocket socket_;
};