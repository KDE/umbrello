

#include "umbrellodialog.h"
#include "dialogpage.h"

#include <qpoint.h>
#include <qvbox.h>
#include <kiconloader.h>


UmbrelloDialog::UmbrelloDialog( QWidget *parent, int dialogType, const char *name , bool modal, const QString &caption, int buttonMask):
		KDialogBase( dialogType, caption, buttonMask, Ok , parent, name, modal, true )
{
	if(!parent)
		connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

UmbrelloDialog::~UmbrelloDialog()
{
}

void UmbrelloDialog::addPage( QWidget *page, const QString &title, const QPixmap &pixmap )
{
	DialogPage *dpage = dynamic_cast<DialogPage*>(page);
	if (dpage)
	{
		dpage->setAutoApply(false);
	
		connect(this,SIGNAL(cancelClicked()),page,SLOT(cancel()));
		connect(this,SIGNAL(applyClicked()),page,SLOT(apply()));
		connect(this,SIGNAL(okClicked()),page,SLOT(apply()));
	}
	
	QWidget *box = KDialogBase::addVBoxPage(title,title, pixmap.isNull()? DesktopIcon( "misc"): pixmap );
	page->reparent(box,0,QPoint(0,0));
}

void UmbrelloDialog::setMainWidget( QWidget *page )
{
	DialogPage *dpage = dynamic_cast<DialogPage*>(page);
	if (dpage)
	{
		dpage->setAutoApply(false);
	
		connect(this,SIGNAL(cancelClicked()),page,SLOT(cancel()));
		connect(this,SIGNAL(applyClicked()),page,SLOT(apply()));
		connect(this,SIGNAL(okClicked()),page,SLOT(apply()));
	}
	KDialogBase::setMainWidget(page);
}

//#include "umbrellodialog.moc"

