



#ifndef UMBRELLO_DIALOG
#define UMBRELLO_DIALOG

#include <kdialogbase.h>
#include <qpixmap.h> 

class DialogPage;

class UmbrelloDialog : public KDialogBase
{
public:
	UmbrelloDialog( QWidget *parent, const char *name = 0, bool modal = true, 
	                const QString &caption = "Umbrello UML Modeller");
	
	virtual ~UmbrelloDialog();
	
	virtual void addPage( QWidget *page, const QString &title, const QPixmap &pixmap = QPixmap() );
};

#endif
