/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006 David Nolden <david.nolden.kdevelop@art-master.de>
*/

#include "cachemanager.h"

#include "debug_utils.h"

DEBUG_REGISTER_DISABLED(CacheManager)

void CacheNode::access() const
{
    m_manager->access(this);
}

void CacheManager::remove(const CacheNode* node)
{
    m_set.erase(node);
}

void CacheManager::add(const CacheNode* node)
{
    m_set.insert(node);
}

CacheNode::CacheNode(Manager* manager) : m_manager(manager), m_value(manager->currentMax())   //initialize m_value with the current maximum, so the new node has a chance even in a cache full of high-rated nodes
{
    m_manager->add(this);
}

CacheNode::~CacheNode()
{
    m_manager->remove(this);
};

void CacheManager::restart(unsigned int normalizeby)
{
    m_currentFrame = 1;
    m_currentMax = 1;
    SetType oldSet = m_set;
    m_set = SetType();
    for (SetType::iterator it = oldSet.begin(); it != oldSet.end(); ++it) {
        unsigned int newValue = (*it)->value() / (normalizeby / 1000);
        if (newValue > m_currentMax) m_currentMax = newValue;
        (*it)->setValue(newValue); ///This way not all information is discarded
        m_set.insert(*it);
    }
}

void CacheManager::access(const CacheNode* node)
{
    static const unsigned int limit = (std::numeric_limits<unsigned int>::max() / 3)*2;
    m_set.erase(node);
    node->setValue(m_currentMax+1);
    m_set.insert(node);
    if (node->value() > m_currentMax)
        m_currentMax = node->value();
    if (node->value() > limit)
        restart(node->value());
}

void CacheManager::setMaxNodes (int maxNodes)
{
    m_maxNodes = maxNodes;
    increaseFrame();
}

void CacheManager::increaseFrame()
{
    QString dbg_src(QLatin1String("CacheManager"));
    m_currentFrame ++;
    if (m_set.size() > m_maxNodes) {
        DEBUG_N(dbg_src) << "Have " << m_set.size() << " nodes, maximum is " << m_maxNodes << ", erasing." << endl;
        int mustErase = m_set.size() - m_maxNodes;
        while (!m_set.empty() && mustErase != 0) {
            --mustErase;
            SetType::iterator it = m_set.begin();
            erase(*it);
        }
        DEBUG_N(dbg_src) << "Have " << m_set.size() << " nodes after erasing." << endl;
    }
}

void CacheManager::removeLowerHalf()
{
    int maxNodes = m_maxNodes;
    setMaxNodes(m_set.size() / 2);
    setMaxNodes(maxNodes);
}

void CacheManager::saveMemory()
{
    removeLowerHalf();
}
