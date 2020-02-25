#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "esp_http_client.h"

void data_load(void);
void data_request(void);

#ifdef __cplusplus
}
#endif
