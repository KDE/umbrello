/*
   Copyright 2009 Milian Wolff <mail@milianw.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MAGICCONSTANTNAVIGATIONCONTEXT_H
#define MAGICCONSTANTNAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/editor/cursorinrevision.h>

namespace Php {

class MagicConstantNavigationContext : public KDevelop::AbstractNavigationContext {
public:
    MagicConstantNavigationContext(KDevelop::TopDUContextPointer topContext,
                                   const KDevelop::SimpleCursor& position,
                                   const QString& constant);
    virtual QString name() const;
    virtual QString html(bool shorten = false);
private:
    KDevelop::CursorInRevision m_position;
    QString m_constant;
};

}

#endif // MAGICCONSTANTNAVIGATIONCONTEXT_H
