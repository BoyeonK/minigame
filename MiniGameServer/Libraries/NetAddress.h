#pragma once
#include <string>
#include <cstdint>

class NetAddress {
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockaddr);
	NetAddress(std::wstring ip, uint16_t port);

	SOCKADDR_IN&	GetSockAddr() { return _sockaddr; }
	std::wstring	GetIpAddress();
	uint16_t		GetPort() { return ::ntohs(_sockaddr.sin_port); }

	static IN_ADDR	Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _sockaddr = {};
};

