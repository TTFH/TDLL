#ifndef _PROS_PROXY_H
#define _PROS_PROXY_H

#ifdef _MSC_VER
#define PROXY(target_name, exp_name) __pragma(comment(linker, "/export:" #exp_name "=" #target_name "." #exp_name))
#else
#define PROXY(target_name, exp_name) asm(".section .drectve\n\t.ascii \" -export:" #exp_name "=" #target_name "." #exp_name "\" ")
#endif

const bool TELEMETRY_ENABLED = true;

PROXY(amateurs, prosdk_user_get_kernel_session_id_visual_alias);
PROXY(amateurs, prosdk_user_get_connection_state);
PROXY(amateurs, prosdk_user_connect_xbox);
PROXY(amateurs, prosdk_user_connect_steam);
PROXY(amateurs, prosdk_user_connect_psn_token);
PROXY(amateurs, prosdk_user_connect_epic_online_services);
PROXY(amateurs, prosdk_user_connect_developer);
PROXY(amateurs, prosdk_update);
PROXY(amateurs, prosdk_term);
PROXY(amateurs, prosdk_telemetry_user_event);
PROXY(amateurs, prosdk_telemetry_deprecated_event);
PROXY(amateurs, prosdk_mods_upload_mod_image);
PROXY(amateurs, prosdk_mods_unsubscribe_from_mod);
PROXY(amateurs, prosdk_mods_subscribe_to_mod);
PROXY(amateurs, prosdk_mods_rate_mod);
PROXY(amateurs, prosdk_mods_get_connection_state);
PROXY(amateurs, prosdk_mods_disconnect);
PROXY(amateurs, prosdk_mods_connect);
PROXY(amateurs, prosdk_mods_add_mod_version);
PROXY(amateurs, prosdk_mods_add_mod);
PROXY(amateurs, prosdk_is_executing_requests);
PROXY(amateurs, prosdk_init);
PROXY(amateurs, prosdk_http_request_get_framed);
PROXY(amateurs, prosdk_http_request_get);
PROXY(amateurs, prosdk_get_error_message);
PROXY(amateurs, prosdk_disconnect_everything);
PROXY(amateurs, prosdk_container_transfer_upload_rawdata);
PROXY(amateurs, prosdk_authorization_user_create);
PROXY(amateurs, prosdk_account_get_status);
PROXY(amateurs, prosdk_account_get_qr_code);
PROXY(amateurs, prosdk_account_connect);
PROXY(amateurs, hydra5_diagnostics_release_crash_reporter_user_token);
PROXY(amateurs, hydra5_diagnostics_get_crash_reporter_user_token);

#endif
