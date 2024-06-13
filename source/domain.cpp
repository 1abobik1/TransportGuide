#include <string_view>
#include <unordered_set>

#include "../header/domain.h"

using namespace std;

namespace domain {

    bool operator==(const Stop& lhs, const Stop& rhs) {
        return (lhs.name == rhs.name && lhs.coordinate == rhs.coordinate);
    }

    bool operator==(const Route& lhs, const Route& rhs) {
        return (lhs.name == rhs.name);
    }

} // namespace domain