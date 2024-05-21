#define _USE_MATH_DEFINES
#include "../header/geo.h"

#include <cmath>

namespace geo {

    bool operator==(const Coordinates& lhs, const Coordinates& rhs) {
        return (std::abs(lhs.lat - rhs.lat) < 1e-6 && std::abs(lhs.lng - rhs.lng) < 1e-6);
    }

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        const double dr = M_PI / 180.0;
        const double R = 6371000;
        double p1 = (from.lat * dr), p2 = (to.lat * dr);
        double dl = std::abs(from.lng - to.lng);
        double CosSigma = sin(p1) * sin(p2) + cos(p1) * cos(p2) * cos(dl * dr);
        return std::acos(CosSigma) * R;
    }

}  // namespace geo