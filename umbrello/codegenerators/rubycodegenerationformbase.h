#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>


#include <Qt3Support/Q3GroupBox>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QWidget>
#include <Qt3Support/Q3MimeSourceFactory>

class Ui_RubyCodeGenerationFormBase
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    Q3GroupBox *groupBox2;
    QGridLayout *gridLayout2;
    QGridLayout *gridLayout3;
    QCheckBox *m_generateConstructors;
    QCheckBox *m_generateAssocAccessors;
    QCheckBox *m_generateAttribAccessors;
    QGridLayout *gridLayout4;
    QComboBox *m_accessorScopeCB;
    QLabel *textLabel1_3;
    QGridLayout *gridLayout5;
    QLabel *textLabel1_3_2;
    QComboBox *m_assocFieldScopeCB;
    Q3GroupBox *groupBox1;
    QHBoxLayout *hboxLayout;
    QLabel *textLabel1;
    QComboBox *m_SelectCommentStyle;
    QLabel *textLabel1_2;

    void setupUi(QWidget *RubyCodeGenerationFormBase)
    {
    RubyCodeGenerationFormBase->setObjectName(QString::fromUtf8("RubyCodeGenerationFormBase"));
    RubyCodeGenerationFormBase->resize(QSize(462, 376).expandedTo(RubyCodeGenerationFormBase->minimumSizeHint()));
    gridLayout = new QGridLayout(RubyCodeGenerationFormBase);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(11);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout1 = new QGridLayout();
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(0);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    groupBox2 = new Q3GroupBox(RubyCodeGenerationFormBase);
    groupBox2->setObjectName(QString::fromUtf8("groupBox2"));
    QSizePolicy sizePolicy((QSizePolicy::Policy)5, (QSizePolicy::Policy)4);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(groupBox2->sizePolicy().hasHeightForWidth());
    groupBox2->setSizePolicy(sizePolicy);
    groupBox2->setColumnLayout(0, Qt::Vertical);
    groupBox2->layout()->setSpacing(6);
    groupBox2->layout()->setMargin(11);
    gridLayout2 = new QGridLayout(groupBox2->layout());
    gridLayout2->setAlignment(Qt::AlignTop);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    gridLayout3 = new QGridLayout();
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(0);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    m_generateConstructors = new QCheckBox(groupBox2);
    m_generateConstructors->setObjectName(QString::fromUtf8("m_generateConstructors"));
    m_generateConstructors->setChecked(false);

    gridLayout3->addWidget(m_generateConstructors, 0, 0, 1, 1);

    m_generateAssocAccessors = new QCheckBox(groupBox2);
    m_generateAssocAccessors->setObjectName(QString::fromUtf8("m_generateAssocAccessors"));
    m_generateAssocAccessors->setChecked(true);

    gridLayout3->addWidget(m_generateAssocAccessors, 2, 0, 1, 1);

    m_generateAttribAccessors = new QCheckBox(groupBox2);
    m_generateAttribAccessors->setObjectName(QString::fromUtf8("m_generateAttribAccessors"));
    m_generateAttribAccessors->setChecked(true);

    gridLayout3->addWidget(m_generateAttribAccessors, 1, 0, 1, 1);


    gridLayout2->addLayout(gridLayout3, 0, 0, 1, 1);

    gridLayout4 = new QGridLayout();
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(0);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    m_accessorScopeCB = new QComboBox(groupBox2);
    m_accessorScopeCB->setObjectName(QString::fromUtf8("m_accessorScopeCB"));
    m_accessorScopeCB->setCurrentIndex(3);

    gridLayout4->addWidget(m_accessorScopeCB, 0, 1, 1, 1);

    textLabel1_3 = new QLabel(groupBox2);
    textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));

    gridLayout4->addWidget(textLabel1_3, 0, 0, 1, 1);


    gridLayout2->addLayout(gridLayout4, 1, 0, 1, 1);

    gridLayout5 = new QGridLayout();
    gridLayout5->setSpacing(6);
    gridLayout5->setMargin(0);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    textLabel1_3_2 = new QLabel(groupBox2);
    textLabel1_3_2->setObjectName(QString::fromUtf8("textLabel1_3_2"));

    gridLayout5->addWidget(textLabel1_3_2, 0, 0, 1, 1);

    m_assocFieldScopeCB = new QComboBox(groupBox2);
    m_assocFieldScopeCB->setObjectName(QString::fromUtf8("m_assocFieldScopeCB"));
    m_assocFieldScopeCB->setCurrentIndex(3);

    gridLayout5->addWidget(m_assocFieldScopeCB, 0, 1, 1, 1);


    gridLayout2->addLayout(gridLayout5, 2, 0, 1, 1);


    gridLayout1->addWidget(groupBox2, 2, 0, 1, 1);

    groupBox1 = new Q3GroupBox(RubyCodeGenerationFormBase);
    groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
    QSizePolicy sizePolicy1((QSizePolicy::Policy)5, (QSizePolicy::Policy)4);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(groupBox1->sizePolicy().hasHeightForWidth());
    groupBox1->setSizePolicy(sizePolicy1);
    groupBox1->setColumnLayout(0, Qt::Vertical);
    groupBox1->layout()->setSpacing(6);
    groupBox1->layout()->setMargin(11);
    hboxLayout = new QHBoxLayout(groupBox1->layout());
    hboxLayout->setAlignment(Qt::AlignTop);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    textLabel1 = new QLabel(groupBox1);
    textLabel1->setObjectName(QString::fromUtf8("textLabel1"));

    hboxLayout->addWidget(textLabel1);

    m_SelectCommentStyle = new QComboBox(groupBox1);
    m_SelectCommentStyle->setObjectName(QString::fromUtf8("m_SelectCommentStyle"));

    hboxLayout->addWidget(m_SelectCommentStyle);


    gridLayout1->addWidget(groupBox1, 1, 0, 1, 1);

    textLabel1_2 = new QLabel(RubyCodeGenerationFormBase);
    textLabel1_2->setObjectName(QString::fromUtf8("textLabel1_2"));
    QSizePolicy sizePolicy2((QSizePolicy::Policy)5, (QSizePolicy::Policy)4);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(textLabel1_2->sizePolicy().hasHeightForWidth());
    textLabel1_2->setSizePolicy(sizePolicy2);

    gridLayout1->addWidget(textLabel1_2, 0, 0, 1, 1);


    gridLayout->addLayout(gridLayout1, 0, 0, 1, 1);

    retranslateUi(RubyCodeGenerationFormBase);

    QMetaObject::connectSlotsByName(RubyCodeGenerationFormBase);
    } // setupUi

    void retranslateUi(QWidget *RubyCodeGenerationFormBase)
    {
    groupBox2->setTitle(QApplication::translate("RubyCodeGenerationFormBase", "Auto-Generate Methods", 0, QApplication::UnicodeUTF8));
    m_generateConstructors->setText(QApplication::translate("RubyCodeGenerationFormBase", "Empty constructor methods", 0, QApplication::UnicodeUTF8));
    m_generateAssocAccessors->setText(QApplication::translate("RubyCodeGenerationFormBase", "Association accessor methods", 0, QApplication::UnicodeUTF8));
    m_generateAttribAccessors->setText(QApplication::translate("RubyCodeGenerationFormBase", "Attribute accessor methods", 0, QApplication::UnicodeUTF8));
    m_accessorScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Public", 0, QApplication::UnicodeUTF8));
    m_accessorScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Private", 0, QApplication::UnicodeUTF8));
    m_accessorScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Protected", 0, QApplication::UnicodeUTF8));
    m_accessorScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "From Parent Object", 0, QApplication::UnicodeUTF8));
    textLabel1_3->setText(QApplication::translate("RubyCodeGenerationFormBase", "Default attribute accessor scope:", 0, QApplication::UnicodeUTF8));
    textLabel1_3_2->setText(QApplication::translate("RubyCodeGenerationFormBase", "Default association field scope:", 0, QApplication::UnicodeUTF8));
    m_assocFieldScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Public", 0, QApplication::UnicodeUTF8));
    m_assocFieldScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Private", 0, QApplication::UnicodeUTF8));
    m_assocFieldScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Protected", 0, QApplication::UnicodeUTF8));
    m_assocFieldScopeCB->addItem(QApplication::translate("RubyCodeGenerationFormBase", "From Parent Role", 0, QApplication::UnicodeUTF8));
    groupBox1->setTitle(QApplication::translate("RubyCodeGenerationFormBase", "Documentation", 0, QApplication::UnicodeUTF8));
    textLabel1->setText(QApplication::translate("RubyCodeGenerationFormBase", "Style:", 0, QApplication::UnicodeUTF8));
    m_SelectCommentStyle->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Begin-End (=begin  =end)", 0, QApplication::UnicodeUTF8));
    m_SelectCommentStyle->addItem(QApplication::translate("RubyCodeGenerationFormBase", "Hash (#)", 0, QApplication::UnicodeUTF8));
    textLabel1_2->setText(QApplication::translate("RubyCodeGenerationFormBase", "<p align=\"center\">Ruby Code Generation</p>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(RubyCodeGenerationFormBase);
    } // retranslateUi

};

namespace Ui {
    class RubyCodeGenerationFormBase: public Ui_RubyCodeGenerationFormBase {};
} // namespace Ui

class RubyCodeGenerationFormBase : public QWidget, public Ui::RubyCodeGenerationFormBase
{
    Q_OBJECT

public:
    RubyCodeGenerationFormBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~RubyCodeGenerationFormBase();

protected slots:
    virtual void languageChange();

};

#endif // FORM1_H
