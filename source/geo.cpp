#define _USE_MATH_DEFINES
#include "../header/geo.h"

#include <cmath>

namespace geo {

    bool operator==(const Coordinates& lhs, const Coordinates& rhs) {
        return (std::abs(lhs.lat - rhs.lat) < 1e-6 && std::abs(lhs.lng - rhs.lng) < 1e-6);
    }

    

}  // namespace geo