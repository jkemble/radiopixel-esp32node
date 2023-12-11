#include "patterns/Strip.h"


namespace Pattern
{

void Strip::setAllColor( Color::rgb32_t color)
{
    for ( uint16_t i = 0; i < numPixels( ); i++ ) 
    {
        setPixelColor( i, color );
    }
}

/*
void Strip::setAllFade( uint8_t v )
{
    for ( uint16_t i = 0; i < numPixels( ); i++ ) 
    {
        setPixelColor( i, ColorFade( getPixelColor( i ), v ) );
    }
}
*/

}
