#pragma once

#include "CIEXYZ.h"
#include "CIEUCS1960.h"


namespace Color
{

// Approximate CCT from CIE xyY (1931 I assume)
// T should be in [3000, 25000]
// https://en.wikipedia.org/wiki/Color_temperature#Approximation
// Hernández-Andrés, Javier; Lee, RL; Romero, J (September 20, 1999).
// "Calculating Correlated Color Temperatures Across the Entire Gamut of
// Daylight and Skylight Chromaticities" (PDF). Applied Optics. 38 (27): 5703–5709.
template<typename T>
T CCTfromxyY_HernandezAndresLeeRomero( CIEXYZ::xyY<T> xyY )
{
    // constants for the low and high temperature versions
    struct Constants {
        T xe, ye;
        T A0, A1, A2, A3;
        T t1, t2, t3;
    } low = {
        0.3366, 0.1735,
        -949.86315, 6253.80338, 28.70599, 0.00004,
        0.92159, 0.20039, 0.07125
    }, high = {
        0.3356, 0.1691,
        36284.48953, 0.00228, 5.4535E-36, 0,
        0.07861, 0.01543, 0
    };

    // the formula
    auto f = [](Constants& c, T x, T y)
    {
        T n = (x-c.xe)/(y-c.ye);
        return c.A0 +
            c.A1 * exp(-n/c.t1) +
            c.A2 * exp(-n/c.t2) +
            (c.t3 ? c.A3 * exp(-n/c.t3) : 0);
    };

    // try the low temperature version, and use the high temperature
    // version if needed
    T CCT = f( low, xyY.x, xyY.y );
    if ( CCT >= 50000 )
    {
        CCT = f( high, xyY.x, xyY.y );
    }

    return CCT;
}

// Approximate CCT from CIE xyY (1931 I assume)
// Ohno, 2011
// CCT result accuracy:
// * <=0.40% error with T in [1800, 2100]
// * <=0.08% error with T in [2100, 12400]
// * <=0.40% error with T in [12400, 25000]
// * <=0.50% error with T in [25000, 35000]
// Duv result accuracy: (when xyY on Planckian locus!)
// * <=0.0005 in [1800, 2100]
// * <=0.000035 in [2100, 35000]
// https://cormusa.org/wp-content/uploads/2018/04/CORM_2011_Calculation_of_CCT_and_Duv_and_Practical_Conversion_Formulae.pdf
template<typename T>
std::pair<T, T> CCTDuvfromxyY_Ohno( CIEXYZ::xyY<T> xyY )
{
    T k[7][7] =
    {
        //{ -0.471106, 1.925865, -2.4243787, 1.5317403, -0.5179722, 0.0893944, -0.00616793}, // from Leukos 2013 paper
        { -1.77348E-1,1.115559E+0,-1.5008606E+0,9.750013E-1,-3.307009E-1,5.6061400E-2,-3.7146000E-3 }, // from CORM 2011 presentation
        { 5.308409E-04,2.1595434E-03,-4.3534788E-03,3.6196568E-03,-1.589747E-03,3.5700160E-04,-3.2325500E-05 },
        { -8.58308927E-01,1.964980251E+00,-1.873907584E+00,9.53570888E-01,-2.73172022E-01,4.17781315E-02,-2.6653835E-03 },
        { -2.3275027E+02,1.49284136E+03,-2.7966888E+03,2.51170136E+03,-1.1785121E+03,2.7183365E+02,-2.3524950E+01 },
        { -5.926850606E+08,1.34488160614E+09,-1.27141290956E+09,6.40976356945E+08,-1.81749963507E+08,2.7482732935E+07,-1.731364909E+06 },
        { -2.3758158E+06,3.89561742E+06,-2.65299138E+06,9.60532935E+05,-1.9500061E+05,2.10468274E+04,-9.4353083E+02 },
        { 2.8151771E+06,-4.11436958E+06,2.48526954E+06,-7.93406005E+05,1.4101538E+05,-1.321007E+04,5.0857956E+02 }
    };
    auto poly = [&k]( size_t i, T a ) {
        // Horner's method
        return (((((k[i][6]*a + k[i][5])*a + k[i][4])*a + k[i][3])*a + k[i][2])*a + k[i][1])*a + k[i][0];
    };

    CIEUCS1960::uv<T> TP(xyY), FP(0.292, 0.240);
    T du(TP.u - FP.u), dv(TP.v - FP.v);
    T Lfp(sqrt(du*du + dv*dv));
    //T a1(atan(dv/du));
    //T a((a1 >= 0) ? a1 : (a1 + M_PI));
    T a(acos(du/Lfp));
    T Lbb(poly(0, a));
    T Duv(Lfp - Lbb);
    T T1, dTc1;
    if (a < 2.54)
    {
        T1 = 1/poly(1,a);
        dTc1 = poly(3,a) * ((Lbb+0.01)/Lfp) * (Duv/0.01);
    }
    else
    {
        T1 = 1/poly(2,a);
        dTc1 = poly(4,a) * ((Lbb+0.01)/Lfp) * (Duv/0.01);
    }
    T T2(T1 - dTc1);
    T c(log10(T2));
    T dTc2;
    if (Duv >= 0)
    {
        dTc2 = poly(5,c);
    }
    else
    {
        dTc2 = poly(6,c) * pow(Duv/0.03,2);
    }
    return std::make_pair( T2-dTc2, Duv );
}

// Approximate CCT and Duv from CIE xyY (1931 I assume)
// T should be in [1800, 35000]
template<typename T>
std::pair<T, T> CCTDuvfromxyY( CIEXYZ::xyY<T> xyY )
{
    return CCTDuvfromxyY_Ohno<T>(xyY); // more precise, but use double!
}


// forward declare
template<typename T>
CIEXYZ::xyY<T> xyYfromCCT( T CCT );

// calculate Duv from xyY and CCT
// T should be in [3000, 25000]
// Duv is only meaningful in the interval [-0.05,0.05]
// Ohno, 2011
// https://cormusa.org/wp-content/uploads/2018/04/CORM_2011_Calculation_of_CCT_and_Duv_and_Practical_Conversion_Formulae.pdf
template<typename T>
T DuvfromxyY( CIEXYZ::xyY<T> xyY, T CCT )
{
    // convert CCT to the xyY on the Planckian locus
    CIEXYZ::xyY<T> locusxyY(xyYfromCCT(CCT));

    // convert both the original xyY and the planckian xyY to UCS
    CIEUCS1960::uv<T> targetuv(xyY);
    CIEUCS1960::uv<T> locusuv(locusxyY);

    // return the uv distance (Duv)
    return CIEUCS1960::D(locusuv, targetuv);
}

// Kim et al, US Patent 7024034
// T should be in [1650, 35000]
// results have <=0.120% error with T in [1650, 3250]
// results have <=0.045% error with T in [3250, 35000]
template<typename T>
CIEXYZ::xyY<T> xyYfromCCT_Kim( T CCT )
{
    // constants for the x formula
    struct xConstants { T G, M, K, C; }
    xlow{  -0.2661239E9, -0.2343580E6, 0.8776956E3, 0.179910 },
    xhigh{ -3.0258469E9,  2.1070379E6, 0.2226347E3, 0.240390 };
    // constants for the y formula
    struct yConstants { T C3, C2, C1, C0; }
    ylow{ -1.1063814, -1.34811020, 2.18555832, -0.20219683 },
    ymid{ -0.9549476, -1.37418593, 2.09137015, -0.16748867 },
    yhigh{ 3.0817580, -5.87338670, 3.75112997, -0.37001483 };

    // the formulas
    auto xf = [](xConstants& c, T CCT)
    {
        return c.G / pow(CCT, 3) + c.M / pow(CCT, 2) + c.K / CCT + c.C;
    };
    auto yf = [](yConstants& c, T xc)
    {
        return c.C3 * pow(xc, 3) + c.C2 * pow(xc, 2) + c.C1 * xc + c.C0;
    };

    // call the formula with the applicable constants
    T xc( ( CCT < 4000 ) ? xf( xlow, CCT ) :
                           xf( xhigh, CCT ) );
    T yc( ( CCT < 2222 ) ? yf( ylow, xc ) :
        ( ( CCT < 4000 ) ? yf( ymid, xc ) :
                           yf( yhigh, xc )));

    return CIEXYZ::xyY{ xc, yc };
}

// Krystek 1985
// T should be in [1000, 15000]
// results have <=0.10% error with T in [1000, 6000]
// results have <=0.07% error with T in [6000, 14300]
// results have <=0.62% error with T in [14300, 25000]
// results have <=1.00% error with T in [25000, 35000]
template<typename T>
CIEXYZ::xyY<T> xyYfromCCT_Krystek( T CCT )
{
    T u = ( 0.860117757 + 1.54118254E-4 * CCT + 1.28641212E-7 * CCT * CCT) /
        ( 1 + 8.42420235E-4 * CCT + 7.08145163E-7 * CCT * CCT );
    T v = ( 0.317398726 + 4.22806245E-5 * CCT + 4.20481691E-8 * CCT * CCT) /
        ( 1 - 2.89741816E-5 * CCT + 1.61456053E-7 * CCT * CCT );
    return CIEUCS1960::uv<T>( u, v ).toxyY();
}

// CIE1931 2 degree coordinates for a given CCT
// CCT must be in the interval [1000, 35000]
// results have <=0.100% error with CCT in [1000, 3500]
// results have <=0.045% error with CCT in [3500, 35000]
// https://en.wikipedia.org/wiki/Planckian_locus#Approximation
template<typename T>
CIEXYZ::xyY<T> xyYfromCCT( T CCT )
{
    // Krystek is more accurate down to the lower values
    if ( CCT > 3500 )
    {
        return xyYfromCCT_Kim( CCT );
    }
    else
    {
        return xyYfromCCT_Krystek( CCT );
    }
}

// calculate xyY from CCT and Duv
// CCT must be in the interval [1000, 25000]
// Duv is only meaningful in the interval [-0.05,0.05]
// this uses the method from
// Ohno, Y. (2013), Practical Use and Calculation of CCT and Duv, Leukos, [online]
// https://doi.org/10.1080/15502724.2014.839020
template<typename T>
CIEXYZ::xyY<T> xyYfromCCTDuv( T CCT, T Duv = 0 )
{
    // first get the locus xyY for the CCT
    CIEXYZ::xyY<T> locusxyY(xyYfromCCT(CCT));

    // shortcut when Duv is zero
    if (Duv == 0)
    {
        return locusxyY;
    }

    // next get an xyY for a slightly higher CCT, to get the slope of
    // the locus at this point
    CIEXYZ::xyY<T> nextxyY(xyYfromCCT(CCT+1));

    // convert xyY to uv and get the du, dv at the locus
    CIEUCS1960::uv<T> locusuv(locusxyY);
    CIEUCS1960::uv<T> nextuv(nextxyY);
    T du = locusuv.u - nextuv.u;
    T dv = locusuv.v - nextuv.v;

    // calculate the sine and cosine of the angle
    T dh = sqrt( du * du + dv * dv );
    T sinT = dv / dh;
    T cosT = du / dh;

    CIEUCS1960::uv<T> targetuv(locusuv.u - Duv * sinT, locusuv.v + Duv * cosT);
    return targetuv.toxyY();
}

} // namespace Color
