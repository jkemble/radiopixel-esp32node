#pragma once

#include <cstdint>
#include <array>
#include "../export.h"


namespace Color
{

#pragma pack( push, 1 )
// RGB packed into 3 consecutive bytes, in order: R, G, B
// Note the location of values is INDEPENDENT of endianness
struct rgb24_t
{
    rgb24_t( uint8_t _r = 0, uint8_t _g = 0, uint8_t _b = 0 )
        : r( _r ), g( _g ), b( _b )
    {
    }

    uint8_t r;
    uint8_t g;
    uint8_t b;

    // predefined colors
    static rgb24_t Black( )   { return rgb24_t(    0,    0,    0 ); }
    static rgb24_t Red( )     { return rgb24_t( 0xff,    0,    0 ); }
    static rgb24_t Green( )   { return rgb24_t(    0, 0xff,    0 ); }
    static rgb24_t Blue( )    { return rgb24_t(    0,    0, 0xff ); }
    static rgb24_t Cyan( )    { return rgb24_t(    0, 0xff, 0xff ); }
    static rgb24_t Magenta( ) { return rgb24_t( 0xff,    0, 0xff ); }
    static rgb24_t Yellow( )  { return rgb24_t( 0xff, 0xff,    0 ); }
    static rgb24_t White( )   { return rgb24_t( 0xff, 0xff, 0xff ); }
};
#pragma pack( pop )

#pragma pack( push, 1 )
// RGB packed into 32 bits, from MSB to LSB: unused, R, G, B
// Note the location of values is DEPENDENT on endianness
struct rgb32_t
{
    rgb32_t( uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t unused = 0 )
    {
        setAll( r, g, b, unused );
    }

    uint8_t unused( ) const
    {
        return ( packed >> 24 ) & 0xff;
    }

    void setUnused( uint8_t unused )
    {
        packed = ( packed & 0x00ffffff ) + ( unused << 24 );
    }

    uint8_t r( ) const
    {
        return ( packed >> 16 ) & 0xff;
    }

    void setR( uint8_t r )
    {
        packed = ( packed & 0xff00ffff ) + ( r << 16 );
    }

    uint8_t g( ) const
    {
        return ( packed >> 8 ) & 0xff;
    }

    void setG( uint8_t g )
    {
        packed = ( packed & 0xffff00ff ) + ( g << 8 );
    }

    uint8_t b( ) const
    {
        return packed & 0xff;
    }

    void setB( uint8_t b )
    {
        packed = ( packed & 0xffffff00 ) + b;
    }

    void setAll( uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t unused = 0 )
    {
        packed = ( unused << 24 ) + ( r << 16 ) + ( g << 8 ) + b;
    }

    uint32_t packed;

    operator uint32_t( ) const
    {
        return packed;
    }

    // colors
    static rgb32_t Black( )   { return rgb32_t(    0,    0,    0 ); }
    static rgb32_t Red( )     { return rgb32_t( 0xff,    0,    0 ); }
    static rgb32_t Green( )   { return rgb32_t(    0, 0xff,    0 ); }
    static rgb32_t Blue( )    { return rgb32_t(    0,    0, 0xff ); }
    static rgb32_t Cyan( )    { return rgb32_t(    0, 0xff, 0xff ); }
    static rgb32_t Magenta( ) { return rgb32_t( 0xff,    0, 0xff ); }
    static rgb32_t Yellow( )  { return rgb32_t( 0xff, 0xff,    0 ); }
    static rgb32_t White( )   { return rgb32_t( 0xff, 0xff, 0xff ); }
};
#pragma pack( pop )

#pragma pack( push, 1 )
// WRGB packed into 32 bits
struct wrgb32_t
{
    wrgb32_t( uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t w = 0 )
    {
        setAll( r, g, b, w );
    }

    uint8_t w( ) const
    {
        return ( packed >> 24 ) & 0xff;
    }

    void setW( uint8_t w )
    {
        packed = ( packed & 0x00ffffff ) + ( w << 24 );
    }

    uint8_t r( ) const
    {
        return ( packed >> 16 ) & 0xff;
    }

    void setR( uint8_t r )
    {
        packed = ( packed & 0xff00ffff ) + ( r << 16 );
    }

    uint8_t g( ) const
    {
        return ( packed >> 8 ) & 0xff;
    }

    void setG( uint8_t g )
    {
        packed = ( packed & 0xffff00ff ) + ( g << 8 );
    }

    uint8_t b( ) const
    {
        return packed & 0xff;
    }

    void setB( uint8_t b )
    {
        packed = ( packed & 0xffffff00 ) + b;
    }

    void setAll( uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t w = 0 )
    {
        packed = ( w << 24 ) + ( r << 16 ) + ( g << 8 ) + b;
    }

    uint32_t packed;

    operator uint32_t( ) const
    {
        return packed;
    }
};
#pragma pack( pop )


// Calculates RGB color gradient values
class LIGHTTOOLS_API Gradient
{
public:
    Gradient();

    struct Step
    {
        uint8_t pos;
        rgb32_t color;
    };

    void clearSteps( );
    void addStep( uint8_t pos, rgb32_t color );
    void setSteps( Step *st, uint8_t steps );

    rgb32_t getColor( uint8_t pos );

/*
    void smear();
    void randomize();
    void randomize(int, int); // set low and high range
    void peturb(int, int);
    void fade();
    void wipe(byte level);
*/

private:
    std::array< Step, 10> m_steps;
    uint8_t m_stepCount;
};

// Color tools

// generic 8 bit fader
inline uint8_t fade( uint8_t low, uint8_t high, uint8_t v )
{
    // promote to 32 bits to ensure enough range to handle multiplication
    return ( static_cast< uint32_t >( high ) - low ) * v / 255 + low;
}

// decrease intensity by value (0-255)
rgb32_t LIGHTTOOLS_API ColorFade( rgb32_t color, uint8_t value );

// crossfade between colors
rgb32_t LIGHTTOOLS_API ColorBlend( rgb32_t color1, rgb32_t color2, uint8_t value );

// random color
rgb32_t LIGHTTOOLS_API ColorRandom( );

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
rgb32_t LIGHTTOOLS_API ColorWheel( uint8_t WheelPos );

} // namespace Color
