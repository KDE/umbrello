#include <kdialog.h>
#include <klocale.h>
#include "rubycodegenerationformbase.h"

#include <qvariant.h>
/*
 *  Constructs a RubyCodeGenerationFormBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
RubyCodeGenerationFormBase::RubyCodeGenerationFormBase(QWidget* parent, const char* name, Qt::WFlags fl)
    : QWidget(parent, name, fl)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
RubyCodeGenerationFormBase::~RubyCodeGenerationFormBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void RubyCodeGenerationFormBase::languageChange()
{
    retranslateUi(this);
}

#include "rubycodegenerationformbase.moc"
