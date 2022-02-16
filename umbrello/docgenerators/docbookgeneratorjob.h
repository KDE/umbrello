/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2007-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DOCBOOKGENERATORJOB_H
#define DOCBOOKGENERATORJOB_H

#include <QThread>

/**
 * This class is used to generate docbook from the document.
 * It emits the name of the temporary file to which it as generated the docbook as output.
 * It runs in a separate thread.
 *
 * @short Generates DocBook from the Document
 * @author Sharan Rao
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class DocbookGeneratorJob : public QThread 
{
    Q_OBJECT

  public:
    explicit DocbookGeneratorJob(QObject* parent);

  protected:
    void run();

  signals:
    void docbookGenerated(const QString& url);

};

#endif
