 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 #ifndef UMBRELLO_UMLASSOCIATIONTOOL_H
 #define UMBRELLO_UMLASSOCIATIONTOOL_H
 
 #include "../../diagram/associationtool.h"


namespace Umbrello
{

class UMLAssociationTool : public AssociationTool
{
public:
	enum ToolType { Generalization, Composition, Aggregation,
	                Dependency, Association, UniDiAssociation  };
	
	UMLAssociationTool( DiagramView  *view);
	virtual ~UMLAssociationTool();
	
	virtual void activate();
	virtual void deactivate();
	void setType( ToolType );
protected:
	virtual void setCursor( );
	virtual void createPath( );
	
	ToolType m_type;

};




}

#endif //UMBRELLO_UMLASSOCIATIONTOOL_H
