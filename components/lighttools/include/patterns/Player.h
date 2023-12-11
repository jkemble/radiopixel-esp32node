#pragma once

#include "../export.h"
#include "Pattern.h"

namespace Pattern
{

#pragma pack( push, 1 )
// The binary pattern control interface
struct PlayerControl
{
    uint8_t intensity;
    uint8_t pattern;
    uint8_t speed;
    Color::rgb24_t color[ 3 ];
    uint8_t level[ 3 ] = { 0x80, 0x80, 0x80 };
};
#pragma pack( pop )


// Manages the playback state of a Pattern, including looping and dynamic speed adjustments
class LIGHTTOOLS_API Player
{
public:
    Player()
        : m_start( 0 ), m_pattern( 0 ), m_patternId( -1 ),
          m_offset( 0 ), m_count( 0 ), m_speed( 35 )
    {
    }

    //! update to the player with a new pattern, and/or changed pattern parameters
    void UpdatePattern( ms_t now, const PlayerControl& control, Strip *strip );

    //! update the strip with the current pattern
    void UpdateStrip( ms_t now, Strip *strip );

protected:
    ms_t m_start; // time we started the current pattern, adjusted when changing speed

    Pattern *m_pattern;
    uint8_t m_patternId;
    ms_t m_offset; // the last offset we were at, needed when speed is 0
    unsigned long m_count; // the cycle count when Loop() was last called, scaled by speed
    uint8_t m_speed; // speed as a percent, ie 0 - 255%
};

} // namespace Pattern
