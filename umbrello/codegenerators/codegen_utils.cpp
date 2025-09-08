/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codegen_utils.h"

// app includes
#include "umlapp.h"
#include "umldoc.h"

namespace Codegen_Utils {

/**
 * Return list of C++ datatypes.
 */
QStringList cppDatatypes()
{
    QStringList l;
    l.append(QStringLiteral("char"));
    l.append(QStringLiteral("int"));
    l.append(QStringLiteral("float"));
    l.append(QStringLiteral("double"));
    l.append(QStringLiteral("bool"));
    l.append(QStringLiteral("string"));
    l.append(QStringLiteral("unsigned char"));
    l.append(QStringLiteral("signed char"));
    l.append(QStringLiteral("unsigned int"));
    l.append(QStringLiteral("signed int"));
    l.append(QStringLiteral("short int"));
    l.append(QStringLiteral("unsigned short int"));
    l.append(QStringLiteral("signed short int"));
    l.append(QStringLiteral("long int"));
    l.append(QStringLiteral("signed long int"));
    l.append(QStringLiteral("unsigned long int"));
    l.append(QStringLiteral("long double"));
    l.append(QStringLiteral("wchar_t"));

    return l;
}

/**
 * Get list of C++ reserved keywords.
 */
const QStringList reservedCppKeywords()
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords.append(QStringLiteral("and"));
        keywords.append(QStringLiteral("and_eq"));
        keywords.append(QStringLiteral("__asm__"));
        keywords.append(QStringLiteral("asm"));
        keywords.append(QStringLiteral("__attribute__"));
        keywords.append(QStringLiteral("auto"));
        keywords.append(QStringLiteral("bitand"));
        keywords.append(QStringLiteral("bitor"));
        keywords.append(QStringLiteral("bool"));
        keywords.append(QStringLiteral("break"));
        keywords.append(QStringLiteral("BUFSIZ"));
        keywords.append(QStringLiteral("case"));
        keywords.append(QStringLiteral("catch"));
        keywords.append(QStringLiteral("char"));
        keywords.append(QStringLiteral("CHAR_BIT"));
        keywords.append(QStringLiteral("CHAR_MAX"));
        keywords.append(QStringLiteral("CHAR_MIN"));
        keywords.append(QStringLiteral("class"));
        keywords.append(QStringLiteral("CLOCKS_PER_SEC"));
        keywords.append(QStringLiteral("clock_t"));
        keywords.append(QStringLiteral("compl"));
        keywords.append(QStringLiteral("__complex__"));
        keywords.append(QStringLiteral("complex"));
        keywords.append(QStringLiteral("const"));
        keywords.append(QStringLiteral("const_cast"));
        keywords.append(QStringLiteral("continue"));
        keywords.append(QStringLiteral("__DATE__"));
        keywords.append(QStringLiteral("DBL_DIG"));
        keywords.append(QStringLiteral("DBL_EPSILON"));
        keywords.append(QStringLiteral("DBL_MANT_DIG"));
        keywords.append(QStringLiteral("DBL_MAX"));
        keywords.append(QStringLiteral("DBL_MAX_10_EXP"));
        keywords.append(QStringLiteral("DBL_MAX_EXP"));
        keywords.append(QStringLiteral("DBL_MIN"));
        keywords.append(QStringLiteral("DBL_MIN_10_EXP"));
        keywords.append(QStringLiteral("DBL_MIN_EXP"));
        keywords.append(QStringLiteral("default"));
        keywords.append(QStringLiteral("delete"));
        keywords.append(QStringLiteral("DIR"));
        keywords.append(QStringLiteral("div_t"));
        keywords.append(QStringLiteral("do"));
        keywords.append(QStringLiteral("double"));
        keywords.append(QStringLiteral("dynamic_cast"));
        keywords.append(QStringLiteral("E2BIG"));
        keywords.append(QStringLiteral("EACCES"));
        keywords.append(QStringLiteral("EAGAIN"));
        keywords.append(QStringLiteral("EBADF"));
        keywords.append(QStringLiteral("EBADMSG"));
        keywords.append(QStringLiteral("EBUSY"));
        keywords.append(QStringLiteral("ECANCELED"));
        keywords.append(QStringLiteral("ECHILD"));
        keywords.append(QStringLiteral("EDEADLK"));
        keywords.append(QStringLiteral("EDOM"));
        keywords.append(QStringLiteral("EEXIST"));
        keywords.append(QStringLiteral("EFAULT"));
        keywords.append(QStringLiteral("EFBIG"));
        keywords.append(QStringLiteral("EILSEQ"));
        keywords.append(QStringLiteral("EINPROGRESS"));
        keywords.append(QStringLiteral("EINTR"));
        keywords.append(QStringLiteral("EINVAL"));
        keywords.append(QStringLiteral("EIO"));
        keywords.append(QStringLiteral("EISDIR"));
        keywords.append(QStringLiteral("else"));
        keywords.append(QStringLiteral("EMFILE"));
        keywords.append(QStringLiteral("EMLINK"));
        keywords.append(QStringLiteral("EMSGSIZE"));
        keywords.append(QStringLiteral("ENAMETOOLONG"));
        keywords.append(QStringLiteral("ENFILE"));
        keywords.append(QStringLiteral("ENODEV"));
        keywords.append(QStringLiteral("ENOENT"));
        keywords.append(QStringLiteral("ENOEXEC"));
        keywords.append(QStringLiteral("ENOLCK"));
        keywords.append(QStringLiteral("ENOMEM"));
        keywords.append(QStringLiteral("ENOSPC"));
        keywords.append(QStringLiteral("ENOSYS"));
        keywords.append(QStringLiteral("ENOTDIR"));
        keywords.append(QStringLiteral("ENOTEMPTY"));
        keywords.append(QStringLiteral("ENOTSUP"));
        keywords.append(QStringLiteral("ENOTTY"));
        keywords.append(QStringLiteral("enum"));
        keywords.append(QStringLiteral("ENXIO"));
        keywords.append(QStringLiteral("EOF"));
        keywords.append(QStringLiteral("EPERM"));
        keywords.append(QStringLiteral("EPIPE"));
        keywords.append(QStringLiteral("ERANGE"));
        keywords.append(QStringLiteral("EROFS"));
        keywords.append(QStringLiteral("ESPIPE"));
        keywords.append(QStringLiteral("ESRCH"));
        keywords.append(QStringLiteral("ETIMEDOUT"));
        keywords.append(QStringLiteral("EXDEV"));
        keywords.append(QStringLiteral("EXIT_FAILURE"));
        keywords.append(QStringLiteral("EXIT_SUCCESS"));
        keywords.append(QStringLiteral("explicit"));
        keywords.append(QStringLiteral("export"));
        keywords.append(QStringLiteral("extern"));
        keywords.append(QStringLiteral("false"));
        keywords.append(QStringLiteral("__FILE__"));
        keywords.append(QStringLiteral("FILE"));
        keywords.append(QStringLiteral("FILENAME_MAX"));
        keywords.append(QStringLiteral("float"));
        keywords.append(QStringLiteral("FLT_DIG"));
        keywords.append(QStringLiteral("FLT_EPSILON"));
        keywords.append(QStringLiteral("FLT_MANT_DIG"));
        keywords.append(QStringLiteral("FLT_MAX"));
        keywords.append(QStringLiteral("FLT_MAX_10_EXP"));
        keywords.append(QStringLiteral("FLT_MAX_EXP"));
        keywords.append(QStringLiteral("FLT_MIN"));
        keywords.append(QStringLiteral("FLT_MIN_10_EXP"));
        keywords.append(QStringLiteral("FLT_MIN_EXP"));
        keywords.append(QStringLiteral("FLT_RADIX"));
        keywords.append(QStringLiteral("FLT_ROUNDS"));
        keywords.append(QStringLiteral("FOPEN_MAX"));
        keywords.append(QStringLiteral("for"));
        keywords.append(QStringLiteral("fpos_t"));
        keywords.append(QStringLiteral("friend"));
        keywords.append(QStringLiteral("__FUNCTION__"));
        keywords.append(QStringLiteral("__GNUC__"));
        keywords.append(QStringLiteral("goto"));
        keywords.append(QStringLiteral("HUGE_VAL"));
        keywords.append(QStringLiteral("if"));
        keywords.append(QStringLiteral("__imag__"));
        keywords.append(QStringLiteral("inline"));
        keywords.append(QStringLiteral("int"));
        keywords.append(QStringLiteral("INT16_MAX"));
        keywords.append(QStringLiteral("INT16_MIN"));
        keywords.append(QStringLiteral("int16_t"));
        keywords.append(QStringLiteral("INT32_MAX"));
        keywords.append(QStringLiteral("INT32_MIN"));
        keywords.append(QStringLiteral("int32_t"));
        keywords.append(QStringLiteral("INT64_MAX"));
        keywords.append(QStringLiteral("INT64_MIN"));
        keywords.append(QStringLiteral("int64_t"));
        keywords.append(QStringLiteral("INT8_MAX"));
        keywords.append(QStringLiteral("INT8_MIN"));
        keywords.append(QStringLiteral("int8_t"));
        keywords.append(QStringLiteral("INT_FAST16_MAX"));
        keywords.append(QStringLiteral("INT_FAST16_MIN"));
        keywords.append(QStringLiteral("int_fast16_t"));
        keywords.append(QStringLiteral("INT_FAST32_MAX"));
        keywords.append(QStringLiteral("INT_FAST32_MIN"));
        keywords.append(QStringLiteral("int_fast32_t"));
        keywords.append(QStringLiteral("INT_FAST64_MAX"));
        keywords.append(QStringLiteral("INT_FAST64_MIN"));
        keywords.append(QStringLiteral("int_fast64_t"));
        keywords.append(QStringLiteral("INT_FAST8_MAX"));
        keywords.append(QStringLiteral("INT_FAST8_MIN"));
        keywords.append(QStringLiteral("int_fast8_t"));
        keywords.append(QStringLiteral("INT_LEAST16_MAX"));
        keywords.append(QStringLiteral("INT_LEAST16_MIN"));
        keywords.append(QStringLiteral("int_least16_t"));
        keywords.append(QStringLiteral("INT_LEAST32_MAX"));
        keywords.append(QStringLiteral("INT_LEAST32_MIN"));
        keywords.append(QStringLiteral("int_least32_t"));
        keywords.append(QStringLiteral("INT_LEAST64_MAX"));
        keywords.append(QStringLiteral("INT_LEAST64_MIN"));
        keywords.append(QStringLiteral("int_least64_t"));
        keywords.append(QStringLiteral("INT_LEAST8_MAX"));
        keywords.append(QStringLiteral("INT_LEAST8_MIN"));
        keywords.append(QStringLiteral("int_least8_t"));
        keywords.append(QStringLiteral("INT_MAX"));
        keywords.append(QStringLiteral("INTMAX_MAX"));
        keywords.append(QStringLiteral("INTMAX_MIN"));
        keywords.append(QStringLiteral("intmax_t"));
        keywords.append(QStringLiteral("INT_MIN"));
        keywords.append(QStringLiteral("INTPTR_MAX"));
        keywords.append(QStringLiteral("INTPTR_MIN"));
        keywords.append(QStringLiteral("intptr_t"));
        keywords.append(QStringLiteral("_IOFBF"));
        keywords.append(QStringLiteral("_IOLBF"));
        keywords.append(QStringLiteral("_IONBF"));
        keywords.append(QStringLiteral("jmp_buf"));
        keywords.append(QStringLiteral("__label__"));
        keywords.append(QStringLiteral("LC_ALL"));
        keywords.append(QStringLiteral("LC_COLLATE"));
        keywords.append(QStringLiteral("LC_CTYPE"));
        keywords.append(QStringLiteral("LC_MONETARY"));
        keywords.append(QStringLiteral("LC_NUMERIC"));
        keywords.append(QStringLiteral("LC_TIME"));
        keywords.append(QStringLiteral("LDBL_DIG"));
        keywords.append(QStringLiteral("LDBL_EPSILON"));
        keywords.append(QStringLiteral("LDBL_MANT_DIG"));
        keywords.append(QStringLiteral("LDBL_MAX"));
        keywords.append(QStringLiteral("LDBL_MAX_10_EXP"));
        keywords.append(QStringLiteral("LDBL_MAX_EXP"));
        keywords.append(QStringLiteral("LDBL_MIN"));
        keywords.append(QStringLiteral("LDBL_MIN_10_EXP"));
        keywords.append(QStringLiteral("LDBL_MIN_EXP"));
        keywords.append(QStringLiteral("ldiv_t"));
        keywords.append(QStringLiteral("__LINE__"));
        keywords.append(QStringLiteral("LLONG_MAX"));
        keywords.append(QStringLiteral("long"));
        keywords.append(QStringLiteral("LONG_MAX"));
        keywords.append(QStringLiteral("LONG_MIN"));
        keywords.append(QStringLiteral("L_tmpnam"));
        keywords.append(QStringLiteral("M_1_PI"));
        keywords.append(QStringLiteral("M_2_PI"));
        keywords.append(QStringLiteral("M_2_SQRTPI"));
        keywords.append(QStringLiteral("MB_CUR_MAX"));
        keywords.append(QStringLiteral("MB_LEN_MAX"));
        keywords.append(QStringLiteral("mbstate_t"));
        keywords.append(QStringLiteral("M_E"));
        keywords.append(QStringLiteral("M_LN10"));
        keywords.append(QStringLiteral("M_LN2"));
        keywords.append(QStringLiteral("M_LOG10E"));
        keywords.append(QStringLiteral("M_LOG2E"));
        keywords.append(QStringLiteral("M_PI"));
        keywords.append(QStringLiteral("M_PI_2"));
        keywords.append(QStringLiteral("M_PI_4"));
        keywords.append(QStringLiteral("M_SQRT1_2"));
        keywords.append(QStringLiteral("M_SQRT2"));
        keywords.append(QStringLiteral("mutable"));
        keywords.append(QStringLiteral("namespace"));
        keywords.append(QStringLiteral("new"));
        keywords.append(QStringLiteral("not"));
        keywords.append(QStringLiteral("not_eq"));
        keywords.append(QStringLiteral("NPOS"));
        keywords.append(QStringLiteral("NULL"));
        keywords.append(QStringLiteral("operator"));
        keywords.append(QStringLiteral("or"));
        keywords.append(QStringLiteral("or_eq"));
        keywords.append(QStringLiteral("__PRETTY_FUNCTION__"));
        keywords.append(QStringLiteral("private"));
        keywords.append(QStringLiteral("protected"));
        keywords.append(QStringLiteral("PTRDIFF_MAX"));
        keywords.append(QStringLiteral("PTRDIFF_MIN"));
        keywords.append(QStringLiteral("ptrdiff_t"));
        keywords.append(QStringLiteral("public"));
        keywords.append(QStringLiteral("RAND_MAX"));
        keywords.append(QStringLiteral("__real__"));
        keywords.append(QStringLiteral("register"));
        keywords.append(QStringLiteral("reinterpret_cast"));
        keywords.append(QStringLiteral("restrict"));
        keywords.append(QStringLiteral("return"));
        keywords.append(QStringLiteral("SCHAR_MAX"));
        keywords.append(QStringLiteral("SCHAR_MIN"));
        keywords.append(QStringLiteral("SEEK_CUR"));
        keywords.append(QStringLiteral("SEEK_END"));
        keywords.append(QStringLiteral("SEEK_SET"));
        keywords.append(QStringLiteral("short"));
        keywords.append(QStringLiteral("SHRT_MAX"));
        keywords.append(QStringLiteral("SHRT_MIN"));
        keywords.append(QStringLiteral("SIGABRT"));
        keywords.append(QStringLiteral("SIGALRM"));
        keywords.append(QStringLiteral("SIG_ATOMIC_MAX"));
        keywords.append(QStringLiteral("SIG_ATOMIC_MIN"));
        keywords.append(QStringLiteral("sig_atomic_t"));
        keywords.append(QStringLiteral("SIGCHLD"));
        keywords.append(QStringLiteral("SIGCONT"));
        keywords.append(QStringLiteral("SIG_DFL"));
        keywords.append(QStringLiteral("SIG_ERR"));
        keywords.append(QStringLiteral("SIGFPE"));
        keywords.append(QStringLiteral("SIGHUP"));
        keywords.append(QStringLiteral("SIG_IGN"));
        keywords.append(QStringLiteral("SIGILL"));
        keywords.append(QStringLiteral("SIGINT"));
        keywords.append(QStringLiteral("SIGKILL"));
        keywords.append(QStringLiteral("signed"));
        keywords.append(QStringLiteral("SIGPIPE"));
        keywords.append(QStringLiteral("SIGQUIT"));
        keywords.append(QStringLiteral("SIGSEGV"));
        keywords.append(QStringLiteral("SIGSTOP"));
        keywords.append(QStringLiteral("SIGTERM"));
        keywords.append(QStringLiteral("SIGTRAP"));
        keywords.append(QStringLiteral("SIGTSTP"));
        keywords.append(QStringLiteral("SIGTTIN"));
        keywords.append(QStringLiteral("SIGTTOU"));
        keywords.append(QStringLiteral("SIGUSR1"));
        keywords.append(QStringLiteral("SIGUSR2"));
        keywords.append(QStringLiteral("SINT_MAX"));
        keywords.append(QStringLiteral("SINT_MIN"));
        keywords.append(QStringLiteral("SIZE_MAX"));
        keywords.append(QStringLiteral("sizeof"));
        keywords.append(QStringLiteral("size_t"));
        keywords.append(QStringLiteral("SLONG_MAX"));
        keywords.append(QStringLiteral("SLONG_MIN"));
        keywords.append(QStringLiteral("SSHRT_MAX"));
        keywords.append(QStringLiteral("SSHRT_MIN"));
        keywords.append(QStringLiteral("ssize_t"));
        keywords.append(QStringLiteral("static"));
        keywords.append(QStringLiteral("constexpr"));
        keywords.append(QStringLiteral("static_cast"));
        keywords.append(QStringLiteral("__STDC__"));
        keywords.append(QStringLiteral("__STDC_VERSION__"));
        keywords.append(QStringLiteral("stderr"));
        keywords.append(QStringLiteral("stdin"));
        keywords.append(QStringLiteral("stdout"));
        keywords.append(QStringLiteral("struct"));
        keywords.append(QStringLiteral("switch"));
        keywords.append(QStringLiteral("template"));
        keywords.append(QStringLiteral("this"));
        keywords.append(QStringLiteral("throw"));
        keywords.append(QStringLiteral("__TIME__"));
        keywords.append(QStringLiteral("time_t"));
        keywords.append(QStringLiteral("TMP_MAX"));
        keywords.append(QStringLiteral("true"));
        keywords.append(QStringLiteral("try"));
        keywords.append(QStringLiteral("typedef"));
        keywords.append(QStringLiteral("typeid"));
        keywords.append(QStringLiteral("typename"));
        keywords.append(QStringLiteral("typeof"));
        keywords.append(QStringLiteral("UCHAR_MAX"));
        keywords.append(QStringLiteral("UINT16_MAX"));
        keywords.append(QStringLiteral("uint16_t"));
        keywords.append(QStringLiteral("UINT32_MAX"));
        keywords.append(QStringLiteral("uint32_t"));
        keywords.append(QStringLiteral("UINT64_MAX"));
        keywords.append(QStringLiteral("uint64_t"));
        keywords.append(QStringLiteral("UINT8_MAX"));
        keywords.append(QStringLiteral("uint8_t"));
        keywords.append(QStringLiteral("UINT_FAST16_MAX"));
        keywords.append(QStringLiteral("uint_fast16_t"));
        keywords.append(QStringLiteral("UINT_FAST32_MAX"));
        keywords.append(QStringLiteral("uint_fast32_t"));
        keywords.append(QStringLiteral("UINT_FAST64_MAX"));
        keywords.append(QStringLiteral("uint_fast64_t"));
        keywords.append(QStringLiteral("UINT_FAST8_MAX"));
        keywords.append(QStringLiteral("uint_fast8_t"));
        keywords.append(QStringLiteral("UINT_LEAST16_MAX"));
        keywords.append(QStringLiteral("uint_least16_t"));
        keywords.append(QStringLiteral("UINT_LEAST32_MAX"));
        keywords.append(QStringLiteral("uint_least32_t"));
        keywords.append(QStringLiteral("UINT_LEAST64_MAX"));
        keywords.append(QStringLiteral("uint_least64_t"));
        keywords.append(QStringLiteral("UINT_LEAST8_MAX"));
        keywords.append(QStringLiteral("uint_least8_t"));
        keywords.append(QStringLiteral("UINT_MAX"));
        keywords.append(QStringLiteral("UINTMAX_MAX"));
        keywords.append(QStringLiteral("uintmax_t"));
        keywords.append(QStringLiteral("UINTPTR_MAX"));
        keywords.append(QStringLiteral("uintptr_t"));
        keywords.append(QStringLiteral("ULLONG_MAX"));
        keywords.append(QStringLiteral("ULONG_MAX"));
        keywords.append(QStringLiteral("union"));
        keywords.append(QStringLiteral("unsigned"));
        keywords.append(QStringLiteral("USHRT_MAX"));
        keywords.append(QStringLiteral("using"));
        keywords.append(QStringLiteral("va_list"));
        keywords.append(QStringLiteral("virtual"));
        keywords.append(QStringLiteral("void"));
        keywords.append(QStringLiteral("__volatile__"));
        keywords.append(QStringLiteral("volatile"));
        keywords.append(QStringLiteral("WCHAR_MAX"));
        keywords.append(QStringLiteral("WCHAR_MIN"));
        keywords.append(QStringLiteral("wchar_t"));
        keywords.append(QStringLiteral("wctrans_t"));
        keywords.append(QStringLiteral("wctype_t"));
        keywords.append(QStringLiteral("WEOF"));
        keywords.append(QStringLiteral("while"));
        keywords.append(QStringLiteral("WINT_MAX"));
        keywords.append(QStringLiteral("WINT_MIN"));
        keywords.append(QStringLiteral("wint_t"));
        keywords.append(QStringLiteral("xor"));
        keywords.append(QStringLiteral("xor_eq"));
    }

    return keywords;
}

/**
 * Add C++ stereotypes.
 */
void createCppStereotypes()
{
    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->findOrCreateStereotype(QStringLiteral("constructor"));
    // declares an operation as friend
    umldoc->findOrCreateStereotype(QStringLiteral("friend"));
    // to use in methods that aren't abstract
    umldoc->findOrCreateStereotype(QStringLiteral("virtual"));
}

/**
 * Return the input string with the first letter capitalized.
 */
QString capitalizeFirstLetter(const QString &string)
{
    if (string.isEmpty())
        return QString();
    QChar firstChar = string.at(0);
    return firstChar.toUpper() + string.mid(1);
}

}  // end namespace Codegen_Utils

