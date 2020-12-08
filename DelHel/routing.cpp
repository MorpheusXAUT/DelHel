#include "pch.h"

#include "routing.h"

std::ostream& operator<<(std::ostream& os, const routing& rt)
{
    os << "Routing(" << rt.adep;
    if (rt.adest.size() > 0) {
        os << "->" << rt.adest << ",";
    }
    else {
        os << ",";
    }

    os << rt.minlvl << "<RFL<" << rt.maxlvl << "): ";

    for (int i = 0; i < rt.waypts.size(); i++) {
        os << rt.waypts[i];
        if (i < rt.waypts.size() - 1) {
            os << ";";
        }
    }

    return os;
}
