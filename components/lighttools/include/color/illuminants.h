#pragma once

#include "SPD.h"
#include "CIEXYZ.h"


namespace Color
{

// SPD of a black body radiator
// https://en.wikipedia.org/wiki/Planckian_locus
// https://en.wikipedia.org/wiki/Color_temperature
template<typename T>
SPD<T> BlackBody(T cct, T start = visible<T>().first, T end = visible<T>().second, T delta = 5) {
    // define some juicy constants
    const T h( 6.62607015e-34 ); // Planck's constant, J/Hz-1
    const T c( 299792458 ); // the speed of light, meters/sec
    const T k( 1.380649e-23); // Boltzmann's constant, J K-1
    const T c2( h * c / k );

    // setup a vector to hold the powers
    std::vector<T> powers;
    powers.reserve( end - start + 1 );

    // calculate the raw powers
    // we ignore c1 since we only care about the relative power
    T peak = 0;
    for ( T wavelength = start; wavelength <= end; wavelength += delta)
    {
        T lambda( wavelength * 1.0e-9 ); // wavelength in meters
        T power( 1 / ( pow( lambda, 5 ) * ( exp( c2 / ( lambda * cct ) ) - 1 ) ) );
        powers.push_back( power );
        peak = std::max( peak, power );
    }

    // convert to relative power by divinding everything by peak power
    for ( T& power : powers )
    {
        power = power / peak;
    }

    return SPD<T>(start, delta, powers);
}

// CIE A
// https://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_A
template< typename T >
SPD<T> CIE_Illuminant_A_SPD( T start = visible<T>().first, T end = visible<T>().second )
{
    const T delta = 5;
    const T CT = 2856;
    return BlackBody<T>( CT, start, end, delta );
}

template< typename T >
constexpr CIEXYZ::xyY<T> CIE_Illuminant_A_xyY()
{
    return CIEXYZ::xyY<T>{ 0.44758, 0.40745, 1.0 };
}

// CIE D
// https://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_series_D
template< typename T >
constexpr CIEXYZ::xyY<T> CIE_Illuminant_D_xyY(T cct);

// cct should be in the interval [4000, 25000]
template< typename T >
SPD<T> CIE_Illuminant_D_SPD( T cct = 6500 )
{
    // start with chromaticity
    auto xyY( CIE_Illuminant_D_xyY<T>(cct));

    // calculate multipliers from chromaticity
    T M =  ( 0.0241 + 0.2562 * xyY.x - 0.7341 * xyY.y);
    T M1 = (-1.3515 - 1.7703 * xyY.x + 5.9114 * xyY.y) / M;
    T M2 = ( 0.0300 -31.4424 * xyY.x +30.0717 * xyY.y ) / M;

    // component SPDs based on eigenvectors
    // from Wyszecki and Stiles, Table V(3.3.4)
    // downloaded from https://www.rit.edu/science/munsell-color-science-lab-educational-resources#useful-color-data
    const SPD<T> S0( 300, 10, {
        0.04, 6, 29.6, 55.3, 57.3, 61.8, 61.5, 68.8, 63.4, 65.8, 94.8,
        104.8, 105.9, 96.8, 113.9, 125.6, 125.5, 121.3, 121.3, 113.5,
        113.1, 110.8, 106.5, 108.8, 105.3, 104.4, 100, 96, 95.1, 89.1,
        90.5, 90.3, 88.4, 84, 85.1, 81.9, 82.6, 84.9, 81.3, 71.9, 74.3,
        76.4, 63.3, 71.7, 77, 65.2, 47.7, 68.6, 65, 66, 61, 53.3, 58.9, 61.9
    }),
    S1( 300, 10, {
        0.02, 4.5, 22.4, 42, 40.6, 41.6, 38, 42.4, 38.5, 35, 43.4, 46.3,
        43.9, 37.1, 36.7, 35.9, 32.6, 27.9, 24.3, 20.1, 16.2, 13.2, 8.6,
        6.1, 4.2, 1.9, 0, -1.6, -3.5, -3.5, -5.8, -7.2, -8.6, -9.5, -10.9,
        -10.7, -12, -14, -13.6, -12, -13.3, -12.9, -10.6, -11.6, -12.2,
        -10.2, -7.8, -11.2, -10.4, -10.6, -9.7, -8.3, -9.3, -9.8
    }),
    S2( 300, 10, {
        0, 2, 4, 8.5, 7.8, 6.7, 5.3, 6.1, 2, 1.2, -1.1, -0.5, -0.7,
        -1.2, -2.6, -2.9, -2.8, -2.6, -2.6, -1.8, -1.5, -1.3, -1.2,
        -1, -0.5, -0.3, 0, 0.2, 0.5, 2.1, 3.2, 4.1, 4.7, 5.1, 6.7,
        7.3, 8.6, 9.8, 10.2, 8.3, 9.6, 8.5, 7, 7.6, 8, 6.7, 5.2, 7.4,
        6.8, 7, 6.4, 5.5, 6.1, 6.5,
    });

    return S0 + M1 * S1 + M2 * S2;
}

// cct must be in the interval [4000, 25000]
template< typename T >
constexpr CIEXYZ::xyY<T> CIE_Illuminant_D_xyY(T cct)
{
    // calculate chromaticity - not on the Planckian locus!
    T xD = ( cct <= 7000) ?
        ( ( ( -4.6070E9 / cct + 2.9678E6 ) / cct + 0.09911E3 ) / cct + 0.244063 ) :
        ( ( ( -2.0064E9 / cct + 1.9018E6 ) / cct + 0.24748E3 ) / cct + 0.237040 );
    T yD = ( -3 * xD + 2.87 ) * xD - 0.275;

    return CIEXYZ::xyY<T>{ xD, yD };
}

// CIE E: equal energy
template< typename T >
SPD<T> CIE_Illuminant_E_SPD( T start = visible<T>().first, T end = visible<T>().second )
{
    const T delta = 5;
    return SPD<T>( start, delta, std::vector<T>( ( end - start ) / delta + 1, 1.0 ) );
}

template< typename T >
constexpr CIEXYZ::xyY<T> CIE_Illuminant_E_xyY()
{
    return CIEXYZ::xyY<T>{ 1.0/3, 1.0/3, 1.0/3 };
}

}
