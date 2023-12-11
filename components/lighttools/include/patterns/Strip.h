#pragma once

#include "../export.h"
#include "../color/RGB.h"


namespace Pattern
{

// Pixel strip abstract base class
class LIGHTTOOLS_API Strip
{
public:
    virtual ~Strip( ) { };

    // returns the number of pixels
    virtual uint16_t numPixels( ) const = 0;

    // get a pixel color value
    virtual Color::rgb32_t getPixelColor( uint16_t pixel ) const = 0;

    // set a single pixel to a color
    virtual void setPixelColor( uint16_t pixel, Color::rgb32_t color ) = 0;

    // set all pixels to a color
    virtual void setAllColor( Color::rgb32_t color );

    // independent control of brightness
    virtual uint8_t getBrightness( ) const = 0;

    // independent control of brightness
    virtual void setBrightness( uint8_t bright ) = 0;

    // send the buffered pixel data to the hardware
    virtual void transmit() const = 0;
};

}
