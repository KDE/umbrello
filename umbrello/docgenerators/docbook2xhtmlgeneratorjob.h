/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2007-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DOCBOOK2XHTMLGENERATORJOB_H
#define DOCBOOK2XHTMLGENERATORJOB_H

#include <QThread>

#if QT_VERSION >= 0x050000
#include <QUrl>
#else
#include <KUrl>
#endif

/**
 * This class is used to generate XHTML from Docbook.
 * It emits the name of the temporary file to which it as generated the XHtml as output.
 * It runs in a separate thread.
 *
 * @short Generates XHtml from Docbook
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class Docbook2XhtmlGeneratorJob : public QThread
{
    Q_OBJECT
  public:
#if QT_VERSION >= 0x050000
    Docbook2XhtmlGeneratorJob(QUrl& docBookUrl, QObject* parent);
#else
    Docbook2XhtmlGeneratorJob(KUrl& docBookUrl, QObject* parent);
#endif
  protected:
     void run();

  private:
#if QT_VERSION >= 0x050000
     QUrl m_docbookUrl;
#else
     KUrl m_docbookUrl;
#endif

  signals:
     void xhtmlGenerated(const QString&);
};

#endif
