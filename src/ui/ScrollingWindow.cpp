// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppParameterMayBeConst
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppMemberFunctionMayBeConst
#include <Geode/Geode.hpp>

#include "ScrollingWindow.hpp"

using namespace geode::prelude;

bool ScrollingWindow::init(CCSize size) {
    inner = CCMenu::create();
    inner->setContentSize(size);

    if (!ListView::init(CCArray::createWithObject(inner), BoomListType::Default, size.width, size.height))
        return false;
    return true;
}
void ScrollingWindow::setupList(float) {
    if (m_entries->count() <= 0) return;
    m_tableView->reloadData();

    m_tableView->moveToTopWithOffset(m_itemSeparation);

    auto cell = static_cast<TableViewCell*>(m_tableView->m_contentLayer->getChildren()->objectAtIndex(0));
    cell->m_backgroundLayer->setVisible(false);
    cell->setAnchorPoint({0, 0});
    cell->ignoreAnchorPointForPosition(false);
    cell->setPosition(0, 0);
    m_tableView->m_contentLayer->setPosition({0, 0});
}
TableViewCell* ScrollingWindow::getListCell(char const* key) {
    return new TableViewCell(key, m_width, m_itemSeparation);
}


unsigned int ScrollingWindow::getInnerChildrenCount() {
    return inner->getChildrenCount();
}
CCArray* ScrollingWindow::getInnerChildren() {
    return inner->getChildren();
}

void ScrollingWindow::resizeInner(float height) {
    height = std::max(height, innerHeight());
    CCSize size = {getContentWidth(), height};
    inner->setContentSize(size);
    inner->getParent()->setContentSize(size);
    m_tableView->m_contentLayer->setContentSize(size);
}
void ScrollingWindow::scrollTo(float y) {
    m_tableView->m_contentLayer->setPositionY(-y);
}
float ScrollingWindow::innerHeight() {
    return m_tableView->m_contentLayer->getContentHeight();
}
float ScrollingWindow::scrollHeight() {
    return -m_tableView->m_contentLayer->getPositionY();
}

ScrollingWindow* ScrollingWindow::create(CCSize size) {
    auto ret = new ScrollingWindow();
    ret->m_itemSeparation = size.height;
    ret->m_cellOpacity = 0;
    ret->m_cellBorderColor = {0, 0, 0, 0};
    if (ret->init(size)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}
