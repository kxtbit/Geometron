#pragma once

#ifndef GEOMETRON_GPOINT_HPP
#define GEOMETRON_GPOINT_HPP

#include <Geode/cocos/cocoa/CCGeometry.h>

struct GPoint {
    double x;
    double y;

    constexpr GPoint(const double x, const double y) : x(x), y(y) {}
    explicit constexpr GPoint(const cocos2d::CCPoint& point) : x(point.x), y(point.y) {}

    double constexpr dot(const GPoint& other) const {
        return other.x * x + other.y * y;
    }
    double constexpr magnitude() const {
        return std::sqrt(dot(*this));
    }
    GPoint constexpr unit() const {
        double d = std::sqrt(dot(*this));
        return GPoint(x / d, y / d);
    }

    GPoint constexpr operator+(const GPoint& r) const {
        return {x + r.x, y + r.y};
    }
    GPoint constexpr operator-(const GPoint& r) const {
        return {x - r.x, y - r.y};
    }
    GPoint constexpr operator*(const GPoint& r) const {
        return {x * r.x, y * r.y};
    }
    GPoint constexpr operator*(double r) const {
        return {x * r, y * r};
    }
    GPoint constexpr operator/(const GPoint& r) const {
        return {x / r.x, y / r.y};
    }
    GPoint constexpr operator/(double r) const {
        return {x / r, y / r};
    }

    GPoint constexpr operator-() const {
        return {-x, -y};
    }

    explicit constexpr operator std::string() const {
        return fmt::format("point {{{}, {}}}", x, y);
    }
    explicit constexpr operator cocos2d::CCPoint() const {
        return {static_cast<float>(x), static_cast<float>(y)};
    }
};
constexpr GPoint operator*(double a, GPoint b) {
    return b * a;
}
constexpr GPoint operator/(double a, GPoint b) {
    return GPoint {a, a} / b;
}

namespace sol {
    template<>
    struct is_automagical<GPoint> : std::false_type {};
}

#endif //GEOMETRON_GPOINT_HPP