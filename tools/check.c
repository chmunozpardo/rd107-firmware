const uint8_t check_Sign_Table [] =
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,
    0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,
    0x00,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,
    0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,
    0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0x00,0x00,0x00,
    0x00,0x00,0x01,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x03,0xFF,0xFF,0xFF,0x80,0x00,0x00,
    0x00,0x00,0x03,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xC0,0x00,0x00,
    0x00,0x00,0x07,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xE0,0x00,0x00,
    0x00,0x00,0x0F,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xF0,0x00,0x00,
    0x00,0x00,0x1F,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xF8,0x00,0x00,
    0x00,0x00,0x3F,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFC,0x00,0x00,
    0x00,0x00,0x7F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFE,0x00,0x00,
    0x00,0x00,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0x00,0x00,
    0x00,0x01,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0x80,0x00,
    0x00,0x03,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xC0,0x00,
    0x00,0x07,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xE0,0x00,
    0x00,0x07,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xE0,0x00,
    0x00,0x0F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xF0,0x00,
    0x00,0x1F,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xF8,0x00,
    0x00,0x3F,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFC,0x00,
    0x00,0x3F,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFC,0x00,
    0x00,0x7F,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFE,0x00,
    0x00,0x7F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,
    0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x00,
    0x01,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x80,
    0x01,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0x80,
    0x03,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xC0,
    0x03,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x1F,0xFF,0xC0,
    0x07,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x0F,0xFF,0xE0,
    0x07,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xF0,0x00,0x07,0xFF,0xE0,
    0x07,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xF8,0x00,0x07,0xFF,0xE0,
    0x0F,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFC,0x00,0x03,0xFF,0xF0,
    0x0F,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,0x00,0x03,0xFF,0xF0,
    0x1F,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0x00,0x01,0xFF,0xF8,
    0x1F,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0x80,0x01,0xFF,0xF8,
    0x1F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xC0,0x01,0xFF,0xF8,
    0x3F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xE0,0x00,0xFF,0xFC,
    0x3F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFF,0xF0,0x00,0xFF,0xFC,
    0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xF8,0x00,0x7F,0xFC,
    0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xF8,0x00,0x7F,0xFC,
    0x7F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xF0,0x00,0x7F,0xFE,
    0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xE0,0x00,0x3F,0xFE,
    0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xC0,0x00,0x3F,0xFE,
    0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0x80,0x00,0x3F,0xFE,
    0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,0x3F,0xFE,
    0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFF,0xFE,0x00,0x00,0x3F,0xFF,
    0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xFC,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xF8,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0x06,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xF0,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0x0F,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xE0,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0x1F,0x80,0x00,0x00,0x3F,0xFF,0xFF,0xC0,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0x3F,0xC0,0x00,0x00,0x7F,0xFF,0xFF,0x80,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0x7F,0xE0,0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x00,0xFF,0xF0,0x00,0x01,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x01,0xFF,0xF8,0x00,0x03,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x03,0xFF,0xFC,0x00,0x07,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x07,0xFF,0xFE,0x00,0x0F,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x0F,0xFF,0xFF,0x00,0x1F,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x1F,0xFF,0xFF,0x80,0x3F,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x3F,0xFF,0xFF,0xC0,0x7F,0xFF,0xFF,0x80,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x1F,0xFF,0xFF,0xE0,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x0F,0xFF,0xFF,0xF1,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x07,0xFF,0xFF,0xFB,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xF8,0x00,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,0x1F,0xFF,
    0xFF,0xFC,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x3F,0xFF,
    0x7F,0xFC,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,0x3F,0xFE,
    0x7F,0xFC,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x3F,0xFE,
    0x7F,0xFC,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0x80,0x00,0x00,0x00,0x00,0x3F,0xFE,
    0x7F,0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x3F,0xFE,
    0x7F,0xFE,0x00,0x00,0x0F,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
    0x3F,0xFE,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x7F,0xFC,
    0x3F,0xFE,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x7F,0xFC,
    0x3F,0xFF,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0xFF,0xFC,
    0x3F,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0xFF,0xFC,
    0x1F,0xFF,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x01,0xFF,0xF8,
    0x1F,0xFF,0x80,0x00,0x00,0x3F,0xFF,0xFF,0x80,0x00,0x00,0x00,0x00,0x01,0xFF,0xF8,
    0x1F,0xFF,0x80,0x00,0x00,0x1F,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xF8,
    0x0F,0xFF,0xC0,0x00,0x00,0x0F,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xF0,
    0x0F,0xFF,0xC0,0x00,0x00,0x07,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xF0,
    0x07,0xFF,0xE0,0x00,0x00,0x03,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xE0,
    0x07,0xFF,0xE0,0x00,0x00,0x01,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xE0,
    0x07,0xFF,0xF0,0x00,0x00,0x00,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xE0,
    0x03,0xFF,0xF8,0x00,0x00,0x00,0x7F,0xC0,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xC0,
    0x03,0xFF,0xF8,0x00,0x00,0x00,0x3F,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xC0,
    0x01,0xFF,0xFC,0x00,0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0x80,
    0x01,0xFF,0xFE,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x80,
    0x00,0xFF,0xFE,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x00,
    0x00,0x7F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,
    0x00,0x7F,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFE,0x00,
    0x00,0x3F,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFC,0x00,
    0x00,0x3F,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFC,0x00,
    0x00,0x1F,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xF8,0x00,
    0x00,0x0F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xF0,0x00,
    0x00,0x07,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xE0,0x00,
    0x00,0x07,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xE0,0x00,
    0x00,0x03,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xC0,0x00,
    0x00,0x01,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0x80,0x00,
    0x00,0x00,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0x00,0x00,
    0x00,0x00,0x7F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFE,0x00,0x00,
    0x00,0x00,0x3F,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFC,0x00,0x00,
    0x00,0x00,0x1F,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xF8,0x00,0x00,
    0x00,0x00,0x0F,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xF0,0x00,0x00,
    0x00,0x00,0x07,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xE0,0x00,0x00,
    0x00,0x00,0x03,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xC0,0x00,0x00,
    0x00,0x00,0x01,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x03,0xFF,0xFF,0xFF,0x80,0x00,0x00,
    0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0x00,0x00,0x00,
    0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,
    0x00,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,
    0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,
    0x00,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
};

sFONT check_Sign =
{
    check_Sign_Table,
    128, /* Width */
    128, /* Height */
};0xf800
