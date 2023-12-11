#pragma once

#include <limits>
#include <cassert>
#include <cmath>
#include <vector>


namespace Color
{
    // The visible wavelengths
    template< typename T >
    constexpr std::pair<T, T> visible()
    {
        return std::make_pair<T, T>( 380, 750 );
    }

    // Spectral Power Distribution
    // Start must be equal or less than end, and there must be at least one value.
    // Delta must be non-zero.
    template< typename T >
    class SPD
    {
    public:
        // ctor
        constexpr SPD( T start, T delta, const std::vector<T>& powers )
            : m_start( start ), m_end( start + (powers.size() - 1) * delta ),
            m_delta( delta ), m_powers( powers )
        {
            assert( start > 0 );
            assert( powers.size() >= 1 );
            assert( delta > 0 );
        }

        // special case for a single wavelength
        constexpr SPD( T start )
            : SPD( start, 1, { 1 } )
        {
        }

        SPD<T>& operator*=( T factor )
        {
            for ( auto& power : m_powers )
            {
                power *= factor;
            }

            return *this;
        }

        SPD<T>& operator+=( const SPD<T>& rhs )
        {
            int index = 0;
            for ( T wavelength = m_start; wavelength <= m_end; wavelength += m_delta )
            {
                m_powers[ index ] += rhs.power( wavelength );
                ++index;
            }

            return *this;
        }

        T start() const
        {
            return m_start;
        }

        T end() const
        {
            return m_end;
        }

        T delta() const
        {
            return m_delta;
        }

        // returns the power at \a wavelength, interpolating if needed
        // returns 0 for wavelengths below start or above end
        T power( T wavelength ) const
        {
            // check for outside the boundaries
            if ( wavelength < m_start || wavelength > m_end )
            {
                return 0;
            }

            // avoid divide by zero for single wavelength case
            if ( m_delta == 0 )
            {
                return m_powers[ 0 ];
            }

            // get the value on the left side of the wavelength
            T index( ( wavelength - m_start ) / m_delta );
            size_t whole( trunc( index ) );
            T power_( m_powers[ whole ] );

            // interpolate to the next value if needed
            T frac( index - whole );
            if ( frac > 0.01 )
            {
                T next( m_powers[ whole + 1 ] );
                power_ += ( next - power_ ) * frac;
            }

            return power_;
        }

    private:
        T m_start, m_end, m_delta;
        std::vector< T > m_powers;
    };

    template<typename T>
    inline SPD<T> operator+( SPD<T> lhs, const SPD<T>& rhs )
    {
        lhs += rhs;
        return lhs;
    }

    template<typename T>
    inline SPD<T> operator*( SPD<T> lhs, T rhs )
    {
        lhs *= rhs;
        return lhs;
    }

    template<typename T>
    inline SPD<T> operator*( T lhs, SPD<T> rhs )
    {
        rhs *= lhs;
        return rhs;
    }
}
