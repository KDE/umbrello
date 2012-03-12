/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlfiledialog.h"
#include "umlviewimageexportermodel.h"

// kde includes
#include <KMimeType>
#include <klocalizedstring.h>

static QStringList mime2KdeFilter(const QStringList &mimeTypes, QString *allExtensions = 0)
{
    const KUrl emptyUrl;
    QStringList kdeFilter;
    QStringList allExt;
    foreach( const QString& mimeType, mimeTypes ) {
        KMimeType::Ptr mime( KMimeType::mimeType(mimeType) );
        if (mime) {
            allExt += mime->patterns();
            kdeFilter.append(mime->patterns().join(QLatin1String(" ")) + QLatin1Char('|') +
            mime->comment(emptyUrl));
        }
        else if (mimeType == "image/x-dot") {
            allExt += "*.dot";
            kdeFilter.append(QLatin1String("*.dot|Dot Files"));
        }
    }
    if (allExtensions) {
        allExt.sort();
        *allExtensions = allExt.join(QLatin1String(" "));
    }
    return kdeFilter;
}


UMLFileDialog::UMLFileDialog(const KUrl &startDir, const QString &filter, QWidget *parent, QWidget *widget)
  : m_dialog(new KFileDialog(startDir, filter, parent, widget))
{
}

UMLFileDialog::~UMLFileDialog()
{
    delete m_dialog;
}

int UMLFileDialog::exec()
{
    return m_dialog->exec();
}

void UMLFileDialog::setCaption(const QString &caption)
{
    m_dialog->setCaption(caption);
}

void UMLFileDialog::setOperationMode(KFileDialog::OperationMode mode)
{
    m_dialog->setOperationMode(mode);
}

void UMLFileDialog::setUrl(const KUrl &url, bool clearforward)
{
    m_dialog->setUrl(url, clearforward);
}

KUrl UMLFileDialog::selectedUrl()
{
    return m_dialog->selectedUrl();
}

void UMLFileDialog::setSelection(const QString &name)
{
    m_dialog->setSelection(name);
}

void UMLFileDialog::setMimeFilter(const QStringList &types, const QString &defaultType)
{
    QString allExtensions;
    QStringList filters = mime2KdeFilter(types, &allExtensions);
    if (defaultType.isEmpty() && (types.count() > 1)) {
        filters.prepend(allExtensions + QLatin1Char('|') + i18n("All Supported Files"));
    }
    m_dialog->setFilter(filters.join(QLatin1String("\n")));
}

QString UMLFileDialog::currentMimeFilter()
{
    QString currentFilter = m_dialog->currentFilter();
    #ifdef Q_WS_WIN32
    // using native KFileDialog returns empty filter, so we need a workaround
    if (currentFilter.isEmpty()) {
        KUrl url = m_dialog->selectedUrl();
        QFileInfo fi(url.toLocalFile());
        return UMLViewImageExporterModel::imageTypeToMimeType(fi.suffix());
    }
    #endif
    return UMLViewImageExporterModel::imageTypeToMimeType(currentFilter.remove("*."));
}

#include "umlfiledialog.moc"
