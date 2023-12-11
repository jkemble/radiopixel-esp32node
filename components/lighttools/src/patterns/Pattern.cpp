#include <math.h>
#include <cstdlib> // rand
#include "patterns/Pattern.h"


namespace Pattern
{

Pattern::Pattern( )
{
    m_color[ 0 ] = Color::rgb32_t::Red( );
    m_color[ 1 ] = Color::rgb32_t::White( );
    m_color[ 2 ] = Color::rgb32_t::Green( );

    m_level[ 0 ] = m_level[ 1 ] = m_level[ 2 ] = 0x80;
}

//-------------------------------------------------------------

Pattern *CreatePattern( uint8_t pattern )
{
    switch ( pattern )
    {
    case MiniTwinkle:
        return new MiniTwinklePattern( );
    case MiniSparkle:
        return new MiniSparklePattern( );
    case Sparkle:
        return new SparklePattern( );
    case Rainbow:
        return new RainbowPattern( );
    case Flash:
        return new FlashPattern( );
    case March:
        return new MarchPattern( );
    case Wipe:
        return new WipePattern( );
    case Gradient:
        return new GradientPattern( );
    case Fixed:
        return new FixedPattern( );
    case Strobe:
        return new StrobePattern( );
    case CandyCane:
        return new CandyCanePattern( );
    case Test:
    default:
        return new TestPattern( );
    }
}

//-------------------------------------------------------------

ms_t FlashPattern::GetDuration( Strip * )
{
    return 4000;
}

void FlashPattern::Update( Strip *strip, ms_t offset )
{
    uint16_t t = offset * 300 / GetDuration( strip );
    uint16_t o = t % 100;
    Color::rgb32_t col = color( t / 100 );
    if ( ( o <= 10 ) || ( o >= 20 && o <= 30 ) )
    {
        strip->setAllColor( col );
    }
    else if ( o > 30 && o <= 60 )
    {
        uint8_t f = ( 60 - o ) * 255 / 30;
        strip->setAllColor( Color::ColorFade( col, f ) );
    }
    else
    {
        strip->setAllColor( 0 );
    }
}

//-------------------------------------------------------------

ms_t RainbowPattern::GetDuration( Strip * )
{
    return 2000;
}

void RainbowPattern::Update( Strip *strip, ms_t offset )
{
    for ( int i = 0; i < strip->numPixels( ); i++ )
    {
        uint8_t t = 255 - offset * 255 / GetDuration( strip );
        uint8_t p = i * 255 / strip->numPixels( );
        strip->setPixelColor( i, Color::ColorWheel( ( p + t ) % 255 ) );
    }
}

//-------------------------------------------------------------

ms_t SparklePattern::GetDuration( Strip * )
{
    return 100;
}

void SparklePattern::Loop( Strip *strip, ms_t offset )
{
    // strobe - new pixels each loop
    strip->setAllColor( 0 );
    for ( int c = Color::fade( 1, strip->numPixels( ), m_level[ 0 ] ); c; c-- )
    {
        Color::rgb32_t col = color( std::rand( ) % 3 );
        if ( col == 0 )
            col = Color::ColorRandom( );
        strip->setPixelColor( std::rand( ) % strip->numPixels( ), col );
    }

    Update( strip, offset );
}

//-------------------------------------------------------------

void MiniSparklePattern::Update( Strip *strip, ms_t offset )
{
    // 25% duty cycle
    if ( offset > GetDuration( strip ) / 4 )
    {
        strip->setAllColor( 0 );
    }
}

//-------------------------------------------------------------

MiniTwinklePattern::MiniTwinklePattern()
{
}

void MiniTwinklePattern::Init( Strip *strip, ms_t offset )
{
    m_lastDim = m_lastLit = offset;
    Loop( strip, offset );
}

ms_t MiniTwinklePattern::GetDuration( Strip * )
{
    return 1000;
}

void MiniTwinklePattern::Update( Strip *strip, ms_t offset )
{
    int duration( GetDuration( strip ) );

    // dim down all pixels
    ms_t dimDelta( delta( m_lastDim, offset, duration ) );
    int dim( 255 - ( dimDelta * 255 / duration ) );
    if ( dim < 255 )
    {
        for ( int i = 0; i < strip->numPixels( ); ++i )
        {
            Color::rgb32_t col = strip->getPixelColor( i );
            col = Color::ColorFade( col, dim );
            strip->setPixelColor( i, col );
        }
        m_lastDim = offset;
    }

    // add any new pixels as needed
    long total( Color::fade( 1, strip->numPixels( ), m_level[ 0 ] ) );
    ms_t litDelta( delta( m_lastLit, offset, duration ) );
    long todo( litDelta * total / duration );
    if ( todo > 0 )
    {
        for ( ; todo > 0; todo-- )
        {
            int i = std::rand( ) % strip->numPixels( );
            Color::rgb32_t col( color( std::rand( ) % 3 ) );
            strip->setPixelColor( i, col );
        }
        m_lastLit = offset; // only update if we lit something!
    }
}

ms_t MiniTwinklePattern::delta( ms_t previous, ms_t next, ms_t duration )
{
    if ( next >= previous )
    {
        return next - previous;
    }
    else
    {
        return duration - previous + next;
    }
}

//-------------------------------------------------------------

ms_t MarchPattern::GetDuration( Strip * )
{
    return 1000;
}

void MarchPattern::Update( Strip *strip, ms_t offset )
{
    // a segment is one color, there are three segments in a loop

    // duration of a loop
    uint32_t duration = GetDuration( strip );
    // length of each segment?
    uint8_t length = std::max<uint8_t>(m_level[0], 2);
    // how far are we through all three segments
    uint32_t o = ( length * 3 ) - ( offset * length * 3 / duration );

    for ( int i = 0; i < strip->numPixels( ); i++ )
    {
        // fade level based on position within segment
        uint32_t e = ( i + o ) % length;
        if ( e > ( length / 2 ) )
        {
            e = length - e;
        }
        if ( e > ( length / 4 ) )
        {
            e = e / 2;
        }
        else
        {
            e = 0;
        }
        uint8_t f = e * 255 / ( length / 2 );

        // color based on segment
        auto c = color( ( ( i + o ) / length ) % 3 );

        strip->setPixelColor( i, Color::ColorFade( c, f ) );
    }
}

//-------------------------------------------------------------

ms_t WipePattern::GetDuration( Strip * )
{
    return 3000;
}

void WipePattern::Update( Strip *strip, ms_t offset )
{
    for ( int i = 0; i < strip->numPixels( ); i++ )
    {
        int d = GetDuration( strip );
        int t = offset * ( strip->numPixels( ) * 3 ) / d;
        t = ( strip->numPixels( ) * 3 ) - t; // offset due to time
        int c = ( i + t ) / strip->numPixels( );
        int e = ( i + t ) % strip->numPixels( );
        uint8_t f = e * 255 / strip->numPixels( );
        f = ( f < 128 ) ? 0 : ( ( f - 128 ) * 2 );
        strip->setPixelColor( i, Color::ColorFade( color( c ), f ) );
    }
}

//-------------------------------------------------------------
/*
Color::Gradient::Step steps[ 4 ] = {
    { 0, Color::rgb32_t( 0x20, 0, 0 ) },
    { 32, Color::rgb32_t( 0xc0, 0x19, 0 ) },
    { 192, Color::rgb32_t( 0xff, 0x60, 0x10 ) },
    { 255, Color::rgb32_t( 0x40, 0x40, 0xff ) }
};
*/

GradientPattern::GradientPattern( )
{
    mp1 = mp2 = NULL;
}

ms_t GradientPattern::GetDuration( Strip * )
{
    return 1000;
}

void GradientPattern::Init( Strip *strip, ms_t offset )
{
    // setup gradient
    ResetGradient();
    changed = false;

    // setup maps
    mp1 = new uint8_t[ strip->numPixels( ) ];
    mp2 = new uint8_t[ strip->numPixels( ) ];
    if ( mp1 && mp2 )
    {
        for ( int i = 0; i < strip->numPixels( ); i++ )
        {
            mp1[ i ] = mp2[ i ] = i;
        }
    }
    Loop( strip, offset );
}

void GradientPattern::Loop( Strip *strip, ms_t offset )
{
    // create a new random map
    if ( mp1 && mp2 )
    {
        for ( int i = 0; i < strip->numPixels( ); i++ )
        {
            mp1[ i ] = mp2[ i ];
            mp2[ i ] = std::rand( ) % strip->numPixels( );
        }
    }
    Update( strip, offset );
}

void GradientPattern::Update( Strip *strip, ms_t offset )
{
    if ( changed )
    {
        ResetGradient();
        changed = false;
    }

    for ( int i = 0; i < strip->numPixels( ); i++ )
    {
        auto c1 = Color::rgb32_t::Red( ), c2 = Color::rgb32_t::Red( );
        if ( mp1 && mp2 )
        {
            c1 = grad.getColor( ( int )mp1[ i ] * 255 / strip->numPixels( ) );
            c2 = grad.getColor( ( int )mp2[ i ] * 255 / strip->numPixels( ) );
        }
        strip->setPixelColor( i, Color::ColorBlend( c1, c2, offset * 255 / GetDuration( strip ) ) );
    }
}

void GradientPattern::setColor( int index, Color::rgb32_t _color )
{
    if ( color( index ) != _color )
    {
        Pattern::setColor( index, _color );
        changed = true;
    }
}

void GradientPattern::setLevel( int index, uint8_t _level )
{
    if ( level( index ) != _level )
    {
        Pattern::setLevel( index, _level );
        changed = true;
    }
}

void GradientPattern::ResetGradient()
{
    grad.clearSteps( );
    if ( m_level[ 0 ] > 6 && m_level[ 0 ] < 249 )
    {
        grad.addStep( 0, m_color[ 0 ] );
        grad.addStep( m_level[ 0 ] / 3, m_color[ 1 ] );
        grad.addStep( ( int )m_level[ 0 ] * 2 / 3, m_color[ 2 ] );
        grad.addStep( m_level[ 0 ], m_color[ 0 ] );
        grad.addStep( m_level[ 0 ] + 1, 0 );
        grad.addStep( 255, 0 );
    }
    else
    {
        grad.addStep( 0, m_color[ 0 ] );
        grad.addStep( 85, m_color[ 1 ] );
        grad.addStep( 170, m_color[ 2 ] );
        grad.addStep( 255, m_color[ 0 ] );
    }
}

GradientPattern::~GradientPattern( )
{
    delete [] mp1;
    mp1 = NULL;
    delete [] mp2;
    mp2 = NULL;
}

//-------------------------------------------------------------

ms_t StrobePattern::GetDuration( Strip * )
{
    return 750; // 4Hz at 100% spped, 10Hz at 250% speed
}

void StrobePattern::Update( Strip *strip, ms_t offset )
{
    ms_t third( GetDuration( strip ) / 3 );
    if ( ( offset / third ) != ( m_lastOffset / third ) )
    {
        strip->setAllColor( color( offset / third ) );
    }
    else
    {
        strip->setAllColor( 0 );
    }
    m_lastOffset = offset;
}

//-------------------------------------------------------------

ms_t CandyCanePattern::GetDuration( Strip * )
{
    return 200;
}

void CandyCanePattern::Update( Strip *strip, ms_t offset )
{
    int c = 0;
    if ( offset < ( GetDuration( strip ) / 2 ) )
        c = 1;
    for ( int i = 0; i < strip->numPixels( ); i++ )
    {
        strip->setPixelColor( i, color( c + ( i % 2 ) ) );
    }
}

//-------------------------------------------------------------

ms_t TestPattern::GetDuration( Strip * )
{
    return 1000;
}

void TestPattern::Update( Strip *strip, ms_t )
{
    switch ( m_level[0] >> 4 )
    {
    case 0:
    {
        // show value in level[1] as a count of LEDs
        const int code = m_level[1];
        const int space = 3;
        for ( int i = 0; i < strip->numPixels( ); i++ )
        {
            bool on( ( i % ( code + space ) ) < code );
            strip->setPixelColor( i, on ? m_color[ 0 ] : Color::rgb32_t::Black( ) );
        }
        break;
    }

    case 1:
        // intensity scale
        for ( int i = 0; i < strip->numPixels( ); i++ )
        {
            uint8_t f = i * 255 / strip->numPixels( );
            strip->setPixelColor( i, Color::ColorFade( m_color[ 0 ], f ) );
        }
        break;

    case 2:
    {
        // test gradient
        Color::Gradient grad;
        grad.clearSteps( );
        grad.addStep( 0, m_color[ 0 ] );
        grad.addStep( 85, m_color[ 1 ] );
        grad.addStep( 170, m_color[ 2 ] );
        grad.addStep( 255, m_color[ 0 ] );
        for ( int i = 0; i < strip->numPixels( ); i++ )
        {
            strip->setPixelColor( i, grad.getColor( i * 255 / strip->numPixels( ) ) );
        }
        break;
    }

    default:
        strip->setAllColor(Color::rgb32_t::Black());
    }
}

//-------------------------------------------------------------

ms_t FixedPattern::GetDuration( Strip * )
{
    return 750;
}

void FixedPattern::Update( Strip *strip, ms_t offset )
{
    int step = 3 * offset / GetDuration( strip );
    auto col( color( step ) );
    for ( int i = 0; i < strip->numPixels( ); i++ )
    {
        strip->setPixelColor( i, ( i % 3 == step ) ? col : Color::rgb32_t::Black( ) );
    }
}

} // namespace Pattern
