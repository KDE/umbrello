/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DOCBOOK2XHTMLGENERATORJOB_H
#define DOCBOOK2XHTMLGENERATORJOB_H

#include <QtCore/QThread>

#include <kurl.h>

/**
 * This class is used to generate XHTML from Docbook.
 * It emits the name of the temporary file to which it as generated the XHtml as output.
 * It runs in a separate thread.
 *
 * @short Generates XHtml from Docbook
 * @author Sharan Rao
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class Docbook2XhtmlGeneratorJob : public QThread
{
    Q_OBJECT
  public:
    Docbook2XhtmlGeneratorJob(KUrl& docBookUrl,QObject* parent);

  protected:
     void run();

  private:
     KUrl m_docbookUrl;

  signals:
     void xhtmlGenerated(const QString&);
};

#endif
