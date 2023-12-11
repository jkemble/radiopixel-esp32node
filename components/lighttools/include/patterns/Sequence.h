#pragma once

#include <vector>
#include <span>
#include "../export.h"
#include "Player.h"


namespace Pattern
{

// A Sequence is a series of PlayerControl commands, each with their own timing
class LIGHTTOOLS_API Sequence
{
public:
    virtual ~Sequence() {}

    //! start at the first step
    //! returns the new step
    virtual int Reset( );

    //! advance to the next step
    //! \a step is the current step
    //! \a timed is true if the current step has run to completion, false if the user is manually advancing the step
    //! returns the new step
    virtual int Advance( int step, [[maybe_unused]] bool timed = false );

    //! The total number of steps
    virtual int GetStepCount( ) const = 0;

    //! time (in ms) to stay in this step
    virtual ms_t GetDuration( int step ) const = 0;

    //! The player control block for \a step
    virtual const PlayerControl GetCommand( int step ) const = 0;
};

#pragma pack( push, 1 )
// A Step combines the timing and PlayerControl information contained in a Sequence step
struct LIGHTTOOLS_API Step
{
    ms_t duration;
    PlayerControl command;
};
#pragma pack( pop )


// Implements Sequence by storing Steps in a vector
class LIGHTTOOLS_API OrderedSequence : public Sequence
{
public:
    virtual int GetStepCount( ) const override{ return m_steps.size(); }
    virtual ms_t GetDuration( int step ) const override{ return m_steps[ step ].duration; }
    virtual const PlayerControl GetCommand( int step ) const override { return m_steps[ step ].command; }

    void AddStep( Step step ) { m_steps.push_back( step ); }
    void AddSteps( std::span<Step> steps ) { m_steps.insert( m_steps.end(), steps.begin(), steps.end() ); }

protected:
    std::vector<Step> m_steps;
};

// Similar to OrderedSequence, but runs a random step followed by the last
// step.  Manual (timed == false) Advance()s go to a random step, timeout Advance()s
// go to the last step
class LIGHTTOOLS_API RandomSequence : public OrderedSequence
{
public:
    virtual int Reset( );
    virtual int Advance( int step, bool timed = false );
};

// Predefined steps
LIGHTTOOLS_API const Step& idleStep();
LIGHTTOOLS_API const std::span<Step> alertSteps();
LIGHTTOOLS_API const std::span<Step> randomSteps();

} // namespace Pattern
