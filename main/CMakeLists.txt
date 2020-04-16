idf_component_register(SRCS
                    "app_main.c" 
                    "src/qrcodegen.c"
                    "src/handler_rgb.c"
                    "src/handler_spi.c"
                    "src/handler_web.c"
                    "src/handler_data.c"
                    "src/handler_sntp.c"
                    "src/handler_wifi.c"
                    "src/handler_files.c"
                    "src/handler_parse.c"
                    "src/handler_relay.c"
                    "src/handler_touch.c"
                    "src/handler_buzzer.c"
                    "src/handler_screen.c"
                    "src/handler_search.c"
                    "src/handler_wiegand.c"
                    "src/Fonts/font8.c"
                    "src/Fonts/font12.c"
                    "src/Fonts/font16.c"
                    "src/Fonts/font20.c"
                    "src/Fonts/font24.c"
                    "src/Waveshare_ILI9486.c"
                    INCLUDE_DIRS "inc")

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front")
spiffs_create_partition_image(www ${WEB_SRC_DIR} FLASH_IN_PROJECT)