#pragma once

#ifndef GEOMETRON_SCROLLINGWINDOW_HPP
#define GEOMETRON_SCROLLINGWINDOW_HPP

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScrollingWindow : public ListView {
    CCMenu* inner = nullptr;

    bool init(CCSize size);

    void setupList(float) override;
    TableViewCell* getListCell(char const* key) override;
public:
    void addInnerChild(auto&&... args) {
        return inner->addChild(std::forward<decltype(args)>(args)...);
    }
    void addInnerChildAtPosition(auto&&... args) {
        return inner->addChildAtPosition(std::forward<decltype(args)>(args)...);
    }
    unsigned int getInnerChildrenCount();
    CCArray* getInnerChildren();

    void resizeInner(float height);
    void scrollTo(float y);
    float innerHeight();
    float scrollHeight();

    static ScrollingWindow* create(CCSize size);
};

#endif //GEOMETRON_SCROLLINGWINDOW_HPP