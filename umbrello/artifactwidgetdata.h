/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ARTIFACTWIDGETDATA_H
#define ARTIFACTWIDGETDATA_H

#include "umlwidgetdata.h"

/**
 * This class holds all the ArtifactWidget's Information. All this
 * information goes to a file or clipboard when an ConceptWidget
 * object is saved. With this class we are trying to achieve
 * isolation between data and display layers.
 *
 * @author Jonathan Riddell
 */
class ArtifactWidgetData : public UMLWidgetData {
public:

	/**
	 * creates a ArtifactWidgetData object
	 */
	ArtifactWidgetData(SettingsDlg::OptionState optionState);

	/**
	 * creates a copy of a ArtifactWidgetData object
	 */
	ArtifactWidgetData(ArtifactWidgetData& Other);

	/**
	 * destructor, empty
	 */
	virtual ~ArtifactWidgetData();

	/**
	 * Make the same as another ArtifactWidgetData
	 */
	virtual ArtifactWidgetData& operator=(ArtifactWidgetData& Other);

	/**
	 * Compare two ArtifactWidgetData.
	 */
	virtual bool operator==(ArtifactWidgetData& Other);

	/**
	 * Saves to the <artifactwidget> element
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads from a <artifactwidget> element
	 */
	bool loadFromXMI(QDomElement& qElement);

protected:

};

#endif
