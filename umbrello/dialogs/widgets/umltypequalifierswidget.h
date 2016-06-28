#ifndef UMLTYPEQUALIFIERSWIDGET_H
#define UMLTYPEQUALIFIERSWIDGET_H

#include "ui_umltypequalifierswidget.h"
#include <QtWidgets>
#include <QMap>

class UMLClassifierListItem;

class UMLTypeQualifiersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UMLTypeQualifiersWidget(QWidget *parent = 0);

    void apply();
    void setUMLClassifierItem(UMLClassifierListItem *o);

private:

    Ui::UMLTypeQualifiersWidget *ui;
    UMLClassifierListItem *m_qualifier;

};

#endif // UMLTYPEQUALIFIERSWIDGET_H
