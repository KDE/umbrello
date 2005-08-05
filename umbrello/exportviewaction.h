#if !defined(__EXPORTVIEWACTION_H__)
#define __EXPORTVIEWACTION_H__

#include "kaction.h"

class ExportViewAction : public KAction {
    Q_OBJECT
  public:
    ExportViewAction(QString extension);
    void exportAllViews();
    
  private:
    QString m_extension;
    QString m_mimetype;
    
};
#endif        //  #if !defined(__EXPORTVIEWACTION_H__)

