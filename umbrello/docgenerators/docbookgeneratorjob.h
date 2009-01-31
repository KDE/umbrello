/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DOCBOOKGENERATORJOB_H
#define DOCBOOKGENERATORJOB_H

#include <QtCore/QThread>

/**
 * This class is used to generate docbook from the document.
 * It emits the name of the temporary file to which it as generated the docbook as output.
 * It runs in a separate thread.
 *
 * @short Generates DocBook from the Document
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class DocbookGeneratorJob : public QThread 
{
    Q_OBJECT

  public:
    DocbookGeneratorJob(QObject* parent );

  protected:
    void run();

  signals:
    void docbookGenerated(const QString& url);

};

#endif
