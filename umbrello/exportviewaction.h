#if !defined(__EXPORTVIEWACTION_H__)
#define __EXPORTVIEWACTION_H__

#include <kaction.h>
class KActionCollection;
class UMLView;


class ExportViewAction : public KAction {
    Q_OBJECT
public:
    ExportViewAction(QString extension, QObject* parent=0);
    ExportViewAction(KActionCollection* parent);

public slots:
    void exportView(UMLView* view, QString imageMimeType);
    void exportAllViews();
protected:
    void updateMimetype();
    void askFormat();

private:
    QString m_extension;
    QString m_mimetype;

};
#endif        //  #if !defined(__EXPORTVIEWACTION_H__)

