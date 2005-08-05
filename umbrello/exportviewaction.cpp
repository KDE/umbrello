#include "exportviewaction.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include <kdebug.h>

ExportViewAction::ExportViewAction(QString extension)
  : m_extension(extension) {
    UMLApp *app = UMLApp::app();
    if (extension == "eps") {
        m_mimetype = "image/x-eps";
    }
    else if (extension == "png") {
        m_mimetype = "image/png";
    }
    else if (extension == "svg") {
        m_mimetype = "image/svg+xml";
    }
    kdDebug() << "m_mimetype:  " << m_mimetype;
}

void ExportViewAction::exportAllViews() {
    UMLApp *app = UMLApp::app();
    kdDebug() << "Exporting All Views..." << endl;
    UMLViewList views = app->getDocument()->getViewIterator();
    for(UMLView *view = views.first(); view; view = views.next()) {
        view->exportImageTo(m_mimetype);
    }
}


#include "exportviewaction.moc"

