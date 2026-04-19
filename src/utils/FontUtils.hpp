#pragma once

#ifndef GEOMETRON_FONTUTILS_HPP
#define GEOMETRON_FONTUTILS_HPP

#include <Geode/Geode.hpp>

using namespace geode::prelude;

using utf16char = char16_t;
using utf16string = std::basic_string<utf16char>;

float getKerning(CCBMFontConfiguration* fontData, utf16char left, utf16char right);
ccBMFontDef* getCharDef(CCBMFontConfiguration* fontData, utf16char c);

std::vector<size_t> wrapString(const std::string& str, float maxWidth, CCBMFontConfiguration* fontData);

std::string escapeString(const std::string& str);

#endif //GEOMETRON_FONTUTILS_HPP