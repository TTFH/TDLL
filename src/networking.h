#ifndef _NETWORKING_H
#define _NETWORKING_H

#include <map>
#include <string>

int HttpRequest(const char*, const char*, std::map<std::string, std::string>, const char*, const char*, std::string&);

class Broadcast {
private:
	int port;
	SOCKET udp_socket;
	sockaddr_in send_addr;
	sockaddr_in recv_addr;
public:
	Broadcast();
	void Init(int port);
	void Send(const char* message);
	void Receive(char* buffer, int buffer_size);
};

#endif
