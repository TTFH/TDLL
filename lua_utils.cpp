#include <string.h>
#include <curl/curl.h>
#include "lua_utils.h"

void LuaPushList(lua_State* L, std::vector<int> list) {
	td_lua_createtable(L, list.size(), 0);
	for (size_t i = 0; i < list.size(); i++) {
		lua_pushinteger(L, list[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

void LuaPushVector(lua_State* L, Vector v) {
	td_lua_createtable(L, 3, 0);

	lua_pushnumber(L, v.x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, v.y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, v.z);
	lua_rawseti(L, -2, 3);
}

void LuaPushQuat(lua_State* L, Quat q) {
	td_lua_createtable(L, 4, 0);

	lua_pushnumber(L, q.x);
	lua_rawseti(L, -2, 1);

	lua_pushnumber(L, q.y);
	lua_rawseti(L, -2, 2);

	lua_pushnumber(L, q.z);
	lua_rawseti(L, -2, 3);

	lua_pushnumber(L, q.w);
	lua_rawseti(L, -2, 4);
}

void LuaPushTransform(lua_State* L, Transform transform) {
	td_lua_createtable(L, 0, 2);

	td_lua_pushstring(L, "pos");
	LuaPushVector(L, transform.pos);
	lua_settable(L, -3);

	td_lua_pushstring(L, "rot");
	LuaPushQuat(L, transform.rot);
	lua_settable(L, -3);
}

Vector LuaToVector(lua_State* L, int index) {
	Vector v;
	lua_rawgeti(L, index, 1);
	v.x = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 2);
	v.y = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 3);
	v.z = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return v;
}

void LuaPushFuntion(lua_State* L, const char* name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_setglobal(L, name);
}

size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* response) {
	response->append((char*)contents, size * nmemb);
	return size * nmemb;
}

int HttpRequest(const char* endpoint, std::map<std::string, std::string> headers, const char* request, std::string& response) {
	CURL* curl;
	CURLcode res;
	int http_code = 500;
	response.clear();
	curl = curl_easy_init();
	if (curl != NULL) {
		curl_easy_setopt(curl, CURLOPT_URL, endpoint);
		if (request != NULL && strlen(request) > 0)
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);

		curl_slist* curl_headers = NULL;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
			std::string header = it->first + ": " + it->second;
			curl_headers = curl_slist_append(curl_headers, header.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		else
			response = curl_easy_strerror(res);
		
		curl_slist_free_all(curl_headers);
		curl_easy_cleanup(curl);
	} else
		response = "Failed to initialize curl";
	return http_code;
}
