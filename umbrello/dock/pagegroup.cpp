#include "pagegroup.h"

#include "pages.h"


PageGroup::PageGroup(QObject* parent) : QObject(parent)
{
    
}

PageGroup::~PageGroup()
{
    
}

const int& PageGroup::getIdentifier()
{
    return m_Identifier;
}

void PageGroup::addPage(DockPage* page)
{
    
}