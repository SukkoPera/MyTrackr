/*

  i2c OLED driver
  * only supports i2c
  * currently assumes 128*64 monochrome

*/

#ifndef OledDriver_h
#define OledDriver_h

#include <Arduino.h>
#include "font.h"

//** OLED I2C address, you may need to change this
#define OLED_ADDRESS	        		0x3C

//** dimensional settings
#define SSD1306_PIXEL_WIDTH             128
#define SSD1306_PIXEL_HEIGHT            64
#define SSD1306_PAGE_COUNT              8
#define SSD1306_PAGE_HEIGHT             (SSD1306_PIXEL_HEIGHT / SSD1306_PAGE_COUNT)
#define SSD1306_SEGMENT_COUNT           (SSD1306_PIXEL_WIDTH * SSD1306_PAGE_HEIGHT)

class OledDriver {
  public:
    //~ OledDriver(int mosi_pin, int clk_pin, int dc_pin, int cs_pin);

    void init();
    void clear();
    int segmentCount();
    void setSegmentCursor(int segment, int page);
    void setTextCursor(int column, int page);
    void writeCommand(byte cmd);
    void writeCommand(byte cmd, byte data);
    void writeData(byte data);
    void writeBuffer(byte *buffer, int len);
};

#endif
