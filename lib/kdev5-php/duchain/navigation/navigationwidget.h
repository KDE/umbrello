/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PHP_NAVIGATIONWIDGET_H
#define PHP_NAVIGATIONWIDGET_H

#include <language/duchain/navigation/abstractnavigationwidget.h>
#include "phpduchainexport.h"

namespace KDevelop {
class IncludeItem;
class SimpleCursor;
}

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT NavigationWidget : public KDevelop::AbstractNavigationWidget
{
    Q_OBJECT
public:

    /**
      * @param htmlPrefix and @param htmlSuffix can be used to add own text before/behind the
      * normal text in the navigation-widget.
      * The texts may contain links to files, using a simple special Syntax:
      * KDEV_FILE_LINK{File}. Every occurrence of KDEV_FILE_LINK will be replaced.
      * */
    NavigationWidget(KDevelop::DeclarationPointer declaration, KDevelop::TopDUContextPointer topContext,
                     const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString(),
                     KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);

    NavigationWidget(const KDevelop::IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext,
                     const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString(),
                     KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);

    /// use this for magic constants
    /// @param position the position of the "use" of the magic constant
    /// @param constant the name of the magic constant, e.g. __LINE__
    NavigationWidget(KDevelop::TopDUContextPointer topContext, KTextEditor::Cursor position, const QString& constant,
                     KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);

    /**
    * Creates a compact html description-text
    **/
    static QString shortDescription(KDevelop::Declaration* declaration);
    static QString shortDescription(const KDevelop::IncludeItem& includeItem);

protected:
    KDevelop::DeclarationPointer m_declaration;
};

}

#endif
