/* This file is part of the KDE project
   Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "urlutil.h"

#include <qstringlist.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <kdebug.h>

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#include <kdeversion.h>
#if (KDE_VERSION_MINOR==0) && (KDE_VERSION_MAJOR==3)
#include <kdevkurl.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Namespace URLUtil
///////////////////////////////////////////////////////////////////////////////

QString URLUtil::filename(const QString & name) {
  int slashPos = name.lastIndexOf('/');
  return slashPos<0 ? name : name.mid(slashPos+1);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::directory(const QString & name) {
  int slashPos = name.lastIndexOf('/');
  return slashPos<0 ? QString("") : name.left(slashPos);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::relativePath(const KUrl & parent, const KUrl & child, uint slashPolicy) {
  bool slashPrefix = slashPolicy & SLASH_PREFIX;
  bool slashSuffix = slashPolicy & SLASH_SUFFIX;
  if (parent == child)
    return slashPrefix ? QString("/") : QString("");

  if (!parent.isParentOf(child)) return QString();
  return child.path( slashSuffix ? KUrl::RemoveTrailingSlash:KUrl::AddTrailingSlash).mid(parent.path(slashSuffix ? KUrl::AddTrailingSlash:KUrl::RemoveTrailingSlash).length());
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::relativePath(const QString & parent, const QString & child, uint slashPolicy) {
  return relativePath(KUrl(parent), KUrl(child), slashPolicy);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::upDir(const QString & path, bool slashSuffix) {
  int slashPos = path.lastIndexOf('/');
  if (slashPos<1) return QString();
  return path.mid(0,slashPos+ (slashSuffix ? 1 : 0) );
}

///////////////////////////////////////////////////////////////////////////////

KUrl URLUtil::mergeURL(const KUrl & source, const KUrl & dest, const KUrl & child) {

  // if already a child of source, then fine
  if (source.isParentOf(child) || source == child) return child;

  // if not a child of dest, return blank URL (error)
  if (!dest.isParentOf(child) && dest != child) return KUrl();

  // if child is same as dest, return source
  if (dest == child) return source;

  // calculate
  QString childUrlStr = child.url(KUrl::RemoveTrailingSlash);
  QString destStemStr = dest.url(KUrl::AddTrailingSlash );
  QString sourceStemStr = source.url(KUrl::AddTrailingSlash);
  return KUrl(sourceStemStr.append( childUrlStr.mid( destStemStr.length() ) ) );

}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::getExtension(const QString & path) {
  int dotPos = path.lastIndexOf('.');
  if (dotPos<0) return QString();
  return path.mid(dotPos+1);
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameRelative(const KUrl &baseDirUrl, const KUrl &url )
{
  QString absBase = extractPathNameAbsolute( baseDirUrl ),
    absRef = extractPathNameAbsolute( url );
  int i = absRef.indexOf( absBase, 0, Qt::CaseSensitive );

  if (i == -1)
    return QString();

  if (absRef == absBase)
    return QString( "." );
  else
    return absRef.replace( 0, absBase.length(), QString() );
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameRelative(const QString &basePath, const KUrl &url )
{
#if (KDE_VERSION_MINOR!=0) || (KDE_VERSION_MAJOR!=3)
  KUrl baseDirUrl = KUrl::fromPathOrUrl( basePath );
#else
  KUrl baseDirUrl = KdevKUrl::fromPathOrUrl( basePath );
#endif
  return extractPathNameRelative( baseDirUrl, url );
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameRelative(const QString &basePath, const QString &absFilePath )
{
#if (KDE_VERSION_MINOR!=0) || (KDE_VERSION_MAJOR!=3)
  KUrl baseDirUrl = KUrl::fromPathOrUrl( basePath ),
       fileUrl = KUrl::fromPathOrUrl( absFilePath );
#else
  KUrl baseDirUrl = KdevKUrl::fromPathOrUrl( basePath ),
       fileUrl = KdevKUrl::fromPathOrUrl( absFilePath );
#endif
  return extractPathNameRelative( baseDirUrl, fileUrl );
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::extractPathNameAbsolute( const KUrl &url )
{
  if (isDirectory( url ))
    return url.path( KUrl::AddTrailingSlash ); // with trailing "/" if none is present
  else
  {
    // Ok, this is an over-tight pre-condition on "url" since I hope nobody will never
    // stress this function with absurd cases ... but who knows?
  /*
    QString path = url.path();
    QFileInfo fi( path );  // Argh: QFileInfo is back ;))
    return ( fi.exists()? path : QString() );
  */
    return url.path();
  }
}

///////////////////////////////////////////////////////////////////////////////

bool URLUtil::isDirectory( const KUrl &url )
{
  return isDirectory( url.path() );
}

///////////////////////////////////////////////////////////////////////////////

bool URLUtil::isDirectory( const QString &absFilePath )
{
  return QDir( absFilePath ).exists();
}

///////////////////////////////////////////////////////////////////////////////

void URLUtil::dump( const KUrl::List &urls, const QString &aMessage )
{
  if (!aMessage.isNull())
  {
    kDebug(9000) << aMessage;
  }
  kDebug(9000) << " List has " << urls.count() << " elements.";

  for (size_t i = 0; i<urls.count(); ++i)
  {
    KUrl url = urls[ i ];
//    kDebug(9000) << " * Element = "  << url.path();
  }
}

///////////////////////////////////////////////////////////////////////////////

QStringList URLUtil::toRelativePaths( const QString &baseDir, const KUrl::List &urls)
{
  QStringList paths;

  for (size_t i=0; i<urls.count(); ++i)
  {
    paths << extractPathNameRelative( baseDir, urls[i] );
  }

  return paths;
}

///////////////////////////////////////////////////////////////////////////////

QString URLUtil::relativePathToFile( const QString & dirUrl, const QString & fileUrl )
{
  if (dirUrl.isEmpty() || (dirUrl == "/"))
    return fileUrl;

  QStringList dir = dirUrl.split('/', QString::SkipEmptyParts);
  QStringList file = fileUrl.split('/', QString::SkipEmptyParts);

  QString resFileName = file.last();
  file.removeLast();

  uint i = 0;
  while ( (i < dir.count()) && (i < (file.count())) && (dir[i] == file[i]) )
    i++;

  QString result_up;
  QString result_down;
  QString currDir;
  QString currFile;
  do
  {
    i >= dir.count() ? currDir = "" : currDir = dir[i];
    i >= file.count() ? currFile = "" : currFile = file[i];
    qDebug("i = %d, currDir = %s, currFile = %s", i, qPrintable(currDir), qPrintable(currFile));
    if (currDir.isEmpty() && currFile.isEmpty())
      break;
    else if (currDir.isEmpty())
      result_down += file[i] + '/';
    else if (currFile.isEmpty())
      result_up += "../";
    else
    {
      result_down += file[i] + '/';
      result_up += "../";
    }
    i++;
  }
  while ( (!currDir.isEmpty()) || (!currFile.isEmpty()) );

  return result_up + result_down + resFileName;
}

///////////////////////////////////////////////////////////////////////////////

// code from qt-3.1.2 version of QDir::canonicalPath()
QString URLUtil::canonicalPath( const QString & path )
{
    QString r;
    char cur[PATH_MAX+1];
    if ( ::getcwd( cur, PATH_MAX ) )
    {
        char tmp[PATH_MAX+1];
        if( ::realpath( QFile::encodeName( path ), tmp ) )
        {
            r = QFile::decodeName( tmp );
        }
        //always make sure we go back to the current dir
        ::chdir( cur );
    }
    return r;
}

///////////////////////////////////////////////////////////////////////////////

//written by "Dawit A." <adawit@kde.org>
//borrowed from his patch to KShell
QString URLUtil::envExpand ( const QString& str )
{
    uint len = str.length();

    if (len > 1 && str[0] == '$')
    {
      int pos = str.find ('/');

      if (pos < 0)
        pos = len;

      const char* ret = qgetenv( QString(str.unicode()+1, pos-1).toLocal8Bit().constData() );

      if (ret)
      {
        QString expandedStr ( QFile::decodeName( ret ) );
        if (pos < (int)len)
          expandedStr += str.mid(pos);
        return expandedStr;
      }
    }

    return str;
}

