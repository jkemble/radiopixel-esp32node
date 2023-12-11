#pragma once

#include "CIEXYZ.h"

namespace Color
{
// The obsolete CIE 1960 UCS space, now used only to measure Duv
namespace CIEUCS1960
{
    // a chromaticity point in the UCS
    template<typename T>
    struct uv
    {
        T u = 0, v = 0;

        template< typename Observer = Color::CIEXYZ::CIE1931_2deg_Observer<T>>
        static constexpr std::pair<uv<T>,uv<T>> range()
        {
            Observer observer;
            auto contour(observer.contour());
            std::pair<uv<T>,uv<T>> _range{ { std::numeric_limits<T>::max(), std::numeric_limits<T>::max()},
                { std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest() } };
            for (const auto& xyY : contour)
            {
                uv<T> uv(xyY);
                _range.first.u = std::min( _range.first.u, uv.u );
                _range.first.v = std::min( _range.first.v, uv.v );
                _range.second.u = std::max( _range.second.u, uv.u );
                _range.second.v = std::max( _range.second.v, uv.v );
            }
            return _range;
        }

        constexpr uv( T _u = 0, T _v = 0)
            : u(_u), v(_v)
        {
        }

        // convert XYZ to uv
        constexpr uv( const CIEXYZ::XYZ<T>& xyz)
        {
            T denom( xyz.X + 15 * xyz.Y + 3 * xyz.Z );
            u = 4 * xyz.X / denom;
            v = 6 * xyz.Y / denom;
        }

        // convert xyY to uv
        constexpr uv( const CIEXYZ::xyY<T>& xyY)
        {
            T denom( -2 * xyY.x + 12 * xyY.y + 3 );
            u = 4 * xyY.x / denom;
            v = 6 * xyY.y / denom;
        }

        bool operator==( const uv<T>& rhs ) const
        {
            return u == rhs.u && v == rhs.v;
        }

        // convert uv to xyY
        // would prefer to make this a constructor of xyY but then
        // headers would reference each other and I'd have to break
        // them down further and I don't want to deal with right now
        CIEXYZ::xyY<T> toxyY() const
        {
            T denom( 2 * u - 8 * v + 4 );
            return CIEXYZ::xyY<T>(3 * u / denom, 2 * v / denom);
        }

        // the distance (Duv) from this point to \a rhs
        // returns negative values if rhs is under or to the right of this point
        constexpr T D( const uv<T>& rhs ) const
        {
            T du( u - rhs.u ), dv( v - rhs.v );
            if ( rhs.v < v || rhs.u > u )
            {
                return -sqrt( du * du + dv * dv );
            }
            else
            {
                return sqrt( du * du + dv * dv );
            }
        }

        friend std::ostream& operator<<( std::ostream& stream, const uv<T>& uv )
        {
            stream << "( u=" << uv.u << ", v=" << uv.v << " )";
            return stream;
        }
    };

    // the distance (Duv) from \a lhs to \a rhs
    // returns negative values if rhs is under or to the right of lhs
    template< typename T>
    constexpr T D( const uv<T>& lhs, const uv<T>& rhs )
    {
        return lhs.D( rhs );
    }
}
}
