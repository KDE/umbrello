#include "exportviewaction.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qstringlist.h>
#include <kinputdialog.h>
#include <kactioncollection.h>
#include <kurl.h>
#include <qfileinfo.h>
#include <qrect.h>
#include <qpicture.h>
#include <qpainter.h>
#include <kapplication.h>
#include <kconfig.h>
#include <qdir.h>


static QStringList supportedTypes;

ExportViewAction::ExportViewAction(KActionCollection* parent) :
    KAction(i18n("Export all Diagrams as Images"),
            SmallIconSet("image"),  0, 
            0, 0, parent, "view_export_all"), 
    m_extension(QString::null),
    m_mimetype("unknown") {
    connect(this, SIGNAL(activated()),
            this, SLOT(exportAllViews()));
        
}
    
 

ExportViewAction::ExportViewAction(QString extension, QObject *parent) :
    KAction(i18n("Export all Diagrams as Images"),
            SmallIconSet("image"), 0, parent),
    m_extension(extension) {
    m_mimetype = "unknown";
    updateMimetype();
    kdDebug() << "m_mimetype:  " << m_mimetype << endl;
    connect(this, SIGNAL(activated()),
            this, SLOT(exportAllViews()));

}

void ExportViewAction::updateMimetype() {
    m_mimetype = UMLView::imageTypeToMimeType(m_extension); 
}

void ExportViewAction::askFormat() {
    if (!supportedTypes.size()) {
        supportedTypes << "eps";
        supportedTypes << "png";
        supportedTypes << "svg";
    }
    m_extension = KInputDialog::getItem(text(),
       i18n("To extension:"), supportedTypes); 
    updateMimetype();       
}

void ExportViewAction::exportView(UMLView* view, QString imageMimetype) {
    UMLApp *app = UMLApp::app();

    KURL url = app->getDocument()->URL();
    QString extDef = UMLView::mimeTypeToImageType(imageMimetype).lower();
    QString file =  view->getName() + "." + extDef;

    QString dir = url.directory();
    QDir qdir(dir);
    QFileInfo finfo(qdir, file);
    file = finfo.filePath();
    
    QString ext = finfo.extension(false);

    QRect rect = view->getDiagramRect();
    if (rect.isEmpty()) {
        kdDebug() << "Can not save an empty diagram" << endl;
        return;
    }
    kdDebug() << "ExportImageTo: " << file << endl;
    if (imageMimetype == "image/x-eps") {
        view->printToFile(file,true);
    } else if (imageMimetype == "image/svg+xml") {
        QPicture* diagram = new QPicture();
        QPainter* painter = new QPainter();
        painter->begin( diagram );

        /* make sure the widget sizes will be according to the
         actually used printer font, important for getDiagramRect()
         and the actual painting */
        view->forceUpdateWidgetFontMetrics(painter);

        QRect rect = view->getDiagramRect();
        painter->translate(-rect.x(),-rect.y());
        view->getDiagram(rect,*painter);
        painter->end();
        diagram->save(file, UMLView::mimeTypeToImageType(imageMimetype).ascii());

        // delete painter and printer before we try to open and fix the file
        delete painter;
        delete diagram;
        // next painting will most probably be to a different device (i.e. the screen)
         view->forceUpdateWidgetFontMetrics(0);
    } else {
        QPixmap diagram(rect.width(), rect.height());
        view->getDiagram(rect, diagram);
        diagram.save(file, UMLView::mimeTypeToImageType(imageMimetype).ascii());
    }

}
void ExportViewAction::exportAllViews() {
    if (m_extension == QString::null)
        askFormat();
    UMLApp *app = UMLApp::app();
    kdDebug() << "Exporting All Views..." << endl;
    UMLViewList views = app->getDocument()->getViewIterator();
    for(UMLView *view = views.first(); view; view = views.next()) {
        exportView(view,m_mimetype);
    }
}


#include "exportviewaction.moc"

