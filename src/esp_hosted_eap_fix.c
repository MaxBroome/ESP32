// ESP-Hosted EAP Weak Stub Fix
//
// The ESP-Hosted library provides weak (4-byte no-op) stubs for all
// esp_eap_client_* and esp_wifi_sta_enterprise_* functions. These stubs
// return ESP_OK without sending any RPCs to the C6 coprocessor, making
// 802.1X enterprise WiFi silently non-functional on the ESP32-P4.
//
// The real implementations exist as esp_*_remote_* variants in the same
// library (esp_hosted_api.c), but the weak stubs never call them.
//
// This file provides strong symbol overrides that forward every call to
// the corresponding remote RPC function, fixing the bug at link time.
// With these in place, the standard Arduino WiFi.begin() enterprise
// overload and all esp_eap_client_* calls work correctly over SDIO.

#include <esp_eap_client.h>
#include <esp_eap_client_remote_api.h>

esp_err_t esp_wifi_sta_enterprise_enable(void) {
    return esp_wifi_remote_sta_enterprise_enable();
}

esp_err_t esp_wifi_sta_enterprise_disable(void) {
    return esp_wifi_remote_sta_enterprise_disable();
}

esp_err_t esp_eap_client_set_identity(const unsigned char *identity, int len) {
    return esp_eap_client_remote_set_identity(identity, len);
}

void esp_eap_client_clear_identity(void) {
    esp_eap_client_remote_clear_identity();
}

esp_err_t esp_eap_client_set_username(const unsigned char *username, int len) {
    return esp_eap_client_remote_set_username(username, len);
}

void esp_eap_client_clear_username(void) {
    esp_eap_client_remote_clear_username();
}

esp_err_t esp_eap_client_set_password(const unsigned char *password, int len) {
    return esp_eap_client_remote_set_password(password, len);
}

void esp_eap_client_clear_password(void) {
    esp_eap_client_remote_clear_password();
}

esp_err_t esp_eap_client_set_new_password(const unsigned char *new_password, int len) {
    return esp_eap_client_remote_set_new_password(new_password, len);
}

void esp_eap_client_clear_new_password(void) {
    esp_eap_client_remote_clear_new_password();
}

esp_err_t esp_eap_client_set_ca_cert(const unsigned char *ca_cert, int ca_cert_len) {
    return esp_eap_client_remote_set_ca_cert(ca_cert, ca_cert_len);
}

void esp_eap_client_clear_ca_cert(void) {
    esp_eap_client_remote_clear_ca_cert();
}

esp_err_t esp_eap_client_set_certificate_and_key(const unsigned char *client_cert, int client_cert_len,
                                                  const unsigned char *private_key, int private_key_len,
                                                  const unsigned char *private_key_password, int private_key_passwd_len) {
    return esp_eap_client_remote_set_certificate_and_key(client_cert, client_cert_len,
                                                          private_key, private_key_len,
                                                          private_key_password, private_key_passwd_len);
}

void esp_eap_client_clear_certificate_and_key(void) {
    esp_eap_client_remote_clear_certificate_and_key();
}

esp_err_t esp_eap_client_set_disable_time_check(bool disable) {
    return esp_eap_client_remote_set_disable_time_check(disable);
}

esp_err_t esp_eap_client_get_disable_time_check(bool *disable) {
    return esp_eap_client_remote_get_disable_time_check(disable);
}

esp_err_t esp_eap_client_set_ttls_phase2_method(esp_eap_ttls_phase2_types type) {
    return esp_eap_client_remote_set_ttls_phase2_method(type);
}

esp_err_t esp_eap_client_set_suiteb_192bit_certification(bool enable) {
    return esp_eap_client_remote_set_suiteb_192bit_certification(enable);
}

esp_err_t esp_eap_client_set_pac_file(const unsigned char *pac_file, int pac_file_len) {
    return esp_eap_client_remote_set_pac_file(pac_file, pac_file_len);
}

esp_err_t esp_eap_client_set_fast_params(esp_eap_fast_config config) {
    return esp_eap_client_remote_set_fast_params(config);
}

esp_err_t esp_eap_client_use_default_cert_bundle(bool use_default_bundle) {
    return esp_eap_client_remote_use_default_cert_bundle(use_default_bundle);
}

esp_err_t esp_eap_client_set_domain_name(const char *domain_name) {
    return esp_eap_client_remote_set_domain_name(domain_name);
}

// esp_eap_client_set_eap_methods is a strong (non-weak) symbol in the hosted
// library that already forwards to the remote implementation — no override needed.
