#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_spiffs.h"

void fs_init(void);

#ifdef __cplusplus
}
#endif
