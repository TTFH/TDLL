#include <stdio.h>
#include <memory>
#include <stdexcept>

#include <winsock2.h>

#include "networking.h"

size_t write_callback(char* contents, size_t size, size_t nmemb, std::string* response) {
	response->append(contents, size * nmemb);
	return size * nmemb;
}

int HttpRequest(const char* method, const char* endpoint, std::map<std::string, std::string> headers, const char* request_body, const char* cookie_file, std::string& response) {
	int http_code = 500;
	response.clear();
	CURL* curl = curl_easy_init();
	if (curl != nullptr) {
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
		curl_easy_setopt(curl, CURLOPT_URL, endpoint);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		if (request_body != nullptr && strlen(request_body) > 0)
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);

		curl_slist* curl_headers = nullptr;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
			std::string header = it->first + ": " + it->second;
			curl_headers = curl_slist_append(curl_headers, header.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

		FILE* ca_bundle = fopen("ca-bundle.crt", "r");
		if (ca_bundle != nullptr) {
			fclose(ca_bundle);
			curl_easy_setopt(curl, CURLOPT_CAINFO, "ca-bundle.crt"); // C:\msys64\usr\ssl\certs
		}

		if (cookie_file != nullptr && strlen(cookie_file) > 0) {
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file);
			curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file);
		}

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		CURLcode res = curl_easy_perform(curl);
		if (res == CURLE_OK)
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		else
			response = curl_easy_strerror(res);

		curl_slist_free_all(curl_headers);
		curl_easy_cleanup(curl);
	} else
		response = "Failed to initialize Curl.";
	return http_code;
}

int request_count = 0;
CURLM* multi_handle = nullptr;
std::map<CURL*, HTTP_Response> running_requests;

int AddRequest(const char* method, const char* endpoint, std::map<std::string, std::string> headers, const char* request_body) {
	if (multi_handle == nullptr)
		multi_handle = curl_multi_init();
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(curl, CURLOPT_URL, endpoint);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	if (request_body != nullptr && strlen(request_body) > 0)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);

	curl_slist* curl_headers = nullptr;
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
		std::string header = it->first + ": " + it->second;
		curl_headers = curl_slist_append(curl_headers, header.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

	FILE* ca_bundle = fopen("ca-bundle.crt", "r");
	if (ca_bundle != nullptr) {
		fclose(ca_bundle);
		curl_easy_setopt(curl, CURLOPT_CAINFO, "ca-bundle.crt");
	}

	HTTP_Response response;
	response.url = endpoint;
	response.id = request_count++;
	running_requests[curl] = response;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &running_requests[curl].body);

	curl_multi_add_handle(multi_handle, curl);
	return response.id;
}

std::vector<HTTP_Response> FetchResponses() {
	int running_handles;
	std::vector<HTTP_Response> responses;
	curl_multi_perform(multi_handle, &running_handles);
	int msgs_left;
	do {
		CURLMsg* msg = curl_multi_info_read(multi_handle, &msgs_left);
		if (msg != nullptr && msg->msg == CURLMSG_DONE) {
			CURL* curl = msg->easy_handle;
			HTTP_Response response = running_requests[curl];
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
			printf("Request to %s finished with status code %d\n", response.url.c_str(), response.status);
			responses.push_back(response);
			running_requests.erase(curl);
			curl_multi_remove_handle(multi_handle, curl);
			curl_easy_cleanup(curl);
		}
	} while (msgs_left > 0);
	return responses;
}

Broadcast::Broadcast() {
	port = 0;
	udp_socket = INVALID_SOCKET;
}

void Broadcast::Init(int port) {
	this->port = port;
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udp_socket == INVALID_SOCKET)
		throw std::runtime_error("Socket creation failed");

	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(port);
	send_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	BOOL broadcast = TRUE;
	if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR)
		throw std::runtime_error("Socket option failed");

	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(port);
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(udp_socket, (SOCKADDR*)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR)
		throw std::runtime_error("Binding failed");
}

void Broadcast::Send(const char* message) {
	if (sendto(udp_socket, message, strlen(message), 0, (SOCKADDR*)&send_addr, sizeof(send_addr)) < 0)
		throw std::runtime_error("Failed to send broadcast message");
}

void Broadcast::Receive(char* buffer, int buffer_size) {
	int recv_addr_size = sizeof(recv_addr);
	int bytes_received = recvfrom(udp_socket, buffer, buffer_size, 0, (SOCKADDR*)&recv_addr, &recv_addr_size);
	if (bytes_received == SOCKET_ERROR)
		throw std::runtime_error("Failed to receive broadcast message");
	buffer[bytes_received] = '\0';
}
