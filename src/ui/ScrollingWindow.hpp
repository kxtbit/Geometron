#pragma once

#ifndef GEOMETRON_SCROLLINGWINDOW_HPP
#define GEOMETRON_SCROLLINGWINDOW_HPP

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScrollingWindow : public ScrollLayer {
    ScrollingWindow(CCSize const& size, bool scrollWheelEnabled, bool vertical);

public:
    void addInnerChild(auto&&... args) {
        return m_contentLayer->addChild(std::forward<decltype(args)>(args)...);
    }
    void addInnerChildAtPosition(auto&&... args) {
        return m_contentLayer->addChildAtPosition(std::forward<decltype(args)>(args)...);
    }
    unsigned int getInnerChildrenCount();
    CCArray* getInnerChildren();

    void resizeInner(float height);
    void scrollTo(float y);
    float innerHeight();
    float scrollHeight();

    static ScrollingWindow* create(CCSize const& size, bool scrollWheelEnabled = true, bool vertical = true);
};

#endif //GEOMETRON_SCROLLINGWINDOW_HPP