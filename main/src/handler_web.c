#include "handler_web.h"

static const char* TAG = "web_handler";

static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
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

static esp_err_t config_wifi_get_handler(httpd_req_t *req)
{
    char ctm[128] = {'\0'};
    char lel[4] = {'\0'};

    char chunk[1024];
    size_t read_bytes;

    wifi_context_t *context = (wifi_context_t *) req->user_ctx;

    int fd = open("/www/head.html", O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(TAG, "Failed to open file : /www/head.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");

    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1) ESP_LOGE(TAG, "Failed to read file : /www/head.html");
        else if (read_bytes > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);

    strcpy(ctm, "<form action=/config method=post>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<label for=opt>Select the Wifi network:</label><br><div class=gr-in>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<div class=in-gr><input class=opt type=radio name=ssid value=0><label align=left for=ssid_0>Default</label></div>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    for(uint8_t i = 0; i < *(context->ap_count); i++)
    {
        memset(ctm, '\0', 128);
        strcpy(ctm, "<div class=in-gr><input class=opt type=radio name=ssid value=");
        sprintf(lel, "%d", i+1);
        strcat(ctm, lel);
        strcat(ctm, ">");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));

        memset(ctm, '\0', 128);
        strcpy(ctm, "<label align=left for=ssid_");
        sprintf(lel, "%d", i+1);
        strcat(ctm, lel);
        strcat(ctm, ">");
        strcat(ctm, (char*)((context->ap_info)[i].ssid));
        strcat(ctm, "</label></div>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
    }

    memset(ctm, '\0', 128);
    strcpy(ctm, "</div>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));

    memset(ctm, '\0', 128);
    strcpy(ctm, "<label for=pass>Enter Wifi password if select:</label><br><br>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<input class=opt maxlength=50 type=text id=pass name=pass><button class=opt type=submit>Set wifi</button></form>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));

    fd = open("/www/end.html", O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(TAG, "Failed to open file : /www/end.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1) ESP_LOGE(TAG, "Failed to read file : /www/end.html");
        else if (read_bytes > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);

    return ESP_OK;
}

static esp_err_t config_wifi_post_handler(httpd_req_t *req)
{
    char buf[100] = {'\0'};
    size_t buf_len = req->content_len;

    wifi_context_t *context = (wifi_context_t *) req->user_ctx;

    if (buf_len > 0)
    {
        memset(buf, '\0', buf_len+1);
        httpd_req_recv(req, buf, buf_len);
        ESP_LOGI(TAG, "POST query => %s", buf);
        char param[50] = {'\0'};
        if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) == ESP_OK) *(context->opt) = atoi(param);
        if (httpd_query_key_value(buf, "pass", param, sizeof(param)) == ESP_OK) strcpy(context->wifi_password, param);
        *(context->http_ind) = 1;
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t config_reg_get_handler(httpd_req_t *req)
{
    char ctm[128] = {'\0'};

    char chunk[1024];
    size_t read_bytes;

    int fd = open("/www/head.html", O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(TAG, "Failed to open file : /www/head.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");

    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1)
        {
            ESP_LOGE(TAG, "Failed to read file : /www/head.html");
        }
        else if (read_bytes > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);

    strcpy(ctm, "<form action=/config method=post>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    struct stat st;
    if (stat(FILE_CONFIG, &st) == 0)
    {
        strcpy(ctm, "<label for=opt>There is a previous device configuration. Do you want to load it?</label><br>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
        memset(ctm, '\0', 128);
        strcpy(ctm, "<div class=gr-in><div class=in-gr><input class=opt type=radio name=opt value=y><label for=opt>Yes</label></div>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
        memset(ctm, '\0', 128);
        strcpy(ctm, "<div class=in-gr><input class=opt type=radio name=opt value=n><label for=opt>No</label></div></div>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
        memset(ctm, '\0', 128);
    }
    else
    {
        strcpy(ctm, "<label for=opt>There is no previous device configuration.</label><br>");
        httpd_resp_send_chunk(req, ctm, strlen(ctm));
        memset(ctm, '\0', 128);
    }
    strcpy(ctm, "<label for=code>Enter registration code:</label><br><br>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));
    memset(ctm, '\0', 128);
    strcpy(ctm, "<input class=opt type=number maxlength=6 id=code name=code><button class=opt type=submit>Set configuration</button></form>");
    httpd_resp_send_chunk(req, ctm, strlen(ctm));

    fd = open("/www/end.html", O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(TAG, "Failed to open file : /www/end.html");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    do {
        read_bytes = read(fd, chunk, 1024);
        if (read_bytes == -1)
        {
            ESP_LOGE(TAG, "Failed to read file : /www/end.html");
        } else if (read_bytes > 0)
        {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    close(fd);

    return ESP_OK;
}

static esp_err_t config_reg_post_handler(httpd_req_t *req)
{
    char buf[100] = {'\0'};
    size_t buf_len = req->content_len;

    reg_context_t *context = (reg_context_t *) req->user_ctx;

    if(buf_len > 0)
    {
        memset(buf, '\0', buf_len+1);
        httpd_req_recv(req, buf, buf_len);
        ESP_LOGI(TAG, "POST query => %s", buf);
        char param[50] = {'\0'};
        if(httpd_query_key_value(buf, "opt", param, sizeof(param)) == ESP_OK) *(context->opt_web) = param[0];
        if(httpd_query_key_value(buf, "code", param, sizeof(param)) == ESP_OK) strcpy(context->registration_code, param);
        *(context->http_ind) = 1;
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t common_get_handler(httpd_req_t *req)
{
    char filepath[128];

    strlcpy(filepath, "/www", sizeof("/www"));
    strlcat(filepath, req->uri, sizeof(filepath));

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
        if (read_bytes == -1) ESP_LOGE(TAG, "Failed to read file : %s", filepath);
        else if (read_bytes > 0) {
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

httpd_handle_t start_webserver(uint8_t server_opt, void *context)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_uri_t config_post =
    {
        .uri       = "/config",
        .method    = HTTP_POST,
        .handler   = NULL,
        .user_ctx  = context
    };
    httpd_uri_t config_get =
    {
        .uri       = "/config",
        .method    = HTTP_GET,
        .handler   = NULL,
        .user_ctx  = context
    };

    if(server_opt == WIFI_WEBSERVER)
    {
        config_post.handler = config_wifi_post_handler;
        config_get.handler  = config_wifi_get_handler;
    }
    else if(server_opt == REG_WEBSERVER)
    {
        config_post.handler = config_reg_post_handler;
        config_get.handler  = config_reg_get_handler;
    }

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