#include <array>
#include "CIEXYZ.h"
#include "../geometry/norm.h"
#include "Solver.h"


namespace Color
{
namespace sRGB
{

// https://en.wikipedia.org/wiki/SRGB
template< typename T >
constexpr std::array< CIEXYZ::xyY< T >, 3 > primaries()
{
    return std::array< CIEXYZ::xyY< T >, 3 >{{
        { 0.64, 0.33, 0.2126 },
        { 0.30, 0.60, 0.7152 },
        { 0.15, 0.06, 0.0722 }
    }};
}

// https://en.wikipedia.org/wiki/SRGB
template< typename T >
constexpr CIEXYZ::xyY< T > whitepoint()
{
    return CIEXYZ::xyY< T >{ 0.3127, 0.3290, 1.0 };
}

// Convert linear brightness to gamma compressed sRGB
// input and output is in the interval [0-1]
// https://en.wikipedia.org/wiki/SRGB#From_CIE_XYZ_to_sRGB
template< typename T >
T oetf( T in )
{
    return ( in <= 0.0031308 ) ?
        ( in * 12.92 ) :
        ( pow( in, 1/2.4) * 1.055 - 0.055 );
}

// Convert gamma compressed sRGB to linear brightness
// input and output is in the interval [0-1]
// https://en.wikipedia.org/wiki/SRGB#From_sRGB_to_CIE_XYZ
template< typename T >
T eotf( T in )
{
    return ( in <= 0.04045 ) ?
        ( in / 12.92 ) :
        pow( ( in + 0.055 ) / 1.055, 2.4 );
}

// an sRGB color point
template< typename T >
struct color
{
    // gamma compressed levels, in the interval [0-1]
    T r = 0, g = 0, b = 0;

    color(T _r = 0, T _g = 0, T _b = 0)
        :r(_r), g(_g), b(_b)
    {
    }

    // Convert \a xyY to sRGB.  \a The Y value must be the range [0,1]
    // and is interpreted as a brightness factor consistently across
    // all chromaticities, rather than each chromaticity having it's
    // own maximum brightness, with the maximum brightness of D65 being
    // 1, blue being 0.0722, etc.
    // Applies a power-normal of \a power if specified.
    color( CIEXYZ::xyY<T> xyY, T power = 1)
    {
        // get the brightest emitter levels for this chromaticity
        auto emitters( primaries<T>());
        std::vector<size_t> outside{ 0, 1, 2 };
        Color::Solver<T> solver;
        auto levels( solver.solve(emitters, xyY, outside));
        if ( levels.empty() ) // ..? but I've seen it happen around blue primary ..edge case when outside?
        {
            r = g = b = 0;
            return;
        }

        // convert levels from Y values to intensity [0,1]
        levels[ 0 ] /= emitters[ 0 ].Y;
        levels[ 1 ] /= emitters[ 1 ].Y;
        levels[ 2 ] /= emitters[ 2 ].Y;

        // apply the brightness
        levels[ 0 ] *= xyY.Y;
        levels[ 1 ] *= xyY.Y;
        levels[ 2 ] *= xyY.Y;

        // apply a p-norm to the brightness to dim down the peaks
        // where multiple emitters are at full .. this eliminates the
        // star pattern seen extending from the center to the CMY points
        if ( power != 1.0 )
        {
            auto norm( Geometry::pnorm<T>( levels, power ) );
            for ( auto& level : levels )
            {
                level /= norm;
            }
        }

        // encode with the sRGB transfer function and set the pixel value
        r = oetf<T>( levels[ 0 ] );
        g = oetf<T>( levels[ 1 ] );
        b = oetf<T>( levels[ 2 ] );
    }

    bool operator==( const color<T>& rhs ) const
    {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    CIEXYZ::xyY<T> toxyY() const
    {
        // convert from compressed to linear power values
        T _r( eotf<T>(r));
        T _g( eotf<T>(g));
        T _b( eotf<T>(b));

        // set primaries to their respective intensities
        auto _primaries(primaries<T>());
        _primaries[0].Y *= _r;
        _primaries[1].Y *= _g;
        _primaries[2].Y *= _b;

        // mix the primaries together
        return CIEXYZ::mix<T>(_primaries);
    }

    friend std::ostream& operator<<( std::ostream& stream, const color<T>& srgb )
    {
        stream << "( r=" << srgb.r << ", g=" << srgb.g << ", b=" << srgb.b << " )";
        return stream;
    }
};

} // namespace sRGB
} // namespace Color
