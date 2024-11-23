#ifndef _NETWORKING_H
#define _NETWORKING_H

#include <map>
#include <vector>
#include <string>

#define CURL_STATICLIB
#include <curl/curl.h>

struct HTTP_Response {
	int id;
	std::string url;
	int status;
	std::string body;
};

int HttpRequest(const char*, const char*, std::map<std::string, std::string>, const char*, const char*, std::string&);

int AddRequest(const char* method, const char* endpoint, std::map<std::string, std::string> headers, const char* request_body);
std::vector<HTTP_Response> FetchResponses();

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
