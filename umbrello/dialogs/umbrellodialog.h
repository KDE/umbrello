



#ifndef UMBRELLO_DIALOG
#define UMBRELLO_DIALOG

#include <kdialogbase.h>
#include <qpixmap.h> 
#include <qstring.h>

class DialogPage;

class UmbrelloDialog : public KDialogBase
{
public:
	UmbrelloDialog( QWidget *parent = 0L, int dialogType = IconList, const char *name = 0, bool modal = true, 
	                const QString &caption = "Umbrello UML Modeller", int buttonMask = Ok | Apply | Cancel);
	
	virtual ~UmbrelloDialog();
	
	virtual void addPage( QWidget *page, const QString &title, const QPixmap &pixmap = QPixmap() );
	
	void setMainWidget( QWidget *page );
};

#endif
