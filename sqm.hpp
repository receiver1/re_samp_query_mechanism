#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <map>

#include "wrappers.hpp"

#pragma comment(lib, "Ws2_32.lib")

enum e_query_type: uint8_t
{
	QUERYTYPE_SERVERINFO = 'i',
	QUERYTYPE_SERVERRULES = 'r',
	QUERYTYPE_SERVERPING = 'p',
	QUERYTYPE_PLAYERLIST = 'c'
};

class c_sqm
{
	SOCKET _socket {INVALID_SOCKET};
	sockaddr _address;
	const char *_ip;
	u_short _port;
public:
	static c_sqm *singleton()
	{
		static c_sqm instance;
		return &instance;
	}

	/*
	*	Initialize WinSock2
	*/
	void initialize(const char *ip, u_short port)
	{
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != 0)
			throw std::runtime_error("Failed to initiates WinSock.");

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(ip);
		address.sin_port = htons(port);

		_socket = socket(address.sin_family, SOCK_DGRAM, IPPROTO_UDP);
		if (_socket == -1)
			throw std::runtime_error("Failed to create socket.");

		u_long param = 1;
		result = ioctlsocket(_socket, FIONBIO, &param);
		if (result < 0)
			throw std::runtime_error("Failed to set socket non-blocking.");

		_address = reinterpret_cast<sockaddr&>(address);
		_ip = ip;
		_port = port;
	}

	/*
	*	Translate IP to bytes
	*/
	std::string translate_ip(std::string ip)
	{
		std::string result, part;
		std::stringstream it(ip);
		while (std::getline(it, part, '.')) {
			result += atoi(part.c_str());
		}
		return result;
	}

	/*
	*	Send query and receive response
	*/
	std::stringstream query(e_query_type type, int timeout)
	{
		std::string packet {"SAMP"};

		packet += translate_ip(_ip);
		packet += _port & 0xFFu;
		packet += _port >> 8 & 0xFFu;
		packet += type;

		sendto(_socket, packet.c_str(), packet.size(), 0, &_address, sizeof(_address));
		return receive(timeout);
	}

	/*
	*	Waiting for response from server
	*/
	std::stringstream receive(int timeout)
	{
		char buffer[2048];
		sockaddr from;
		socklen_t from_length = sizeof(_address);

		std::stringstream packet {};
		time_t start_time = time(0);
		while (time(0) - start_time < timeout)
		{
			Sleep(1);
			int received_bytes = recvfrom(_socket, buffer, sizeof(buffer), 0, &from, &from_length);
			if (received_bytes > 11) {
				packet = std::stringstream(std::string(buffer, received_bytes));
				start_time = time(0);
			}
		}
		return packet;
	}

	template<typename T> 
	T handle(std::stringstream ss)
	{
		return T {ss};
	}
};
