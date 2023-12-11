#include <cmath>
#include "SPD.h"


namespace Color
{

// Spectral Similarity Index
// Overview:
// https://www.oscars.org/science-technology/projects/spectral-similarity-index-ssi
// White paper:
// https://www.oscars.org/sites/oscars/files/ssi_overview_2020-09-16.pdf
// Calculator:
// http://oscars.org/ssi-calculator
// Standard (SMPTE ST 2122:2020):
// https://ieeexplore.ieee.org/document/9147084
template<typename T>
class SSI
{
public:
    SSI(const SPD<T>& reference)
        : m_referenceNormalized(normalize(reference))
    {
    }

    void setReference(const SPD<T>& reference)
    {
        m_referenceNormalized = normalize(reference);
    }

    T grade(const SPD<T>& _sample) const
    {
        // use normalized sample
        SPD<T> sample(normalize(_sample));

        // subtract sample from reference
        std::vector<T> diff;
        diff.reserve((sample.end() - sample.start()) / sample.delta() + 1);
        for (T wavelength = sample.start(); wavelength <= sample.end(); wavelength += sample.delta())
        {
            T ref(m_referenceNormalized.power(wavelength));
            diff.push_back((sample.power(wavelength) - ref) / (ref + 1.0/30));
        }

        // apply weights to difference
        std::vector<T> intro_weights{4.0/15, 22.0/45, 32.0/45, 8.0/9, 44.0/45};
        auto di(diff.begin());
        for(auto ii(intro_weights.begin()); ii != intro_weights.end(); ++di, ++ii)
        {
            *di *= *ii;
        }
        std::vector<T> outro_weights{11.0/15, 3.0/15};
        auto dri(diff.rbegin());
        for(auto ori(outro_weights.rbegin()); ori != outro_weights.rend(); ++dri, ++ori)
        {
            *dri *= *ori;
        }

        // convolute difference with smoothing kernel
        std::vector<T> kernel{0.22, 0.56, 0.22};
        std::vector<T> smooth(convolve(diff, kernel));

        // calculate vector magnitude
        T sum(0);
        for (auto value: smooth)
        {
            sum += value * value;
        }

        return std::round(100.0 - 32.0 * std::sqrt(sum));
    }

protected:
    // Returns \a spd sampled at 1nm, intergrated at 10nm and
    // normalized to unity power
    SPD<T> normalize(const SPD<T>& spd) const
    {
        // integrate at 10nm
        const auto range(std::make_pair<T, T>(380, 670));
        const T delta(10);
        std::vector<T> powers;
        powers.reserve((range.second - range.first) / delta + 1);
        T totalPower(0);
        for (auto wavelength = range.first; wavelength <= range.second; wavelength += delta)
        {
            // integrate by summing 1nm samples, which interpolates as needed
            T power(0);
            for (auto offset = -delta/2; offset < delta/2; offset += 1)
            {
                power += spd.power(wavelength+offset);
            }
            power /= delta;
            powers.push_back(power);
            totalPower += power;
        }

        // normalize to unity power
        for (auto& power : powers)
        {
            power /= totalPower;
        }

        return SPD<T>(range.first, delta, powers);
    }

    // convolve f with g
    // https://stackoverflow.com/questions/24518989/how-to-perform-1-dimensional-valid-convolution
    std::vector<T> convolve(const std::vector<T>& f, const std::vector<T>& g) const
    {
        int const nf = f.size();
        int const ng = g.size();
        int const n  = nf + ng - 1;
        std::vector<T> out(n, T());
        for(auto i(0); i < n; ++i)
        {
            int const jmn = (i >= ng - 1)? i - (ng - 1) : 0;
            int const jmx = (i <  nf - 1)? i            : nf - 1;
            for(auto j(jmn); j <= jmx; ++j) {
                out[i] += (f[j] * g[i - j]);
            }
        }
        return out;
    }

    // the normalized SPD
    SPD<T> m_referenceNormalized;
};

}
