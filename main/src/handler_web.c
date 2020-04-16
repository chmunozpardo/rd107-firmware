#include "handler_web.h"

static const char* TAG = "web_handler";

static esp_err_t (*function_get)(httpd_req_t*);
static esp_err_t (*function_post)(httpd_req_t*);

static esp_err_t common_get_handler(httpd_req_t *req)
{
    char filepath[128];

    strlcpy(filepath, "/www", sizeof("/www"));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(TAG, "Failed to open file : %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char chunk[1024];
    ssize_t read_bytes;
    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1) {
            ESP_LOGE(TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);
    ESP_LOGI(TAG, "File sending complete");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if     (CHECK_FILE_EXTENSION(filepath, ".html")) type = "text/html";
    else if(CHECK_FILE_EXTENSION(filepath, ".js"  )) type = "application/javascript";
    else if(CHECK_FILE_EXTENSION(filepath, ".css" )) type = "text/css";
    else if(CHECK_FILE_EXTENSION(filepath, ".png" )) type = "image/png";
    else if(CHECK_FILE_EXTENSION(filepath, ".jpg" )) type = "image/jpg";
    else if(CHECK_FILE_EXTENSION(filepath, ".ico" )) type = "image/x-icon";
    else if(CHECK_FILE_EXTENSION(filepath, ".svg" )) type = "text/xml";
    return httpd_resp_set_type(req, type);
}

httpd_handle_t start_webserver(void *func_get, void *func_post)
{
    function_get  = func_get;
    function_post = func_post;

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_uri_t config_post =
    {
        .uri       = "/config",
        .method    = HTTP_POST,
        .handler   = function_post,
        .user_ctx  = NULL
    };

    httpd_uri_t config_get =
    {
        .uri       = "/config",
        .method    = HTTP_GET,
        .handler   = function_get,
        .user_ctx  = NULL
    };

    httpd_uri_t common_get_uri =
    {
        .uri       = "/*",
        .method    = HTTP_GET,
        .handler   = common_get_handler,
        .user_ctx  = NULL
    };

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if(httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &config_post);
        httpd_register_uri_handler(server, &config_get);
        httpd_register_uri_handler(server, &common_get_uri);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}