

#include "umbrellodialog.h"
#include "dialogpage.h"

#include <qpoint.h>
#include <qvbox.h>
#include <kiconloader.h>


UmbrelloDialog::UmbrelloDialog( QWidget *parent, const char *name , bool modal, const QString &caption):
		KDialogBase( IconList, caption, Ok | Apply | Cancel | Help, Ok , parent, name, modal, true )
{
	connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

UmbrelloDialog::~UmbrelloDialog()
{
}

void UmbrelloDialog::addPage( QWidget *page, const QString &title, const QPixmap &pixmap )
{
	//page->setAutoApply(false);
	connect(this,SIGNAL(cancelClicked()),page,SLOT(cancel()));
	connect(this,SIGNAL(applyClicked()),page,SLOT(apply()));
	connect(this,SIGNAL(okClicked()),page,SLOT(apply()));
	
	QWidget *box = KDialogBase::addVBoxPage(title,title, pixmap.isNull()? DesktopIcon( "misc"): pixmap );
	page->reparent(box,0,QPoint(0,0));
}

//#include "umbrellodialog.moc"

