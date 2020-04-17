#include "definitions.h"

const uint8_t dreamit_LOGO_Top_Table [] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x80, 0x00,
    0x00, 0x03, 0xC0, 0x00,
    0x00, 0x07, 0xE0, 0x00,
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x3F, 0xFC, 0x00,
    0x00, 0x1F, 0xF8, 0x00,
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x07, 0xE0, 0x00,
    0x00, 0x03, 0xC0, 0x00,
    0x00, 0x01, 0x80, 0x00,
};

sFONT dreamit_LOGO_Top = {
    dreamit_LOGO_Top_Table,
    32, /* Width */
    12, /* Height */
};

const uint8_t dreamit_LOGO_Bot_Table [] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x08, 0x00, 0x00, 0x10, //1
    0x0C, 0x00, 0x00, 0x30, //2
    0x06, 0x00, 0x00, 0x60, //2
    0x07, 0x00, 0x00, 0xE0, //3
    0x03, 0x80, 0x01, 0xC0, //3
    0x03, 0xC0, 0x03, 0xC0, //4
    0x01, 0xE0, 0x07, 0x80, //4
    0x01, 0xF0, 0x0F, 0x80, //5
    0x00, 0xF8, 0x1F, 0x00, //5
    0x00, 0xFC, 0x3F, 0x00, //6
    0x00, 0xFC, 0x3F, 0x00, //6
    0x00, 0x7C, 0x3E, 0x00, //5
    0x00, 0x7C, 0x3E, 0x00, //5
    0x00, 0x3C, 0x3C, 0x00, //4
    0x00, 0x3C, 0x3C, 0x00, //4
    0x00, 0x1C, 0x38, 0x00, //3
    0x00, 0x1C, 0x38, 0x00, //3
    0x00, 0x1C, 0x38, 0x00, //3
    0x00, 0x0C, 0x30, 0x00, //2
    0x00, 0x0C, 0x30, 0x00, //2
    0x00, 0x0C, 0x30, 0x00, //2
    0x00, 0x04, 0x20, 0x00, //1
    0x00, 0x04, 0x20, 0x00, //1
};

sFONT dreamit_LOGO_Bot = {
    dreamit_LOGO_Bot_Table,
    32, /* Width */
    29, /* Height */
};
