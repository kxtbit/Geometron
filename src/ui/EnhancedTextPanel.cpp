// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppMemberFunctionMayBeConst
#include "EnhancedTextPanel.hpp"

#include <string>

#include "../engine/LuaEngine.hpp"
#include "../utils/FontUtils.hpp"

using namespace std::string_literals;

using utf16char = unsigned short;
using utf16string = std::basic_string<utf16char>;

bool EnhancedTextPanel::init(ScrollingWindow* newWindow, float newLineHeight, std::string newFontName, float newFontScale) {
    if (!CCNode::init()) return false;

    window = newWindow;
    lineHeight = newLineHeight;
    fontName = newFontName;
    fontScale = newFontScale;

    fontData = CCBMFontConfiguration::create(fontName.c_str());

    scheduleUpdate();
    return true;
}

void EnhancedTextPanel::setLabelPosition(CCLabelBMFont* label, CCPoint pos) {
    //log::info("{} y position {} -> {}", label->getID(), label->getPositionY(), pos.y);
    label->setPosition(pos);
}
void EnhancedTextPanel::setLabelText(CCLabelBMFont* label, const std::string& text) {
    auto escaped = escapeString(text);
    //log::info("{} text \"{}\" -> \"{}\"", label->getID(), label->getString(), escaped);
    label->setString(text.c_str());
}

void EnhancedTextPanel::shiftUp(int count) {
    if (labels.size() <= 0) return;
    if (std::abs(count) > labels.size()) return resetLabels(firstVisibleLine());

    int firstLine = std::max(0, firstVisibleLine());
    //log::info("firstLine is {}", firstLine);
    std::vector<CCLabelBMFont*> newLabels(labels.size());
    for (int i = 0; i < labels.size() - count; i++) {
        auto label = labels[i];
        setLabelPosition(label, {0, label->getPositionY() + lineHeight * count});
        newLabels[i + count] = label;
    }
    for (int i = 0; i < count; i++) {
        auto label = labels[labels.size() - count + i];
        int line = firstLine + i;
        setLabelPosition(label, {0, line * lineHeight});
        setLabelText(label, getLineFromBottom(line));
        newLabels[i] = label;
    }

    labels.inner()->removeAllObjects();
    for (int i = 0; i < newLabels.size(); i++) {
        labels.push_back(newLabels[i]);
    }
}

void EnhancedTextPanel::resetLabels(int firstLine) {
    if (labels.size() <= 0) return;

    //log::info("resetting labels");
    for (int i = 0; i < labels.size(); i++) {
        auto label = labels[i];
        int line = firstLine + i;
        setLabelText(label, getLineFromBottom(line));
        setLabelPosition(label, {0, line * lineHeight});
    }
}
void EnhancedTextPanel::updateLabels(int newFirstLine, int lastLine) {
    /*for (int i = firstLine; i < lastLine; i++) {
        int j = i - firstLine;
        auto label = labels[j];

        label->setString(getLine(i).c_str());
        label->setPosition(0, i * lineHeight);
    }*/
    if (labels.size() <= 0) return;

    newFirstLine = std::max(0, newFirstLine);
    int prevFirstLine = std::max(0, static_cast<int>(std::floorf(prevWindowY / lineHeight)));
    int prevLastLine = prevFirstLine + labels.size() - 1;
    int offset = newFirstLine - prevFirstLine;
    if (offset != 0) {
        if (std::abs(offset) > labels.size()) return resetLabels(firstVisibleLine());

        std::vector<CCLabelBMFont*> newLabels(labels.size());
        if (offset > 0) {
            for (int i = offset; i < labels.size(); i++) {
                newLabels[i - offset] = labels[i];
            }
            for (int i = 0; i < offset; i++) {
                auto label = labels[i];
                int line = prevLastLine + i + 1;
                setLabelText(label, getLineFromBottom(line));
                setLabelPosition(label, {0, line * lineHeight});
                newLabels[labels.size() - offset + i] = label;
            }
        } else {
            offset = -offset;
            for (int i = 0; i < labels.size() - offset; i++) {
                newLabels[i + offset] = labels[i];
            }
            for (int i = 0; i < offset; i++) {
                auto label = labels[labels.size() - offset + i];
                int line = prevFirstLine - offset + i;
                setLabelText(label, getLineFromBottom(line));
                setLabelPosition(label, {0, line * lineHeight});
                newLabels[i] = label;
            }
        }

        labels.inner()->removeAllObjects();
        for (int i = 0; i < newLabels.size(); i++) {
            labels.push_back(newLabels[i]);
        }
    }
}
void EnhancedTextPanel::update(float delta) {
    float windowHeight = window->getContentHeight();
    float windowY = window->scrollHeight();

    int lineCount = std::ceilf(windowHeight / lineHeight) + 1;
    int firstLine = firstVisibleLine();
    int lastLine = lastVisibleLine();

    if (lineCount > labels.size()) {
        //log::info("height increased -> {}", windowHeight / lineHeight);
        //log::info("number of labels {} -> {}", labels.size(), lineCount);
        //float windowWidth = window->getContentWidth();
        for (int j = labels.size(); j < lineCount; j++) {
            int i = j + firstLine;
            auto newLabel = CCLabelBMFont::create(getLineFromBottom(i).c_str(), fontName.c_str(), -1.0, kCCTextAlignmentLeft);
            newLabel->setScale(fontScale);
            newLabel->setAnchorPoint({0, 0});
            newLabel->ignoreAnchorPointForPosition(false);
            newLabel->setPosition(0, i * lineHeight);
            newLabel->setID(fmt::format("Label{}", j));
            //log::info("add label {}", newLabel->getID());
            addChild(newLabel);

            labels.push_back(newLabel);
        }
    } else if (lineCount < labels.size()) {
        for (int j = labels.size() - 1; j >= lineCount; j--) {
            //log::info("remove label {}", labels[j]->getID());
            labels[j]->removeFromParentAndCleanup(true);
            labels.inner()->removeObjectAtIndex(j, true);
        }
    }

    updateLabels(firstLine, lastLine);
    prevWindowY = windowY;
}

void EnhancedTextPanel::updateLine(int line) {
    //float windowY = window->scrollHeight();
    int firstLine = firstVisibleLine();
    int lastLine = lastVisibleLine();
    int fromBottom = lines.size() - 1 - line;
    if (fromBottom >= firstLine && fromBottom <= lastLine) {
        setLabelText(labels[fromBottom - firstLine], lines[line].c_str());
    }
}

void EnhancedTextPanel::addLine(const std::string& line) {
    //log::info("adding line {}", escapeString(line));
    auto wraps = wrapString(line, window->getContentWidth() / fontScale, fontData);

    int lineCount = 0;
    size_t start = 0;
    for (size_t wrap : wraps) {
        lines.push_back(line.substr(start, wrap - start));
        lineCount++;
        start = wrap;
    }
    lines.push_back(line.substr(start, line.size() - start));
    lineCount++;

    shiftUp(lineCount);
    //resetLabels(firstVisibleLine());
}
std::string EnhancedTextPanel::getLineFromBottom(int i) {
    int strIndex = lines.size() - 1 - i;
    return (strIndex < 0 || strIndex >= lines.size()) ? "" : lines[strIndex];
}
void EnhancedTextPanel::setLine(int i, const std::string& line) {
    //log::info("setting line {} to {}", i, escapeString(line));
    if (i < 0 || i >= lines.size()) return;

    if (i == lines.size() - 1) {
        auto wraps = wrapString(line, window->getContentWidth() / fontScale, fontData);
        if (wraps.size() < 1) goto nowrap;
        lines[i] = line.substr(0, wraps[0]);
        updateLine(i);

        int lineCount = 0;
        size_t start = wraps[0];
        for (int j = 1; j < wraps.size(); j++) {
            size_t wrap = wraps[j];
            lines.push_back(line.substr(start, wrap - start));
            lineCount++;
            start = wrap;
        }
        lines.push_back(line.substr(start, line.size() - start));
        lineCount++;

        shiftUp(lineCount);
        //resetLabels(firstVisibleLine());
    } else {
        nowrap:
        lines[i] = line;
        updateLine(i);
    }
}
int EnhancedTextPanel::lineCount() {
    return lines.size();
}

float EnhancedTextPanel::getLineHeight() {
    return lineHeight;
}
std::string EnhancedTextPanel::getFontName() {
    return fontName;
}
float EnhancedTextPanel::getFontScale() {
    return fontScale;
}

int EnhancedTextPanel::firstVisibleLine() {
    return std::floorf(window->scrollHeight() / lineHeight);
}
int EnhancedTextPanel::lastVisibleLine() {
    return firstVisibleLine() + labels.size() - 1;
}

EnhancedTextPanel* EnhancedTextPanel::create(ScrollingWindow* window, float lineHeight, std::string fontName, float fontScale) {
    auto ret = new EnhancedTextPanel();
    if (ret->init(window, lineHeight, fontName, fontScale)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
