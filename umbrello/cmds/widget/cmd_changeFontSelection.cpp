#include "cmd_changeFontSelection.h"

#include "uml.h"
// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"



#include <klocale.h>
namespace Uml
{
	cmdChangeFontSelection::cmdChangeFontSelection(UMLDoc * doc,UMLView *view,QFont fon)
	{
		
		UMLWidget * widget = view->getFirstMultiSelectedWidget();
		setText(i18n("Change Font")+ " : " + widget->getName());/*+ widget->getName()*/
		pDoc=doc;
		pView=view;
		newFont = fon;
		oldFont = widget -> getFont() ;
	}
	
	void cmdChangeFontSelection::undo()
	{
		pView -> selectionSetFont( oldFont );
		pDoc -> setModified(true);
	}
	
	void cmdChangeFontSelection::redo()
	{
		pView -> selectionSetFont( newFont );
		pDoc -> setModified(true);
	}

}
