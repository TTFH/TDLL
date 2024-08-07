#include <string.h>
#include "lua_utils.h"

#define CURL_STATICLIB
#include <curl/curl.h>

void LuaPushList(lua_State* L, std::vector<int> list) {
	td_lua_createtable(L, list.size(), 0);
	for (size_t i = 0; i < list.size(); i++) {
		lua_pushinteger(L, list[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

void LuaPushVec3(lua_State* L, Vec3 v) {
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
	LuaPushVec3(L, transform.pos);
	lua_settable(L, -3);

	td_lua_pushstring(L, "rot");
	LuaPushQuat(L, transform.rot);
	lua_settable(L, -3);
}

Vec3 LuaToVec3(lua_State* L, int index) {
	Vec3 v;
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

Quat LuaToQuat(lua_State* L, int index) {
	Quat q;
	lua_rawgeti(L, index, 1);
	q.x = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 2);
	q.y = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 3);
	q.z = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, index, 4);
	q.w = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return q;
}

Transform LuaToTransform(lua_State* L, int index) {
	Transform transform;
	lua_getfield(L, index, "pos");
	transform.pos = LuaToVec3(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "rot");
	transform.rot = LuaToQuat(L, -1);
	lua_pop(L, 1);
	return transform;
}

void LuaPushFunction(lua_State* L, const char* name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_setglobal(L, name);
}

void LuaPushEmptyTable(lua_State* L) {
	td_lua_createtable(L, 0, 0);
}

bool LuaIsGlobalDefined(lua_State* L, const char* name) {
	lua_getglobal(L, name);
	bool defined = !lua_isnil(L, -1);
	lua_pop(L, 1);
	return defined;
}

size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* response) {
	response->append((char*)contents, size * nmemb);
	return size * nmemb;
}

int HttpRequest(const char* method, const char* endpoint, std::map<std::string, std::string> headers, const char* request, const char* cookie_file, std::string& response) {
	int http_code = 500;
	response.clear();
	CURL* curl = curl_easy_init();
	if (curl != nullptr) {
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
		curl_easy_setopt(curl, CURLOPT_URL, endpoint);
		if (request != nullptr && strlen(request) > 0)
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);

		curl_slist* curl_headers = nullptr;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
			std::string header = it->first + ": " + it->second;
			curl_headers = curl_slist_append(curl_headers, header.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

		curl_easy_setopt(curl, CURLOPT_CAINFO, "ca-bundle.crt"); // C:\msys64\usr\ssl\certs
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
		response = "Failed to initialize curl";
	return http_code;
}
