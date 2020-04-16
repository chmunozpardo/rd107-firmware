#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "definitions.h"

esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath);
httpd_handle_t start_webserver(void *func_get, void *func_post);
void stop_webserver(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
