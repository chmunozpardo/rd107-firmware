#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

httpd_handle_t start_webserver(uint8_t server_opt, void *context);
void stop_webserver(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
