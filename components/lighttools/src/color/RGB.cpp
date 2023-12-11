#include <utility>
#include <cstdlib> // rand

#include "color/RGB.h"


namespace Color
{

Gradient::Gradient()
    : m_stepCount( 0 )
{
}

void Gradient::clearSteps( )
{
    m_stepCount = 0;
}

void Gradient::addStep( uint8_t pos, rgb32_t color )
{
    if ( m_stepCount < m_steps.size( ) )
    {
        m_steps[ m_stepCount ].pos = pos;
        m_steps[ m_stepCount ].color = color;
        m_stepCount++;
    }
}

void Gradient::setSteps( Step *steps, uint8_t stepCount )
{
    m_stepCount = std::min< uint8_t >( stepCount, m_steps.size( ) );
    for ( int i = 0; i < m_stepCount; ++i )
    {
        m_steps[ i ] = steps[ i ];
    }
}

rgb32_t Gradient::getColor( uint8_t pos )
{
    if ( pos <= m_steps[ 0 ].pos ) 
    {
        return m_steps[ 0 ].color;
    } 
    else if ( pos >= m_steps[ m_stepCount - 1 ].pos ) 
    {
        return m_steps[ m_stepCount - 1 ].color;
    }
    else 
    {
        int i = 0;
        while ( i < ( m_stepCount - 1 ) && 
            !( pos >= m_steps[ i ].pos && pos < m_steps[ i + 1 ].pos ) )
        {
            i++;
        }
        if ( i >= ( m_stepCount - 1 ) )
        {
            return 0;
        }
        if ( m_steps[ i + 1 ].pos == m_steps[ i ].pos )
        {
            return m_steps[ i ].color;
        }
        
        uint8_t f = ( pos - m_steps[ i ].pos ) * 255 / 
            ( m_steps[ i + 1 ].pos - m_steps[ i ].pos );
        return ColorBlend( m_steps[ i ].color, m_steps[ i + 1 ].color, f );
    }
}

/*

void Gradient::smear()
{
    uint8_t prevVal, savePixel;
    uint16_t _numPixels = strip->numPixels();
    if (!strip || !strip->numPixels( ) || !_pixels)
    {
        return;
    }
    _pixels[0] = (uint8_t)((int)(_pixels[0] + _pixels[1])/3);
    prevVal = _pixels[0];
    for (int i=1; i < _numPixels - 1; i++)
    {
        savePixel = _pixels[i];
        _pixels[i] = (uint8_t)((int)(_pixels[i] + _pixels[i+1] + prevVal)/3);
        prevVal = savePixel;
    }
    _pixels[_numPixels-1] = (uint8_t)((int)(_pixels[_numPixels] + _pixels[prevVal])/3);
    updateStrip();
}

void Gradient::randomize()
{
    for (int i=0; i < strip->numPixels( ); i++)
    {
        _pixels[i] = random(255);
    }
    updateStrip();
}

void Gradient::randomize(int low, int high)
{
    for (int i=0; i < strip->numPixels( ); i++)
    {
        _pixels[i] = random(low, high);
    }
    updateStrip();
}

void Gradient::peturb(int low, int high)
{
    int temp;
    for (int i=0; i < strip->numPixels( ); i++)
    {
        temp = _pixels[i] + (random(low, high));
        if (temp < 0) temp = 0;
        if (temp > 255) temp = 255;
        _pixels[i] = temp;
    }
    updateStrip();
}

void Gradient::fade()
{
    for (int i=0; i < strip->numPixels( ); i++)
    {
        if (_pixels[i] < 4)
            _pixels[i] = 0;
        else
            _pixels[i] -= 4;
    }
    updateStrip();
}

void Gradient::wipe(byte level)
{
    for (int i=0; i < strip->numPixels( ); i++)
    {
        _pixels[i] = level;
    }
    updateStrip();
}

*/


rgb32_t ColorFade( rgb32_t c, uint8_t v )
{
    return rgb32_t( fade( 0, c.r( ), v ), fade( 0, c.g( ), v ), fade( 0, c.b( ), v ) );    
}

rgb32_t ColorBlend( rgb32_t c1, rgb32_t c2, uint8_t v )
{
    return rgb32_t( fade( c1.r( ), c2.r( ), v ), fade( c1.g( ), c2.g( ), v ), fade( c1.b( ), c2.b( ), v ) );    
}

rgb32_t ColorRandom( )
{
    return ColorWheel( std::rand( ) % 256 );
}


int fade( int outlow, int outhigh, int in, int inlow, int inhigh )
{
    return ( outhigh - outlow ) * ( in - inlow ) / ( inhigh - inlow ) + outlow;
}

struct Hue
{
    enum
    {
        Red = 0,
        Yellow = 42,
        Green = 85,
        Cyan = 127,
        Blue = 170,
        Magenta = 212,
        RedWrap = 256,  // yes, 256
    };
};

// --> someday this will grow up to be an hsv32_t, and an rgb32_t ctor taking hsv32_t

rgb32_t ColorWheel( uint8_t hue )
{
    if ( hue <= Hue::Yellow )
    {
        return rgb32_t( 255, fade( 0, 255, hue, Hue::Red, Hue::Yellow ), 0 );
    }
    else if( hue <= Hue::Green )
    {
        return rgb32_t( fade( 255, 0, hue, Hue::Yellow, Hue::Green ), 255, 0 );
    }
    else if( hue <= Hue::Cyan )
    {
        return rgb32_t( 0, 255, fade( 0, 255, hue, Hue::Green, Hue::Cyan ) );
    }
    else if( hue <= Hue::Blue )
    {
        return rgb32_t( 0, fade( 255, 0, hue, Hue::Cyan, Hue::Blue ), 255 );
    }
    else if( hue <= Hue::Magenta )
    {
        return rgb32_t( fade( 0, 255, hue, Hue::Blue, Hue::Magenta ), 0, 255 );
    }
    else
    {
        return rgb32_t( 255, 0, fade( 255, 0, hue, Hue::Magenta, Hue::RedWrap ) );
    }
}

} // namespace Color
