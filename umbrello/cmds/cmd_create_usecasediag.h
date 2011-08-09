/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_CREATE_USECASEDIAG_H
#define CMD_CREATE_USECASEDIAG_H

#include <QtGui/QUndoCommand>

//new canvas
#define SOC2011 1
namespace QGV {
  class UMLView;
}


class UMLDoc;
class UMLView;

namespace Uml
{
    class CmdCreateUseCaseDiag : public QUndoCommand
    {
        public:
            explicit CmdCreateUseCaseDiag(UMLDoc* doc, const QString& name = "");
            ~CmdCreateUseCaseDiag();

            void redo();
            void undo();

        private:
            UMLDoc*     m_pUMLDoc;
            UMLView*    m_pUMLView;
#ifdef SOC2011
	    QGV::UMLView* m_pUMLView_new;;
#endif
            QString     m_Name;
    };
}

#endif
