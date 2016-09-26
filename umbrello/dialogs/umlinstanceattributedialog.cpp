
//local includes
#include "umlinstanceattributedialog.h"
#include "ui_umlinstanceattributedialog.h"
#include "classifier.h"
#include "uml.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLInstanceAttributeDialog::UMLInstanceAttributeDialog(QWidget *parent, UMLAttribute *pAttribute)
    : SinglePageDialogBase(parent),
    ui(new Ui::UMLInstanceAttributeDialog),
    m_pAttribute(pAttribute)
{
    setCaption(i18n("Object Attribute Properties"));
    ui->setupUi(mainWidget());
    connect(ui->nameLE, &QLineEdit::textChanged, this, &UMLInstanceAttributeDialog::slotNameChanged);
    connect(ui->valueLE, &QLineEdit::textChanged, this, &UMLInstanceAttributeDialog::slotNameChanged);
}

UMLInstanceAttributeDialog::~UMLInstanceAttributeDialog()
{
    delete ui;
}

bool UMLInstanceAttributeDialog::apply()
{
    QString name = ui->nameLE->text();
    QString value = ui->valueLE->text();
    if (name.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute name."),
                           i18n("Class Name Invalid"), 0);
        ui->nameLE->setText(m_pAttribute->name());
        return false;
    } else if(value.isEmpty()) {
        KMessageBox::error(this, i18n("You have entered an invalid attribute value."),
                           i18n("Value Invalid"), 0);
        return false;
    }
    UMLClassifier * pConcept = dynamic_cast<UMLClassifier *>(m_pAttribute->parent());
    UMLObject *o = pConcept->findChildObject(name);
        if (o && o != m_pAttribute) {
            KMessageBox::error(this, i18n("The attribute name you have chosen is already being used in this operation."),
                               i18n("Attribute Name Not Unique"), 0);
            ui->nameLE->setText(m_pAttribute->name());
            return false;
        }
        m_pAttribute->setName(name);
        m_pAttribute->setInitialValue(value);
        return true;
}

void UMLInstanceAttributeDialog::slotNameChanged(const QString &text)
{
    enableButtonOk(!text.isEmpty());
}
