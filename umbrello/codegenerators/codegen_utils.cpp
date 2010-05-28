/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Copyright (C) 2004-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegen_utils.h"

// app includes
#include "uml.h"
#include "umldoc.h"

namespace Codegen_Utils {

/**
 * Return list of C++ datatypes.
 */
QStringList cppDatatypes()
{
    QStringList l;
    l.append("int");
    l.append("char");
    l.append("bool");
    l.append("float");
    l.append("double");
    l.append("short");
    l.append("long");
    l.append("unsigned int");
    l.append("unsigned short");
    l.append("unsigned long");
    l.append("string");
    return l;
}

/**
 * Get list of C++ reserved keywords.
 */
const QStringList reservedCppKeywords()
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords.append( "and" );
        keywords.append( "and_eq" );
        keywords.append( "__asm__" );
        keywords.append( "asm" );
        keywords.append( "__attribute__" );
        keywords.append( "auto" );
        keywords.append( "bitand" );
        keywords.append( "bitor" );
        keywords.append( "bool" );
        keywords.append( "break" );
        keywords.append( "BUFSIZ" );
        keywords.append( "case" );
        keywords.append( "catch" );
        keywords.append( "char" );
        keywords.append( "CHAR_BIT" );
        keywords.append( "CHAR_MAX" );
        keywords.append( "CHAR_MIN" );
        keywords.append( "class" );
        keywords.append( "CLOCKS_PER_SEC" );
        keywords.append( "clock_t" );
        keywords.append( "compl" );
        keywords.append( "__complex__" );
        keywords.append( "complex" );
        keywords.append( "const" );
        keywords.append( "const_cast" );
        keywords.append( "continue" );
        keywords.append( "__DATE__" );
        keywords.append( "DBL_DIG" );
        keywords.append( "DBL_EPSILON" );
        keywords.append( "DBL_MANT_DIG" );
        keywords.append( "DBL_MAX" );
        keywords.append( "DBL_MAX_10_EXP" );
        keywords.append( "DBL_MAX_EXP" );
        keywords.append( "DBL_MIN" );
        keywords.append( "DBL_MIN_10_EXP" );
        keywords.append( "DBL_MIN_EXP" );
        keywords.append( "default" );
        keywords.append( "delete" );
        keywords.append( "DIR" );
        keywords.append( "div_t" );
        keywords.append( "do" );
        keywords.append( "double" );
        keywords.append( "dynamic_cast" );
        keywords.append( "E2BIG" );
        keywords.append( "EACCES" );
        keywords.append( "EAGAIN" );
        keywords.append( "EBADF" );
        keywords.append( "EBADMSG" );
        keywords.append( "EBUSY" );
        keywords.append( "ECANCELED" );
        keywords.append( "ECHILD" );
        keywords.append( "EDEADLK" );
        keywords.append( "EDOM" );
        keywords.append( "EEXIST" );
        keywords.append( "EFAULT" );
        keywords.append( "EFBIG" );
        keywords.append( "EILSEQ" );
        keywords.append( "EINPROGRESS" );
        keywords.append( "EINTR" );
        keywords.append( "EINVAL" );
        keywords.append( "EIO" );
        keywords.append( "EISDIR" );
        keywords.append( "else" );
        keywords.append( "EMFILE" );
        keywords.append( "EMLINK" );
        keywords.append( "EMSGSIZE" );
        keywords.append( "ENAMETOOLONG" );
        keywords.append( "ENFILE" );
        keywords.append( "ENODEV" );
        keywords.append( "ENOENT" );
        keywords.append( "ENOEXEC" );
        keywords.append( "ENOLCK" );
        keywords.append( "ENOMEM" );
        keywords.append( "ENOSPC" );
        keywords.append( "ENOSYS" );
        keywords.append( "ENOTDIR" );
        keywords.append( "ENOTEMPTY" );
        keywords.append( "ENOTSUP" );
        keywords.append( "ENOTTY" );
        keywords.append( "enum" );
        keywords.append( "ENXIO" );
        keywords.append( "EOF" );
        keywords.append( "EPERM" );
        keywords.append( "EPIPE" );
        keywords.append( "ERANGE" );
        keywords.append( "EROFS" );
        keywords.append( "ESPIPE" );
        keywords.append( "ESRCH" );
        keywords.append( "ETIMEDOUT" );
        keywords.append( "EXDEV" );
        keywords.append( "EXIT_FAILURE" );
        keywords.append( "EXIT_SUCCESS" );
        keywords.append( "explicit" );
        keywords.append( "export" );
        keywords.append( "extern" );
        keywords.append( "false" );
        keywords.append( "__FILE__" );
        keywords.append( "FILE" );
        keywords.append( "FILENAME_MAX" );
        keywords.append( "float" );
        keywords.append( "FLT_DIG" );
        keywords.append( "FLT_EPSILON" );
        keywords.append( "FLT_MANT_DIG" );
        keywords.append( "FLT_MAX" );
        keywords.append( "FLT_MAX_10_EXP" );
        keywords.append( "FLT_MAX_EXP" );
        keywords.append( "FLT_MIN" );
        keywords.append( "FLT_MIN_10_EXP" );
        keywords.append( "FLT_MIN_EXP" );
        keywords.append( "FLT_RADIX" );
        keywords.append( "FLT_ROUNDS" );
        keywords.append( "FOPEN_MAX" );
        keywords.append( "for" );
        keywords.append( "fpos_t" );
        keywords.append( "friend" );
        keywords.append( "__FUNCTION__" );
        keywords.append( "__GNUC__" );
        keywords.append( "goto" );
        keywords.append( "HUGE_VAL" );
        keywords.append( "if" );
        keywords.append( "__imag__" );
        keywords.append( "inline" );
        keywords.append( "int" );
        keywords.append( "INT16_MAX" );
        keywords.append( "INT16_MIN" );
        keywords.append( "int16_t" );
        keywords.append( "INT32_MAX" );
        keywords.append( "INT32_MIN" );
        keywords.append( "int32_t" );
        keywords.append( "INT64_MAX" );
        keywords.append( "INT64_MIN" );
        keywords.append( "int64_t" );
        keywords.append( "INT8_MAX" );
        keywords.append( "INT8_MIN" );
        keywords.append( "int8_t" );
        keywords.append( "INT_FAST16_MAX" );
        keywords.append( "INT_FAST16_MIN" );
        keywords.append( "int_fast16_t" );
        keywords.append( "INT_FAST32_MAX" );
        keywords.append( "INT_FAST32_MIN" );
        keywords.append( "int_fast32_t" );
        keywords.append( "INT_FAST64_MAX" );
        keywords.append( "INT_FAST64_MIN" );
        keywords.append( "int_fast64_t" );
        keywords.append( "INT_FAST8_MAX" );
        keywords.append( "INT_FAST8_MIN" );
        keywords.append( "int_fast8_t" );
        keywords.append( "INT_LEAST16_MAX" );
        keywords.append( "INT_LEAST16_MIN" );
        keywords.append( "int_least16_t" );
        keywords.append( "INT_LEAST32_MAX" );
        keywords.append( "INT_LEAST32_MIN" );
        keywords.append( "int_least32_t" );
        keywords.append( "INT_LEAST64_MAX" );
        keywords.append( "INT_LEAST64_MIN" );
        keywords.append( "int_least64_t" );
        keywords.append( "INT_LEAST8_MAX" );
        keywords.append( "INT_LEAST8_MIN" );
        keywords.append( "int_least8_t" );
        keywords.append( "INT_MAX" );
        keywords.append( "INTMAX_MAX" );
        keywords.append( "INTMAX_MIN" );
        keywords.append( "intmax_t" );
        keywords.append( "INT_MIN" );
        keywords.append( "INTPTR_MAX" );
        keywords.append( "INTPTR_MIN" );
        keywords.append( "intptr_t" );
        keywords.append( "_IOFBF" );
        keywords.append( "_IOLBF" );
        keywords.append( "_IONBF" );
        keywords.append( "jmp_buf" );
        keywords.append( "__label__" );
        keywords.append( "LC_ALL" );
        keywords.append( "LC_COLLATE" );
        keywords.append( "LC_CTYPE" );
        keywords.append( "LC_MONETARY" );
        keywords.append( "LC_NUMERIC" );
        keywords.append( "LC_TIME" );
        keywords.append( "LDBL_DIG" );
        keywords.append( "LDBL_EPSILON" );
        keywords.append( "LDBL_MANT_DIG" );
        keywords.append( "LDBL_MAX" );
        keywords.append( "LDBL_MAX_10_EXP" );
        keywords.append( "LDBL_MAX_EXP" );
        keywords.append( "LDBL_MIN" );
        keywords.append( "LDBL_MIN_10_EXP" );
        keywords.append( "LDBL_MIN_EXP" );
        keywords.append( "ldiv_t" );
        keywords.append( "__LINE__" );
        keywords.append( "LLONG_MAX" );
        keywords.append( "long" );
        keywords.append( "LONG_MAX" );
        keywords.append( "LONG_MIN" );
        keywords.append( "L_tmpnam" );
        keywords.append( "M_1_PI" );
        keywords.append( "M_2_PI" );
        keywords.append( "M_2_SQRTPI" );
        keywords.append( "MB_CUR_MAX" );
        keywords.append( "MB_LEN_MAX" );
        keywords.append( "mbstate_t" );
        keywords.append( "M_E" );
        keywords.append( "M_LN10" );
        keywords.append( "M_LN2" );
        keywords.append( "M_LOG10E" );
        keywords.append( "M_LOG2E" );
        keywords.append( "M_PI" );
        keywords.append( "M_PI_2" );
        keywords.append( "M_PI_4" );
        keywords.append( "M_SQRT1_2" );
        keywords.append( "M_SQRT2" );
        keywords.append( "mutable" );
        keywords.append( "namespace" );
        keywords.append( "new" );
        keywords.append( "not" );
        keywords.append( "not_eq" );
        keywords.append( "NPOS" );
        keywords.append( "NULL" );
        keywords.append( "operator" );
        keywords.append( "or" );
        keywords.append( "or_eq" );
        keywords.append( "__PRETTY_FUNCTION__" );
        keywords.append( "private" );
        keywords.append( "protected" );
        keywords.append( "PTRDIFF_MAX" );
        keywords.append( "PTRDIFF_MIN" );
        keywords.append( "ptrdiff_t" );
        keywords.append( "public" );
        keywords.append( "RAND_MAX" );
        keywords.append( "__real__" );
        keywords.append( "register" );
        keywords.append( "reinterpret_cast" );
        keywords.append( "restrict" );
        keywords.append( "return" );
        keywords.append( "SCHAR_MAX" );
        keywords.append( "SCHAR_MIN" );
        keywords.append( "SEEK_CUR" );
        keywords.append( "SEEK_END" );
        keywords.append( "SEEK_SET" );
        keywords.append( "short" );
        keywords.append( "SHRT_MAX" );
        keywords.append( "SHRT_MIN" );
        keywords.append( "SIGABRT" );
        keywords.append( "SIGALRM" );
        keywords.append( "SIG_ATOMIC_MAX" );
        keywords.append( "SIG_ATOMIC_MIN" );
        keywords.append( "sig_atomic_t" );
        keywords.append( "SIGCHLD" );
        keywords.append( "SIGCONT" );
        keywords.append( "SIG_DFL" );
        keywords.append( "SIG_ERR" );
        keywords.append( "SIGFPE" );
        keywords.append( "SIGHUP" );
        keywords.append( "SIG_IGN" );
        keywords.append( "SIGILL" );
        keywords.append( "SIGINT" );
        keywords.append( "SIGKILL" );
        keywords.append( "signed" );
        keywords.append( "SIGPIPE" );
        keywords.append( "SIGQUIT" );
        keywords.append( "SIGSEGV" );
        keywords.append( "SIGSTOP" );
        keywords.append( "SIGTERM" );
        keywords.append( "SIGTRAP" );
        keywords.append( "SIGTSTP" );
        keywords.append( "SIGTTIN" );
        keywords.append( "SIGTTOU" );
        keywords.append( "SIGUSR1" );
        keywords.append( "SIGUSR2" );
        keywords.append( "SINT_MAX" );
        keywords.append( "SINT_MIN" );
        keywords.append( "SIZE_MAX" );
        keywords.append( "sizeof" );
        keywords.append( "size_t" );
        keywords.append( "SLONG_MAX" );
        keywords.append( "SLONG_MIN" );
        keywords.append( "SSHRT_MAX" );
        keywords.append( "SSHRT_MIN" );
        keywords.append( "ssize_t" );
        keywords.append( "static" );
        keywords.append( "static_cast" );
        keywords.append( "__STDC__" );
        keywords.append( "__STDC_VERSION__" );
        keywords.append( "stderr" );
        keywords.append( "stdin" );
        keywords.append( "stdout" );
        keywords.append( "struct" );
        keywords.append( "switch" );
        keywords.append( "template" );
        keywords.append( "this" );
        keywords.append( "throw" );
        keywords.append( "__TIME__" );
        keywords.append( "time_t" );
        keywords.append( "TMP_MAX" );
        keywords.append( "true" );
        keywords.append( "try" );
        keywords.append( "typedef" );
        keywords.append( "typeid" );
        keywords.append( "typename" );
        keywords.append( "typeof" );
        keywords.append( "UCHAR_MAX" );
        keywords.append( "UINT16_MAX" );
        keywords.append( "uint16_t" );
        keywords.append( "UINT32_MAX" );
        keywords.append( "uint32_t" );
        keywords.append( "UINT64_MAX" );
        keywords.append( "uint64_t" );
        keywords.append( "UINT8_MAX" );
        keywords.append( "uint8_t" );
        keywords.append( "UINT_FAST16_MAX" );
        keywords.append( "uint_fast16_t" );
        keywords.append( "UINT_FAST32_MAX" );
        keywords.append( "uint_fast32_t" );
        keywords.append( "UINT_FAST64_MAX" );
        keywords.append( "uint_fast64_t" );
        keywords.append( "UINT_FAST8_MAX" );
        keywords.append( "uint_fast8_t" );
        keywords.append( "UINT_LEAST16_MAX" );
        keywords.append( "uint_least16_t" );
        keywords.append( "UINT_LEAST32_MAX" );
        keywords.append( "uint_least32_t" );
        keywords.append( "UINT_LEAST64_MAX" );
        keywords.append( "uint_least64_t" );
        keywords.append( "UINT_LEAST8_MAX" );
        keywords.append( "uint_least8_t" );
        keywords.append( "UINT_MAX" );
        keywords.append( "UINTMAX_MAX" );
        keywords.append( "uintmax_t" );
        keywords.append( "UINTPTR_MAX" );
        keywords.append( "uintptr_t" );
        keywords.append( "ULLONG_MAX" );
        keywords.append( "ULONG_MAX" );
        keywords.append( "union" );
        keywords.append( "unsigned" );
        keywords.append( "USHRT_MAX" );
        keywords.append( "using" );
        keywords.append( "va_list" );
        keywords.append( "virtual" );
        keywords.append( "void" );
        keywords.append( "__volatile__" );
        keywords.append( "volatile" );
        keywords.append( "WCHAR_MAX" );
        keywords.append( "WCHAR_MIN" );
        keywords.append( "wchar_t" );
        keywords.append( "wctrans_t" );
        keywords.append( "wctype_t" );
        keywords.append( "WEOF" );
        keywords.append( "while" );
        keywords.append( "WINT_MAX" );
        keywords.append( "WINT_MIN" );
        keywords.append( "wint_t" );
        keywords.append( "xor" );
        keywords.append( "xor_eq" );
    }

    return keywords;
}

/**
 * Add C++ stereotypes.
 */
void createCppStereotypes()
{
    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->findOrCreateStereotype("constructor");
    // declares an operation as friend
    umldoc->findOrCreateStereotype("friend");
    // to use in methods that aren't abstract
    umldoc->findOrCreateStereotype("virtual");
}

/**
 * Return the input string with the first letter capitalized.
 */
QString capitalizeFirstLetter(const QString &string)
{
    QChar firstChar = string.at(0);
    return firstChar.toUpper() + string.mid(1);
}

}  // end namespace Codegen_Utils

