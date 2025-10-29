// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppMemberFunctionMayBeConst
#include <Geode/Geode.hpp>

#include "ScrollingWindow.hpp"

using namespace geode::prelude;

ScrollingWindow::ScrollingWindow(CCSize const& size, bool scrollWheelEnabled, bool vertical)
        : ScrollLayer({0, 0, size.width, size.height}, scrollWheelEnabled, vertical) {
}

unsigned int ScrollingWindow::getInnerChildrenCount() {
    return m_contentLayer->getChildrenCount();
}
CCArray* ScrollingWindow::getInnerChildren() {
    return m_contentLayer->getChildren();
}

void ScrollingWindow::resizeInner(float height) {
    height = std::max(height, innerHeight());
    CCSize size = {getContentWidth(), height};
    m_contentLayer->setContentSize(size);
}
void ScrollingWindow::scrollTo(float y) {
    m_contentLayer->setPositionY(-y);
}
float ScrollingWindow::innerHeight() {
    return m_contentLayer->getContentHeight();
}
float ScrollingWindow::scrollHeight() {
    return -m_contentLayer->getPositionY();
}

ScrollingWindow* ScrollingWindow::create(CCSize const& size, bool scrollWheelEnabled, bool vertical) {
    auto ret = new ScrollingWindow(size, scrollWheelEnabled, vertical);
    ret->autorelease();
    return ret;
}
