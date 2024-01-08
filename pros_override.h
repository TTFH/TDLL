#ifndef _PROS_OVERRIDE_H
#define _PROS_OVERRIDE_H

#include <stdint.h>

#define EXPORT extern "C" __declspec(dllexport)

#define HYDRA_OK 0x87;

const bool TELEMETRY_ENABLED = false;

EXPORT uint64_t* prosdk_user_get_kernel_session_id_visual_alias(uint64_t* param1) {
	return NULL;
}

EXPORT uint64_t prosdk_user_get_connection_state(uint64_t* param1) {
	return 0;
}

EXPORT uint64_t prosdk_user_connect_xbox(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_steam(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_psn_token(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_epic_online_services(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_developer(uint64_t* param1, uint64_t param2, int param3) {
	return HYDRA_OK;
}

EXPORT void prosdk_update() {}

EXPORT void prosdk_term() {}

EXPORT uint64_t prosdk_telemetry_user_event(uint64_t* param1, uint64_t param2, uint32_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT bool prosdk_telemetry_deprecated_event(uint64_t param1, uint32_t param2, uint64_t param3) {
	return false;
}

EXPORT uint64_t prosdk_mods_upload_mod_image(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_mods_unsubscribe_from_mod(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t* param5) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_mods_subscribe_to_mod(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t* param5) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_mods_rate_mod(uint64_t* param1, uint64_t* param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_mods_get_connection_state(uint64_t* param1) {
	return 0;
}

EXPORT void prosdk_mods_disconnect(uint64_t* param1) { }

EXPORT uint32_t prosdk_mods_connect(uint64_t* param1, uint64_t param2) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_mods_add_mod_version(uint64_t* param1, uint64_t* param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_mods_add_mod(uint64_t* param1, uint64_t* param2,  uint64_t* param3, uint64_t param4, uint64_t param5) {
	return HYDRA_OK;
}

EXPORT void prosdk_is_executing_requests() {}

EXPORT uint64_t prosdk_init(uint64_t* param1, uint64_t param2, int param3) {
	return 0;
}

EXPORT uint64_t prosdk_http_request_get_framed(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) {
	return 0;
}

EXPORT uint64_t prosdk_http_request_get(uint64_t* param1, uint64_t param2, uint64_t param3) {
	return 0;
}

EXPORT uint64_t* prosdk_get_error_message() {
	return NULL;
}

EXPORT uint32_t prosdk_disconnect_everything() {
	return 123;
}

EXPORT uint64_t prosdk_container_transfer_upload_rawdata(uint64_t* param1, uint64_t param2, uint64_t* param3, uint64_t param4, uint64_t param5) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_authorization_user_create(uint64_t* param1) {
	return 0;
}

EXPORT const char* prosdk_account_get_status(uint64_t* param1) {
	return "Hello World ABC";
}

EXPORT uint64_t prosdk_account_get_qr_code(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT void prosdk_account_connect(uint64_t* param1, uint64_t param2) { }

EXPORT void hydra5_diagnostics_release_crash_reporter_user_token(uint64_t* param1) {}

EXPORT uint64_t hydra5_diagnostics_get_crash_reporter_user_token(uint64_t param1) {
	return 0;
}

#endif
