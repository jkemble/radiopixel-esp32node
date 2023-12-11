#pragma once

#include <array>
#include <set>
#include <span>
#include <numeric>
#include <iostream>
#include "../export.h"
#include "../geometry/line.h"
#include "../geometry/convexhull.h"
#include "CIEXYZ.h"
#include "illuminants.h"


namespace Color
{

enum Objective {
    Maximize,
    Minimize
};

LIGHTTOOLS_API std::vector<double> solveLP( const std::span< CIEXYZ::xyY<double> > emitters, CIEXYZ::xyY<double> target,
    std::set<size_t> constant = std::set<size_t>(), std::set<size_t> objective = std::set<size_t>(),
    Objective objectiveValue = Maximize, double total = 0 );

template< typename T>
class Solver
{
public:
    //! determines the outside indexes for the specified emitters
    //! these are the emitters that determine the gamut, ie not white or
    //! otherwise inside the gamut
    std::vector<size_t> outside( const std::span< CIEXYZ::xyY<T> > emitters )
    {
        // determine the convex hull
        std::vector< CIEXYZ::PointAdapter<T>> all;
        for ( auto emitter : emitters )
        {
            all.push_back( CIEXYZ::PointAdapter( emitter ) );
        }
        Geometry::ConvexHullSolver<T, CIEXYZ::PointAdapter<T>> hullSolver;
        return hullSolver.solve( all );
    }

    //! calculates the emitter levels that achieve the target chromaticity
    //! emitters - the set of all emitters, in any order
    //! target - the desired chromaticity
    //! outside - the indexes of the emitters that define the gamut, in order around the gamut
    //! constant - the indexes of emitters whose levels are constant level, ie emitters we are not solving for
    //! objective - the indexes of emitters to maximize or minimize, by default all non-constant emitters
    //! objectiveValue - the direction to optimize the objective emitters
    //! total - the combined value of all returned levels, or zero for no restriction
    std::vector<T> solve( const std::span< CIEXYZ::xyY<T> > emitters, CIEXYZ::xyY<T> target,
        std::span<size_t> outside = std::span<size_t>(), std::set<size_t> constant = std::set<size_t>(),
        std::set<size_t> objective = std::set<size_t>(), Objective objectiveValue = Maximize, T total = 0 )
    {
        // first, see if target is outside the gamut
        if ( emitters.size() >= 3 )
        {
            auto result( solveOutside( emitters, outside, target, total ) );
            if ( !result.empty( ) )
            {
                if ( objectiveValue == Minimize )
                {
                    return std::vector<T>( emitters.size(), 0 );
                }
                else
                {
                    return result;
                }
            }
        }

        // for 4 or more emitters, look for an in gamut solution using the N way solver
        if ( emitters.size() >= 4 )
        {
            auto result( solveN( emitters, target, constant, objective, objectiveValue, total ) );
            if ( !result.empty( ) )
            {
                return result;
            }
        }

        // for 3 emitters, try an in gamut solution using the optimized solver
        if ( emitters.size() == 3 )
        {
            auto result( solve3( emitters, target, total ) );
            if ( !result.empty( ) )
            {
                return result;
            }
        }

        // --> handle 2 emitters!

        return {};
    }

    std::vector<T> solveN( const std::span< CIEXYZ::xyY<T> > emitters, CIEXYZ::xyY<T> target,
        std::set<size_t> constant = std::set<size_t>(), std::set<size_t> objective = std::set<size_t>(),
        Objective objectiveValue = Maximize, T total = 0 )
    {
        // convert from templated base type to double, since LP solver base type isn't templated
        std::vector< CIEXYZ::xyY<double> > _emitters;
        for ( const auto& _emitter : emitters )
        {
            _emitters.push_back( { _emitter.x, _emitter.y, _emitter.Y });
        }

        // convert targeted from templated type to double
        CIEXYZ::xyY<double> _target{ target.x, target.y, target.Y };

        // call the solver
        auto _levels( solveLP( _emitters, _target, constant, objective, objectiveValue, total ) );

        // convert back to the templated type
        std::vector<T> levels;
        for ( const auto& _level : _levels )
        {
            levels.push_back( _level );
        }

        return levels;
    }

    // Given the emitters and a target, return the brightness level of
    // each emitter needed to obtain target, if the target is in the gamut
    // If the target is not in the gamut then use solveOutside.
    std::vector<T> solve3( const std::span< CIEXYZ::xyY<T> > emitters, CIEXYZ::xyY<T> target, T total = 0 )
    {
        // if the target is on emitter[2] then return it now
        // we need the line formed by target and emitter[2], and there isn't
        // one if they're the same point
        if ( emitters[2].x == target.x && emitters[2].y == target.y )
        {
            std::vector<T> levels{ 0, 0, emitters[2].Y };
            if ( total && total > levels[2] )
            {
                levels[2] = total;
            }
            return levels;
        }

        // intersection point, from [0] to [1]
        // this is the virtual emitter
        T t( Geometry::intersect<T>( pt(emitters[0]), pt(emitters[1]), pt(emitters[2]), pt(target)));
        if ( t < 0 || t > 1 )
        {
            // outside gamut
            return {};
        }

        // get the [0] and [1] levels needed to make the virtual emitter
        auto S01( solve2(emitters[0], emitters[1], t));

        // make the virtual emitter
        //--> optimize this, part of it is already calculated by solve2
        auto evpt( Geometry::project( pt(emitters[0]), pt(emitters[1]), t ) );
        CIEXYZ::xyY<T> ev{ evpt.x(), evpt.y(), S01[0] + S01[1] };

        // now solve again between emitter[2] and ev

        // intersection point, from emitter[2] to ev
        T u( (target.x - emitters[2].x) / (ev.x - emitters[2].x) );
        if ( u < 0 || u > 1 )
        {
            // outside gamut
            return {};
        }

        // get the [2] and ev levels needed to hit the target
        auto S2v( solve2(emitters[2], ev, u) );
        auto lv( S2v[1] / ev.Y ); // dim level of virtual emitter
        std::vector<T> levels{ S01[0] * lv, S01[1] * lv, S2v[0] };

        // limit to desired total, if requested
        if ( total )
        {
            auto combined( std::accumulate( levels.begin(), levels.end(), 0.0 ) );
            if ( combined > total )
            {
                for ( auto& level : levels )
                {
                    level *= total / combined;
                }
            }
        }

        return levels;
    }

    std::vector<T> solve2( const std::span< CIEXYZ::xyY<T> > emitters, CIEXYZ::xyY<T> target, T total = 0 )
    {
        //-->TODO: calculate "ra" point such that line target<->ra is perpendicular to line emitter[0]<->emitter[1]
        // (invert the slope of the emitter line, watching for divide by zero)
        // then get intersection of the lines
        (void)emitters;
        (void)target;
        (void)total;
        return {};
    }

    std::vector<T> solveOutside( const std::span<CIEXYZ::xyY<T>> emitters,
        const std::span<size_t> outside, CIEXYZ::xyY<T> target, T total = 0 )
    {
        std::vector<T> solution;
        constexpr auto white( Color::CIE_Illuminant_D_xyY<T>(6500) );

        for ( size_t i = 0; i < outside.size(); ++i)
        {
            size_t current( outside[ i ] );
            size_t next( outside[ (i+1) % outside.size() ] );
            T t(Geometry::intersect<T>(pt(white), pt(target), pt(emitters[current]), pt(emitters[next])));
            if ( t >= 0 && t <=1 )
            {
                T u(Geometry::intersect<T>(pt(emitters[current]), pt(emitters[next]), pt(white), pt(target)));
                if ( u >= 0 && u <=1 )
                {
                    auto S( solve2(emitters[current], emitters[next], u));
                    solution.resize(emitters.size(), 0);
                    solution[ current ] = S[ 0 ];
                    solution[ next ] = S[ 1 ];
                    break;
                }
            }
        }

        // limit to desired total, if requested
        if ( !solution.empty() && total )
        {
            auto combined( std::accumulate( solution.begin(), solution.end(), 0.0 ) );
            if ( combined > total )
            {
                for ( auto& level : solution )
                {
                    level *= total / combined;
                }
            }
        }

        return solution;
    }

protected:
    // Given emitters e1 and e2, and a target distance from e1
    // to e2 (ie target 0 = e1, 1 = e2), return the luminance
    // of e1 and e2 needed to obtain the desired color.
    std::array<T,2> solve2(CIEXYZ::xyY<T> e1, CIEXYZ::xyY<T> e2, T target)
    {
        // trivial case, avoid divide by zero
        if ( target == 0 )
        {
            return { e1.Y, 0 };
        }

        // x coordinate of target
        T xmix( e1.x + target * (e2.x - e1.x) );

        // ratio of e1 / e2 luminance
        // https://en.wikipedia.org/wiki/CIE_1931_color_space
        T ratio( ( e1.y * ( e2.x - xmix ) ) /
                 ( e2.y * ( xmix - e1.x ) ) );

        // dim either e1 or e2 to achieve required ratio
        if ( ratio < ( e1.Y / e2.Y) )
        {
            // dim e1
            return { e2.Y * ratio, e2.Y };
        }
        else
        {
            // dim e2
            return { e1.Y, e1.Y / ratio };
        }
    }

    // Returns a point adaptor for \a xyy
    CIEXYZ::PointAdapter< T > pt( CIEXYZ::xyY<T> xyy )
    {
        return CIEXYZ::PointAdapter< T >( xyy );
    }
};

}
