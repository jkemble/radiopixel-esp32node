#pragma once

#include "Strip.h"
#include "../export.h"
#include "../color/RGB.h"


namespace Pattern
{

typedef uint32_t ms_t; // duration in milliseconds

// A Pattern animates the LEDs of a Strip.
//
// A Pattern has a fixed duration and is repeated.  Callers must first call Init()
// to setup the Pattern, then periodically call Update() with the current offset.
// After each loop completes, call Loop with offset reset to the beginning of the
// period.
//
// Each Pattern has three colors it can use in any way, as well as three levels
// that can control any aspect of the Pattern.
class LIGHTTOOLS_API Pattern
{
public:
    Pattern( );

    virtual ~Pattern() {}

    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip * )
    {
        return 40;
    }

    // assume nothing, setup all pixels
    virtual void Init( Strip *strip, ms_t offset )
    {
        strip->setAllColor( Color::rgb32_t::Black( ) );
        Loop( strip, offset );
    }

    // restarting after a loop expired, but not first call
    virtual void Loop( Strip *strip, ms_t offset )
    {
        Update( strip, offset );
    }

    // update pixels as needed
    virtual void Update( Strip *, ms_t ) { }

    // returns color
    Color::rgb32_t color( int index ) const
    {
        return m_color[ index % 3 ];
    }

    // sets color
    virtual void setColor( int index, Color::rgb32_t color )
    {
        m_color[ index % 3 ] = color;
    }

    // returns level
    uint8_t level( int index ) const
    {
        return m_level[ index % 3 ];
    }

    // sets level
    virtual void setLevel( int index, uint8_t level )
    {
        m_level[ index % 3 ] = level;
    }

protected:
    Color::rgb32_t m_color[ 3 ];
    uint8_t m_level[ 3 ];
};


enum PatternId
{
    MiniTwinkle,
    MiniSparkle,
    Sparkle,
    Rainbow,
    Flash,
    March,
    Wipe,
    Gradient,
    Fixed,
    Strobe,
    CandyCane,
    Test,
};

// Pattern factory
//--> do this without allocating the Pattern - map IDs to types instead
LIGHTTOOLS_API Pattern *CreatePattern( uint8_t pattern );


// Flash the entire strip
class LIGHTTOOLS_API FlashPattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

// Rainbow!
class LIGHTTOOLS_API RainbowPattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

class LIGHTTOOLS_API SparklePattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Loop( Strip *strip, ms_t offset );
};

class LIGHTTOOLS_API MiniSparklePattern : public SparklePattern
{
public:
    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

class LIGHTTOOLS_API MiniTwinklePattern : public Pattern
{
public:
    MiniTwinklePattern();

    // assume nothing, setup all pixels
    virtual void Init( Strip *strip, ms_t offset );

    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );

protected:
    ms_t delta( ms_t previous, ms_t next, ms_t duration );

    ms_t m_lastDim;
    ms_t m_lastLit;
};

class LIGHTTOOLS_API MarchPattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

class LIGHTTOOLS_API WipePattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

class LIGHTTOOLS_API GradientPattern : public Pattern
{
public:
    GradientPattern( );

    ~GradientPattern( );

    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // assume nothing, setup all pixels
    virtual void Init( Strip *strip, ms_t offset );

    // restarting after a loop expired, but not first call
    virtual void Loop( Strip *strip, ms_t offset );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );

    virtual void setColor( int index, Color::rgb32_t color );

    virtual void setLevel( int index, uint8_t level );

private:
    void ResetGradient();

    Color::Gradient grad;
    uint8_t *mp1, *mp2;
    bool changed = false; // true if color/level changed and need to reset gradient
};

// Strobe the entire strip
class LIGHTTOOLS_API StrobePattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );

    ms_t m_lastOffset;
};

class LIGHTTOOLS_API FixedPattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

class LIGHTTOOLS_API CandyCanePattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

// test patterns
// level[0]:
//    0-15: binary code (code in level[1], color in color[0])
//    16-31: brightness scale (color in color[0])
//    32-47: gradient, using colors[0-2]
class LIGHTTOOLS_API TestPattern : public Pattern
{
public:
    // returns loop duration, time offset never goes above this
    virtual ms_t GetDuration( Strip *strip );

    // update pixels as needed
    virtual void Update( Strip *strip, ms_t offset );
};

} // namespace Pattern
