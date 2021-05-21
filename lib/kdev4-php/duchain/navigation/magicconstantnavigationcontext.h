/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
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
