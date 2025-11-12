#pragma once

#ifndef GEOMETRON_FIXEDSTRING_HPP
#define GEOMETRON_FIXEDSTRING_HPP

template<size_t L>
struct FixedString {
    static constexpr size_t len = L;

    char v[L + 1] = {};
    constexpr FixedString(const char (&s)[L + 1]) {
        std::copy_n(s, L + 1, v);
    }
    template<size_t LA, size_t LB>
    constexpr FixedString(FixedString<LA> a, FixedString<LB> b) {
        std::copy_n(a.v, LA, v);
        std::copy_n(b.v, LB + 1, v + LA);
    }
};
template<size_t L>
FixedString(const char(&s)[L]) -> FixedString<L - 1>;
template<size_t LA, size_t LB>
FixedString(FixedString<LA>, FixedString<LB>) -> FixedString<LA + LB>;

template<FixedString Str>
constexpr auto operator ""_f() {
    return Str;
}

#endif //GEOMETRON_FIXEDSTRING_HPP