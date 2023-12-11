#include "patterns/Player.h"


namespace Pattern
{

void Player::UpdatePattern( ms_t now, const PlayerControl& control, Strip *strip )
{
    // update intensity
    strip->setBrightness( control.intensity );

    // update pattern
    bool init( !m_pattern || control.pattern != m_patternId );
    if ( init )
    {
        delete m_pattern;
        m_patternId = control.pattern;
        m_pattern = CreatePattern( m_patternId );
    }

    // update speed
    // --> I suspect there are bugs here .. 
    // --> should probably track m_count (cycle count) and m_update (last update timestamp)
    if ( !init && m_speed != control.speed )
    {
        // adjust start and offset to match new speed
        ms_t duration( m_pattern->GetDuration( strip ) );
        ms_t elapsed = ( ( now - m_start ) * m_speed / 100 );
        if ( control.speed == 0 )
        {
            // speed dropped to zero, set offset to where we are right now
            m_offset = elapsed % duration;
        }
        else if ( m_speed == 0 )
        {
            // speed coming up off zero, set start so count and offset stay stable
            // --> if count is large then this could go negative .. 
            // --> don't track this way, see above
            m_start = now - ( m_count * duration ) * 100 / control.speed - m_offset;
        }
        else
        {
            // solve for new start time so the elapsed time (and thus offset
            // and cycle count) stay the same
            m_start = now - ( elapsed * 100 / control.speed );
        }

        // and store the new speed
        m_speed = control.speed;
    }

    // update colors
    for ( int i = 0; i < 3; ++i )
    {
        auto c( control.color[ i ] );
        m_pattern->setColor( i, Color::rgb32_t( c.r, c.g, c.b ) );
    }

    // update levels
    for ( int i = 0; i < 3; ++i )
    {
        m_pattern->setLevel( i, control.level[ i ] );
    }

    // init the pattern if needed
    if ( init )
    {
        m_pattern->Init( strip, 0 );
        m_start = now;
        m_offset = 0;
        m_count = 0;
        m_speed = control.speed;
    }
}

void Player::UpdateStrip( ms_t now, Strip *strip )
{
    // update the strip if it's time
    if ( m_pattern && strip )
    {
        if ( m_speed == 0 )
        {
            // stopped at the previous offset
            m_pattern->Update( strip, m_offset );
        }
        else
        {
            ms_t duration( m_pattern->GetDuration( strip ) );
            ms_t elapsed = ( ( now - m_start ) * m_speed / 100 );
            m_offset = elapsed % duration;
            unsigned long count = elapsed / duration;
            if ( count != m_count )
            {
                m_count = count;
                m_pattern->Loop( strip, m_offset );
            }
            else
            {
                m_pattern->Update( strip, m_offset );
            }
        }
    }
}

} // namespace Pattern
