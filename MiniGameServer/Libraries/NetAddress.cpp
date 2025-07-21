#include "pch.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN sockaddr) {
}

NetAddress::NetAddress(wstring ip, uint16_t port) {
	::memset(&_sockaddr, 0, sizeof(_sockaddr));
	_sockaddr.sin_family = AF_INET;
	_sockaddr.sin_addr = Ip2Address(ip.c_str());
	_sockaddr.sin_port = ::htons(port);
}

std::wstring NetAddress::GetIpAddress() {
	WCHAR buffer[100];
	::InetNtopW(AF_INET, &_sockaddr.sin_addr, buffer, 100);
	return wstring(buffer);
}

IN_ADDR NetAddress::Ip2Address(const WCHAR* ip) {
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
