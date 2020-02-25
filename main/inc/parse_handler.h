#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "esp_log.h"
#include "card_handler.h"

#define REG_TMP_FILE    "/spiffs/registers.tmp"
#define REG_FILE        "/spiffs/registers.db"
#define REG_TIMESTAMP   "/spiffs/timestamp.db"

void parse_data(void);

#ifdef __cplusplus
}
#endif
