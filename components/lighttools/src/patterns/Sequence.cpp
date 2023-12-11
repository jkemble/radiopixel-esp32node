#include <cstdlib> // rand
#include "patterns/Pattern.h"
#include "patterns/Sequence.h"


namespace Pattern
{

const uint8_t FULL = 255;
const uint8_t HALF = 127;
const uint8_t LOW = 10;

const Step& idleStep()
{
    static const Step step =
        {     0, { LOW, Gradient, 35, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 17 } };
    return step;
}

const std::span<Step> alertSteps()
{
    static std::array<Step, 8> steps
    {{
        {  4000, { FULL, Flash,       100, Color::rgb24_t::Yellow(), Color::rgb24_t::Yellow(), Color::rgb24_t::Yellow(), 255 } },
        { 60000, { FULL, March,        40, Color::rgb24_t::Yellow(), Color::rgb24_t::Yellow(), Color::rgb24_t::Yellow(),  34 } },
        { 60000, { FULL, MiniTwinkle, 100, Color::rgb24_t::Yellow(), Color::rgb24_t{ 255, 255, 64 }, Color::rgb24_t::Yellow(),  75 } },
        {     0, { HALF, Gradient,     75, Color::rgb24_t::Yellow(), Color::rgb24_t{ 255, 255, 64 }, Color::rgb24_t::Yellow(),  75 } },
        {  4000, { FULL, Flash,       100, Color::rgb24_t::Red(), Color::rgb24_t::Red(), Color::rgb24_t::Red(), 255 } },
        { 60000, { FULL, March,        40, Color::rgb24_t::Red(), Color::rgb24_t::Red(), Color::rgb24_t::Red(),  34 } },
        { 60000, { FULL, MiniTwinkle, 100, Color::rgb24_t::Red(), Color::rgb24_t{ 255, 64, 64 }, Color::rgb24_t::Red(),  75 } },
        {     0, { HALF, Gradient,     75, Color::rgb24_t::Red(), Color::rgb24_t{ 255, 64, 64 }, Color::rgb24_t::Red(),  75 } }
    }};
    return std::span<Step>(steps);
}

const std::span<Step> randomSteps()
{
    static std::array<Step, 14> steps
    {{
        { 30000, { FULL, MiniTwinkle, 160, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Yellow(), 160 } }, // rwy twinkle
        { 30000, { FULL, MiniTwinkle, 160, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 160 } }, // rwg twinkle
        { 30000, { FULL, Gradient,     35, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Red(), 17 } }, // rwr subtle
        { 30000, { FULL, Gradient,     75, Color::rgb24_t::Blue(), Color::rgb24_t{ 128, 128, 255 }, Color::rgb24_t::Blue(), 75 } }, // blue smooth
        { 30000, { FULL, MiniTwinkle, 160, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Blue(), 160 } }, // rwb
        { 30000, { HALF, CandyCane,    65, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 255 } }, // rwg candy
        { 30000, { HALF, CandyCane,   100, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Red(), 255 } }, // rwr candy
        { 30000, { FULL, Fixed,       100, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 255 } }, // rwg tree
        { 30000, { FULL, March,       127, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 8 } }, // rwg march
        { 30000, { FULL, Wipe,        127, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 8 } }, // rwg wipe
        { 30000, { FULL, MiniSparkle, 255, Color::rgb24_t::Red(), Color::rgb24_t::White(), Color::rgb24_t::Green(), 9 } }, // rwg flicker
        { 30000, { FULL, MiniTwinkle, 100, Color::rgb24_t::Cyan(), Color::rgb24_t::Magenta(), Color::rgb24_t::Yellow(), 128 } }, // cga
        { 30000, { HALF, Rainbow,     100, Color::rgb24_t::White(), Color::rgb24_t::White(), Color::rgb24_t::White(), 255 } }, //  rainbow
        { 30000, { HALF, Strobe,      128, Color::rgb24_t::White(), Color::rgb24_t::White(), Color::rgb24_t::White(), 255 } } // strobe
    }};
    return std::span<Step>(steps);
}


int Sequence::Reset( )
{
    return 0;
}

int Sequence::Advance( int step, [[maybe_unused]] bool timed )
{
    if (timed)
    {
        // timeout - advance one step
        step++;
        return (step < GetStepCount( )) ? step : -1;
    }
    else
    {
        // manual - advance to step after the next 0 duration (no wait time) step
        while ((step < (GetStepCount()-1)) && GetDuration(step))
        {
            step++;
        }
        step++;
        return (step < GetStepCount( )) ? step : -1;
    }
}


int RandomSequence::Reset( )
{
    return std::rand( ) % ( GetStepCount( ) - 1 );
}

int RandomSequence::Advance( [[maybe_unused]] int step, bool timed )
{
    if ( timed )
    {
        // timeout - release
        return -1;
    }
    else
    {
        // manual advance - restart
        return Reset( );
    }
}

} // namespace Pattern
