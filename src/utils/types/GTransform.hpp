// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#pragma once

#ifndef GEOMETRON_GTRANSFORM_HPP
#define GEOMETRON_GTRANSFORM_HPP

#include <Geode/cocos/cocoa/CCAffineTransform.h>

#include "GPoint.hpp"

//essentially a 3x3 matrix where the last row is always [0, 0, 1]
struct GTransform {
    double xx, xy, xc;
    double yx, yy, yc;

    constexpr GTransform(const double xx, const double xy, const double xc, const double yx, const double yy, const double yc)
        : xx(xx), xy(xy), xc(xc), yx(yx), yy(yy), yc(yc) {}
    explicit constexpr GTransform(const cocos2d::CCAffineTransform& t)
        : xx(t.a), xy(t.c), xc(t.tx), yx(t.b), yy(t.d), yc(t.ty) {}

    static constexpr GTransform identity() {
        return {1, 0, 0, 0, 1, 0};
    }
    static constexpr GTransform translation(double x, double y) {
        return {
            1, 0, x,
            0, 1, y,
        };
    }
    static constexpr GTransform translation(const GPoint& point) {return translation(point.x, point.y);}
    static constexpr GTransform scaling(double x, double y) {
        return {
            x, 0, 0,
            0, y, 0,
        };
    }
    static constexpr GTransform scaling(const GPoint& point) {return scaling(point.x, point.y);}
    static constexpr GTransform shear(double x, double y) {
        return {
            1, x, 0,
            y, 1, 0,
        };
    }
    static constexpr GTransform shear(const GPoint& point) {return shear(point.x, point.y);}
    static constexpr GTransform rotation(double rad) {
        double cos = std::cos(rad);
        double sin = std::sin(rad);
        return {
            cos, -sin, 0,
            sin, cos, 0,
        };
    }
    static constexpr GTransform dualRotation(double radX, double radY) {
        return {
            std::cos(radX), -std::sin(radY), 0,
            std::sin(radX), std::cos(radY), 0,
        };
    }
    static constexpr GTransform fromVectors(const GPoint& vecX, const GPoint& vecY) {
        return {
            vecX.x, vecY.x, 0,
            vecX.y, vecY.y, 0,
        };
    }
    static GTransform ofObject(GameObject* object) {
        constexpr double PI = std::numbers::pi;
        constexpr double TO_RAD = PI / 180.0;

        //return scale(object->m_scaleX, object->m_scaleY)
        //    .chain(dualRotation(object->getRotationY() * TO_DEG, object->getRotationX() * TO_DEG))
        //    .chain(translation(object->getPositionX(), object->getPositionY()));

        //this should be equivalent to the above but a bit more efficient
        double posX = object->getPositionX(), posY = object->getPositionY() - 90.0;
        double scaleX = object->m_scaleX, scaleY = object->m_scaleY;
        double radX = -object->getRotationY() * TO_RAD, radY = -object->getRotationX() * TO_RAD;
        return {
            scaleX * std::cos(radX), scaleY * -std::sin(radY), posX,
            scaleX * std::sin(radX), scaleY * std::cos(radY), posY,
        };
    }

    GPoint constexpr pos() const {
        return {xc, yc};
    }
    GPoint constexpr scale() const {
        return {GPoint {xx, yx}.magnitude(), GPoint {xy, yy}.magnitude()};
    }
    GPoint constexpr rotXY() const {
        return {std::atan2(yx, xx), std::atan2(yy, xy) - (std::numbers::pi * 0.5)};
    }
    GPoint constexpr degXY() const {
        return rotXY() * (180.0 / std::numbers::pi);
    }

    GTransform constexpr inverse() const {
        double detMain = xx*yy - xy*yx;
        double detAuxX = yx*yc - xc*yy;
        double detAuxY = xy*xc - yc*xx;
        double mul = 1.0 / detMain;
        return {
            mul * yy, mul * -xy, mul * detAuxX,
            mul * -yx, mul * xx, mul * detAuxY,
        };
    }

    //apply the transform to a point
    GPoint constexpr apply(const GPoint& point) const {
        return {
            xx * point.x + xy * point.y + xc,
            yx * point.x + yy * point.y + yc,
        };
    }
    //apply the transform to a vector (ignores translation)
    GPoint constexpr applyVector(const GPoint& point) const {
        return {
            xx * point.x + xy * point.y,
            yx * point.x + yy * point.y,
        };
    }
    //apply the transform to another transform (matrix multiply)
    GTransform constexpr apply(const GTransform& o) const {
        return {
            (xx * o.xx + xy * o.yx + xc * 0), (xx * o.xy + xy * o.yy + xc * 0), (xx * o.xc + xy * o.yc + xc * 1),
            (yx * o.xx + yy * o.yx + yc * 0), (yx * o.xy + yy * o.yy + yc * 0), (yx * o.xc + yy * o.yc + yc * 1),
        };
    }
    //apply another transform to this transform (perform this transformation first)
    GTransform constexpr chain(const GTransform& o) const {
        return o.apply(*this);
    }
    //return a copy of this transform with o as the origin instead of the identity transform
    GTransform constexpr around(const GTransform& o) const {
        return o.inverse().chain(*this).chain(o);
    }
    GTransform constexpr around(const GPoint& point) const {
        auto o = translation(point);
        return o.inverse().chain(*this).chain(o);
    }

    GTransform constexpr operator+(const GPoint& other) const {
        return {
            xx, xy, xc + other.x,
            yx, yy, yc + other.y,
        };
    }
    GTransform constexpr operator-(const GPoint& other) const {
        return {
            xx, xy, xc - other.x,
            yx, yy, yc - other.y,
        };
    }

    bool constexpr operator==(const GTransform& r) const {
        return xx == r.xx && xy == r.xy && xc == r.xc
            && yx == r.yx && yy == r.yy && yc == r.yc;
    }

    explicit constexpr operator std::string() const {
        auto s = scale(), r = degXY();
        return fmt::format("transform loc {{{}, {}}} scale {{{}, {}}} rot {{{}, {}}} / mat {{{}, {}, {}, {}, {}, {}}}",
            xc, yc,
            s.x, s.y,
            r.x, r.y,
            xx, xy, xc,
            yx, yy, yc);
    }
    explicit constexpr operator cocos2d::CCAffineTransform() const {
        return CCAffineTransformMake(xx, xy, yx, yy, xc, yc);
    }
};

namespace sol {
    template<>
    struct is_automagical<GTransform> : std::false_type {};
}

#endif //GEOMETRON_GTRANSFORM_HPP