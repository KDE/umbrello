/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "idchangelog.h"

IDChangeLog::IDChangeLog() {}

IDChangeLog::IDChangeLog(const IDChangeLog& Other) {
	m_LogArray = Other.m_LogArray;
}

IDChangeLog::~IDChangeLog() {}

IDChangeLog& IDChangeLog::operator=(const IDChangeLog& Other) {
	m_LogArray = Other.m_LogArray;

	return *this;
}

bool IDChangeLog::operator==(const IDChangeLog& /*Other*/) {
	/*It needs to be Implemented*/
	return false;
}

int IDChangeLog::findNewID(int OldID) {
	uint count = m_LogArray.size();
	for(uint i = 0; i < count; i++) {
		if((m_LogArray.point(i)).y() ==  OldID) {
			return  (m_LogArray.point(i)).x();
		}
	}

	return -1;
}

IDChangeLog& IDChangeLog::operator+=(const IDChangeLog& Other) {
	//m_LogArray.putpoints(m_LogArray.size(), Other.m_LogArray.size(), Other)
	uint count = Other.m_LogArray.size();
	for(uint i = 0; i < count; i++) {
		addIDChange((Other.m_LogArray.point(i)).y(), (Other.m_LogArray.point(i)).x());
	}

	return *this;
}

void IDChangeLog::addIDChange(int OldID, int NewID) {
	uint pos;
	if(!findIDChange(OldID, NewID, pos)) {
		pos = m_LogArray.size();
		m_LogArray.resize(pos + 1);
		m_LogArray.setPoint(pos, NewID, OldID);
	} else {
		m_LogArray.setPoint(pos, NewID, OldID);
	}
}

int IDChangeLog::findOldID(int NewID) {
	uint count = m_LogArray.size();
	for(uint i = 0; i < count; i++) {
		if((m_LogArray.point(i)).x() ==  NewID) {
			return  (m_LogArray.point(i)).y();
		}
	}

	return -1;
}

bool IDChangeLog::findIDChange(int OldID, int NewID, uint& pos) {
	uint count = m_LogArray.size();
	for(uint i = 0; i < count; i++) {
		if(((m_LogArray.point(i)).y() ==  OldID) && ((m_LogArray.point(i)).x() ==  NewID)) {
			pos = i;
			return  true;
		}
	}

	return false;
}

void IDChangeLog::removeChangeByNewID(int OldID) {
	uint count = m_LogArray.size();
	for(uint i = 0; i < count; i++) {
		if((m_LogArray.point(i)).y() ==  OldID) {
			m_LogArray.setPoint(i, NullID, OldID);
		}
	}
}
