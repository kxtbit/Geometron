#include "FontUtils.hpp"

#include <string>

using namespace std::string_literals;

float getKerning(CCBMFontConfiguration* fontData, utf16char left, utf16char right) {
    unsigned int key = (left << 16) | (right & 0xffff);
    if (fontData->m_pKerningDictionary) {
        tCCKerningHashElement* element;
        HASH_FIND_INT(fontData->m_pKerningDictionary, &key, element);
        if (element) return static_cast<float>(element->amount) / CC_CONTENT_SCALE_FACTOR();
    }
    return 0;
}
ccBMFontDef* getCharDef(CCBMFontConfiguration* fontData, utf16char c) {
    //by the way i hate these hash tables they're so cursed
    unsigned int key = c;
    tCCFontDefHashElement* element;
    HASH_FIND_INT(fontData->m_pFontDefDictionary, &key, element);
    return element ? &element->fontDef : nullptr;
}
std::vector<size_t> wrapString(const std::string& str, float maxWidth, CCBMFontConfiguration* fontData) {
    //log::info("wrapping string \"{}\" to max width {}", escapeString(str), maxWidth);
    auto ccwstr = cc_utf8_to_utf16(str.c_str());
    if (ccwstr == nullptr) return {};
    static_assert(sizeof(char16_t) == sizeof(uint16_t), "char16_t length is not 2 bytes");
    utf16string wstr = reinterpret_cast<char16_t*>(ccwstr);
    CC_SAFE_DELETE_ARRAY(ccwstr);

    std::vector<size_t> wraps;
    float x = 0;

    utf16char cPrev = -1;
    for (size_t i = 0; i < wstr.size(); i++) {
        utf16char c = wstr.at(i);
        //log::info("processing char {} (\"{}\"), x = {}", c, static_cast<char>(c), x);
        if (c != ' ' && !fontData->m_pCharacterSet->contains(c)) continue;

        auto charDef = getCharDef(fontData, c);
        if (!charDef) continue;
        float advance = static_cast<float>(charDef->xAdvance) / CC_CONTENT_SCALE_FACTOR();
        float kerning = getKerning(fontData, cPrev, c);

        //log::info("advance = {}, kerning = {}, total displacement = {}", advance, kerning, advance + kerning);

        float displacement = advance + kerning;
        if (x + displacement > maxWidth) {
            //log::info("wrapping at {}", i);
            wraps.push_back(i);
            x = 0;
        }
        x += displacement;

        cPrev = c;
    }

    return wraps;
}

std::string escapeString(const std::string& str) {
    std::stringstream out(str);
    out.clear();

    for (char c : str) {
        if (std::isprint(static_cast<unsigned char>(c))) {
            out << c;
        } else switch (c) {
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\\': out << "\\\\"; break;
            default: out << "\\x"s << std::hex << static_cast<unsigned int>(static_cast<unsigned char>(c)) << std::dec;
        }
    }
    return out.str();
}