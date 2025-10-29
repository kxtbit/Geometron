#pragma once

#ifndef GEOMETRON_ENHANCEDTEXTPANEL_HPP
#define GEOMETRON_ENHANCEDTEXTPANEL_HPP

#include <Geode/Geode.hpp>

#include "ScrollingWindow.hpp"

using namespace geode::prelude;

class EnhancedTextPanel : public CCNode {
    CCArrayExt<CCLabelBMFont*> labels = CCArray::create();
    std::vector<std::string> lines;
    //std::vector<CCLabelBMFont*> lineLabels;

    Ref<ScrollingWindow> window = nullptr;
    float lineHeight = 12;
    std::string fontName = "chatFont.fnt";
    float fontScale = 0.75f;

    Ref<CCBMFontConfiguration> fontData;

    bool init(ScrollingWindow* newWindow, float newLineHeight, std::string newFontName, float newFontScale);

    std::vector<size_t> wrapString(const std::string& str, float maxWidth);

    void setLabelPosition(CCLabelBMFont* label, CCPoint pos);
    void setLabelText(CCLabelBMFont* label, const std::string& text);

    void shiftUp(int count);

    float prevWindowY = 0;
    void resetLabels(int firstLine);
    void updateLabels(int firstLine, int lastLine);
    void update(float delta) override;

    void updateLine(int line);
public:
    void addLine(const std::string& line);
    std::string getLineFromBottom(int i);
    void setLine(int i, const std::string& line);
    int lineCount();

    float getLineHeight();
    std::string getFontName();
    float getFontScale();

    int firstVisibleLine();
    int lastVisibleLine();

    static EnhancedTextPanel* create(ScrollingWindow* window, float lineHeight = 12, std::string fontName = "chatFont.fnt", float fontScale = 0.75f);
};

#endif //GEOMETRON_ENHANCEDTEXTPANEL_HPP