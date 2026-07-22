#pragma once
#include <string>
#include <iomanip>
#include <sstream>

namespace Format {

    inline std::string money(double v) {
        std::ostringstream ss;
        ss << "R" << std::fixed << std::setprecision(2) << v;
        return ss.str();
    }

    inline std::string meters(double v) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v << " m";
        return ss.str();
    }

    inline std::string area(double v) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << v << " m²";
        return ss.str();
    }

    inline std::string qty(double v) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << v;
        return ss.str();
    }

    inline std::string number(double v, int precision = 2) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    }
}
