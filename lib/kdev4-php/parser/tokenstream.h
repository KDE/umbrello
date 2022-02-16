/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPTOKENSTREAM_H
#define PHPTOKENSTREAM_H

#include <kdev-pg-token-stream.h>

namespace Php
{

class Token : public KDevPG::Token
{
public:
    qint64 docCommentBegin;
    qint64 docCommentEnd;
};

class TokenStream : public KDevPG::TokenStreamBase<Token>
{
};

}
#endif
