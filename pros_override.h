#ifndef _PROS_OVERRIDE_H
#define _PROS_OVERRIDE_H

#include <stdint.h>

#define EXPORT extern "C" __declspec(dllexport)

#define HYDRA_OK 100;

const bool TELEMETRY_ENABLED = false;

EXPORT uint64_t prosdk_authorization_user_create(uint64_t* param1) {
	return 0;
}

EXPORT uint64_t prosdk_user_connect_developer(uint64_t* param1, uint64_t param2, int param3) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_epic_online_services(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_steam(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_xbox(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_connect_psn_token(uint64_t* param1) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_user_get_connection_state(uint64_t* param1) {
	return 0;
}

EXPORT uint64_t* prosdk_user_get_kernel_session_id_visual_alias(uint64_t* param1) {
	return nullptr;
}

EXPORT uint64_t prosdk_account_connect(uint64_t* param1, uint64_t param2) {
	return HYDRA_OK;
}

EXPORT int prosdk_account_get_qr_code(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT const char* prosdk_account_get_status(uint64_t* param1) {
	return "Hello World ABC";
}

// aka prosdk_mods_mod_by_url_result_release
EXPORT void prosdk_mods_mod_list_result_release(uint64_t* param1) { }

EXPORT void prosdk_mods_mod_item_release(uint64_t param1) { }

EXPORT void prosdk_mods_mod_complaint_type_array_release(uint64_t* param1) { }

EXPORT int prosdk_mods_connect(uint64_t* param1, uint64_t param2) {
	return HYDRA_OK;
}

EXPORT void prosdk_mods_disconnect(uint64_t param1) { }

EXPORT uint64_t prosdk_mods_get_connection_state(uint64_t* param1) {
	return 0;
}

EXPORT int prosdk_mods_get_mod(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT int prosdk_mods_get_mods_by_search_filter(uint64_t* param1, uint32_t* param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT uint64_t* prosdk_mods_get_complaint_type_array(uint64_t* param1) {
	return nullptr;
}

EXPORT int prosdk_mods_subscribe_to_mod(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t* param5) {
	return HYDRA_OK;
}

EXPORT int prosdk_mods_unsubscribe_from_mod(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t* param5) {
	return HYDRA_OK;
}

EXPORT prosdk_mods_submit_mod_complaint(uint64_t* param1, uint64_t* param2, uint64_t* param3, uint64_t param4, uint64_t param5) {
	return HYDRA_OK;
}

EXPORT int prosdk_mods_add_mod() {
	return HYDRA_OK;
}

EXPORT int prosdk_mods_add_mod_version(uint64_t* param1, uint64_t* param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT int prosdk_mods_upload_mod_image(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT int prosdk_mods_rate_mod(uint64_t* param1, uint64_t* param2, uint64_t param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT uint64_t prosdk_telemetry_user_event(uint64_t* param1, uint64_t param2, int param3, uint64_t param4) {
	return HYDRA_OK;
}

EXPORT int prosdk_init(uint64_t* param1, uint64_t param2, int param3) {
	return 0;
}

EXPORT void prosdk_update() { }

EXPORT int prosdk_disconnect_everything() {
	return 0;
}

EXPORT void prosdk_term() { }

EXPORT void prosdk_is_executing_requests() { }

EXPORT uint64_t* prosdk_get_error_message() {
	return nullptr;
}

EXPORT int prosdk_http_request_get(uint64_t* param1, uint64_t param2, uint64_t param3) {
	return 120;
}

EXPORT int prosdk_http_request_get_framed(uint64_t* param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) {
	return 120;
}

EXPORT uint64_t prosdk_container_transfer_upload_rawdata(uint64_t* param1, uint64_t param2, uint64_t* param3, uint64_t param4, uint64_t param5) {
	return HYDRA_OK;
}

EXPORT int hydra5_diagnostics_crash_dump_upload_token(uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4) {
	return 0;
}

EXPORT int hydra5_diagnostics_get_crash_reporter_token(void** param1) {
	return 0;
}

// aka hydra5_client_standalone_get_code_release
EXPORT void hydra5_diagnostics_release_crash_reporter_token(uint64_t param1) { }

#endif
