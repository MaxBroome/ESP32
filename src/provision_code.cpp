#include "provision_code.h"
#include "nvs_manager.h"
#include <esp_random.h>

static const char* NVS_NS  = "device";
static const char* NVS_KEY = "claim_code";

// Exclude ambiguous: 0/O, 1/I/L
static const char CHARS[] = "ABCDEFGHJKMNPQRSTUVWXYZ23456789";
static constexpr int CODE_LEN = 6;

String ProvisionCode::getOrCreate() {
    NvsManager::instance().registerNamespace(NVS_NS);

    String code = NvsManager::instance().getString(NVS_NS, NVS_KEY);
    if (code.length() == CODE_LEN) {
        return code;
    }

    code.reserve(CODE_LEN + 1);
    code = "";
    uint32_t r;
    for (int i = 0; i < CODE_LEN; i++) {
        r = esp_random() % (sizeof(CHARS) - 1);
        code += CHARS[r];
    }

    NvsManager::instance().putString(NVS_NS, NVS_KEY, code);
    Serial.printf("[init] claim code: %s\n", code.c_str());
    return code;
}
