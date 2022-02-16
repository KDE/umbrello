/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
    l.append(QLatin1String("char"));
    l.append(QLatin1String("int"));
    l.append(QLatin1String("float"));
    l.append(QLatin1String("double"));
    l.append(QLatin1String("bool"));
    l.append(QLatin1String("string"));
    l.append(QLatin1String("unsigned char"));
    l.append(QLatin1String("signed char"));
    l.append(QLatin1String("unsigned int"));
    l.append(QLatin1String("signed int"));
    l.append(QLatin1String("short int"));
    l.append(QLatin1String("unsigned short int"));
    l.append(QLatin1String("signed short int"));
    l.append(QLatin1String("long int"));
    l.append(QLatin1String("signed long int"));
    l.append(QLatin1String("unsigned long int"));
    l.append(QLatin1String("long double"));
    l.append(QLatin1String("wchar_t"));

    return l;
}

/**
 * Get list of C++ reserved keywords.
 */
const QStringList reservedCppKeywords()
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords.append(QLatin1String("and"));
        keywords.append(QLatin1String("and_eq"));
        keywords.append(QLatin1String("__asm__"));
        keywords.append(QLatin1String("asm"));
        keywords.append(QLatin1String("__attribute__"));
        keywords.append(QLatin1String("auto"));
        keywords.append(QLatin1String("bitand"));
        keywords.append(QLatin1String("bitor"));
        keywords.append(QLatin1String("bool"));
        keywords.append(QLatin1String("break"));
        keywords.append(QLatin1String("BUFSIZ"));
        keywords.append(QLatin1String("case"));
        keywords.append(QLatin1String("catch"));
        keywords.append(QLatin1String("char"));
        keywords.append(QLatin1String("CHAR_BIT"));
        keywords.append(QLatin1String("CHAR_MAX"));
        keywords.append(QLatin1String("CHAR_MIN"));
        keywords.append(QLatin1String("class"));
        keywords.append(QLatin1String("CLOCKS_PER_SEC"));
        keywords.append(QLatin1String("clock_t"));
        keywords.append(QLatin1String("compl"));
        keywords.append(QLatin1String("__complex__"));
        keywords.append(QLatin1String("complex"));
        keywords.append(QLatin1String("const"));
        keywords.append(QLatin1String("const_cast"));
        keywords.append(QLatin1String("continue"));
        keywords.append(QLatin1String("__DATE__"));
        keywords.append(QLatin1String("DBL_DIG"));
        keywords.append(QLatin1String("DBL_EPSILON"));
        keywords.append(QLatin1String("DBL_MANT_DIG"));
        keywords.append(QLatin1String("DBL_MAX"));
        keywords.append(QLatin1String("DBL_MAX_10_EXP"));
        keywords.append(QLatin1String("DBL_MAX_EXP"));
        keywords.append(QLatin1String("DBL_MIN"));
        keywords.append(QLatin1String("DBL_MIN_10_EXP"));
        keywords.append(QLatin1String("DBL_MIN_EXP"));
        keywords.append(QLatin1String("default"));
        keywords.append(QLatin1String("delete"));
        keywords.append(QLatin1String("DIR"));
        keywords.append(QLatin1String("div_t"));
        keywords.append(QLatin1String("do"));
        keywords.append(QLatin1String("double"));
        keywords.append(QLatin1String("dynamic_cast"));
        keywords.append(QLatin1String("E2BIG"));
        keywords.append(QLatin1String("EACCES"));
        keywords.append(QLatin1String("EAGAIN"));
        keywords.append(QLatin1String("EBADF"));
        keywords.append(QLatin1String("EBADMSG"));
        keywords.append(QLatin1String("EBUSY"));
        keywords.append(QLatin1String("ECANCELED"));
        keywords.append(QLatin1String("ECHILD"));
        keywords.append(QLatin1String("EDEADLK"));
        keywords.append(QLatin1String("EDOM"));
        keywords.append(QLatin1String("EEXIST"));
        keywords.append(QLatin1String("EFAULT"));
        keywords.append(QLatin1String("EFBIG"));
        keywords.append(QLatin1String("EILSEQ"));
        keywords.append(QLatin1String("EINPROGRESS"));
        keywords.append(QLatin1String("EINTR"));
        keywords.append(QLatin1String("EINVAL"));
        keywords.append(QLatin1String("EIO"));
        keywords.append(QLatin1String("EISDIR"));
        keywords.append(QLatin1String("else"));
        keywords.append(QLatin1String("EMFILE"));
        keywords.append(QLatin1String("EMLINK"));
        keywords.append(QLatin1String("EMSGSIZE"));
        keywords.append(QLatin1String("ENAMETOOLONG"));
        keywords.append(QLatin1String("ENFILE"));
        keywords.append(QLatin1String("ENODEV"));
        keywords.append(QLatin1String("ENOENT"));
        keywords.append(QLatin1String("ENOEXEC"));
        keywords.append(QLatin1String("ENOLCK"));
        keywords.append(QLatin1String("ENOMEM"));
        keywords.append(QLatin1String("ENOSPC"));
        keywords.append(QLatin1String("ENOSYS"));
        keywords.append(QLatin1String("ENOTDIR"));
        keywords.append(QLatin1String("ENOTEMPTY"));
        keywords.append(QLatin1String("ENOTSUP"));
        keywords.append(QLatin1String("ENOTTY"));
        keywords.append(QLatin1String("enum"));
        keywords.append(QLatin1String("ENXIO"));
        keywords.append(QLatin1String("EOF"));
        keywords.append(QLatin1String("EPERM"));
        keywords.append(QLatin1String("EPIPE"));
        keywords.append(QLatin1String("ERANGE"));
        keywords.append(QLatin1String("EROFS"));
        keywords.append(QLatin1String("ESPIPE"));
        keywords.append(QLatin1String("ESRCH"));
        keywords.append(QLatin1String("ETIMEDOUT"));
        keywords.append(QLatin1String("EXDEV"));
        keywords.append(QLatin1String("EXIT_FAILURE"));
        keywords.append(QLatin1String("EXIT_SUCCESS"));
        keywords.append(QLatin1String("explicit"));
        keywords.append(QLatin1String("export"));
        keywords.append(QLatin1String("extern"));
        keywords.append(QLatin1String("false"));
        keywords.append(QLatin1String("__FILE__"));
        keywords.append(QLatin1String("FILE"));
        keywords.append(QLatin1String("FILENAME_MAX"));
        keywords.append(QLatin1String("float"));
        keywords.append(QLatin1String("FLT_DIG"));
        keywords.append(QLatin1String("FLT_EPSILON"));
        keywords.append(QLatin1String("FLT_MANT_DIG"));
        keywords.append(QLatin1String("FLT_MAX"));
        keywords.append(QLatin1String("FLT_MAX_10_EXP"));
        keywords.append(QLatin1String("FLT_MAX_EXP"));
        keywords.append(QLatin1String("FLT_MIN"));
        keywords.append(QLatin1String("FLT_MIN_10_EXP"));
        keywords.append(QLatin1String("FLT_MIN_EXP"));
        keywords.append(QLatin1String("FLT_RADIX"));
        keywords.append(QLatin1String("FLT_ROUNDS"));
        keywords.append(QLatin1String("FOPEN_MAX"));
        keywords.append(QLatin1String("for"));
        keywords.append(QLatin1String("fpos_t"));
        keywords.append(QLatin1String("friend"));
        keywords.append(QLatin1String("__FUNCTION__"));
        keywords.append(QLatin1String("__GNUC__"));
        keywords.append(QLatin1String("goto"));
        keywords.append(QLatin1String("HUGE_VAL"));
        keywords.append(QLatin1String("if"));
        keywords.append(QLatin1String("__imag__"));
        keywords.append(QLatin1String("inline"));
        keywords.append(QLatin1String("int"));
        keywords.append(QLatin1String("INT16_MAX"));
        keywords.append(QLatin1String("INT16_MIN"));
        keywords.append(QLatin1String("int16_t"));
        keywords.append(QLatin1String("INT32_MAX"));
        keywords.append(QLatin1String("INT32_MIN"));
        keywords.append(QLatin1String("int32_t"));
        keywords.append(QLatin1String("INT64_MAX"));
        keywords.append(QLatin1String("INT64_MIN"));
        keywords.append(QLatin1String("int64_t"));
        keywords.append(QLatin1String("INT8_MAX"));
        keywords.append(QLatin1String("INT8_MIN"));
        keywords.append(QLatin1String("int8_t"));
        keywords.append(QLatin1String("INT_FAST16_MAX"));
        keywords.append(QLatin1String("INT_FAST16_MIN"));
        keywords.append(QLatin1String("int_fast16_t"));
        keywords.append(QLatin1String("INT_FAST32_MAX"));
        keywords.append(QLatin1String("INT_FAST32_MIN"));
        keywords.append(QLatin1String("int_fast32_t"));
        keywords.append(QLatin1String("INT_FAST64_MAX"));
        keywords.append(QLatin1String("INT_FAST64_MIN"));
        keywords.append(QLatin1String("int_fast64_t"));
        keywords.append(QLatin1String("INT_FAST8_MAX"));
        keywords.append(QLatin1String("INT_FAST8_MIN"));
        keywords.append(QLatin1String("int_fast8_t"));
        keywords.append(QLatin1String("INT_LEAST16_MAX"));
        keywords.append(QLatin1String("INT_LEAST16_MIN"));
        keywords.append(QLatin1String("int_least16_t"));
        keywords.append(QLatin1String("INT_LEAST32_MAX"));
        keywords.append(QLatin1String("INT_LEAST32_MIN"));
        keywords.append(QLatin1String("int_least32_t"));
        keywords.append(QLatin1String("INT_LEAST64_MAX"));
        keywords.append(QLatin1String("INT_LEAST64_MIN"));
        keywords.append(QLatin1String("int_least64_t"));
        keywords.append(QLatin1String("INT_LEAST8_MAX"));
        keywords.append(QLatin1String("INT_LEAST8_MIN"));
        keywords.append(QLatin1String("int_least8_t"));
        keywords.append(QLatin1String("INT_MAX"));
        keywords.append(QLatin1String("INTMAX_MAX"));
        keywords.append(QLatin1String("INTMAX_MIN"));
        keywords.append(QLatin1String("intmax_t"));
        keywords.append(QLatin1String("INT_MIN"));
        keywords.append(QLatin1String("INTPTR_MAX"));
        keywords.append(QLatin1String("INTPTR_MIN"));
        keywords.append(QLatin1String("intptr_t"));
        keywords.append(QLatin1String("_IOFBF"));
        keywords.append(QLatin1String("_IOLBF"));
        keywords.append(QLatin1String("_IONBF"));
        keywords.append(QLatin1String("jmp_buf"));
        keywords.append(QLatin1String("__label__"));
        keywords.append(QLatin1String("LC_ALL"));
        keywords.append(QLatin1String("LC_COLLATE"));
        keywords.append(QLatin1String("LC_CTYPE"));
        keywords.append(QLatin1String("LC_MONETARY"));
        keywords.append(QLatin1String("LC_NUMERIC"));
        keywords.append(QLatin1String("LC_TIME"));
        keywords.append(QLatin1String("LDBL_DIG"));
        keywords.append(QLatin1String("LDBL_EPSILON"));
        keywords.append(QLatin1String("LDBL_MANT_DIG"));
        keywords.append(QLatin1String("LDBL_MAX"));
        keywords.append(QLatin1String("LDBL_MAX_10_EXP"));
        keywords.append(QLatin1String("LDBL_MAX_EXP"));
        keywords.append(QLatin1String("LDBL_MIN"));
        keywords.append(QLatin1String("LDBL_MIN_10_EXP"));
        keywords.append(QLatin1String("LDBL_MIN_EXP"));
        keywords.append(QLatin1String("ldiv_t"));
        keywords.append(QLatin1String("__LINE__"));
        keywords.append(QLatin1String("LLONG_MAX"));
        keywords.append(QLatin1String("long"));
        keywords.append(QLatin1String("LONG_MAX"));
        keywords.append(QLatin1String("LONG_MIN"));
        keywords.append(QLatin1String("L_tmpnam"));
        keywords.append(QLatin1String("M_1_PI"));
        keywords.append(QLatin1String("M_2_PI"));
        keywords.append(QLatin1String("M_2_SQRTPI"));
        keywords.append(QLatin1String("MB_CUR_MAX"));
        keywords.append(QLatin1String("MB_LEN_MAX"));
        keywords.append(QLatin1String("mbstate_t"));
        keywords.append(QLatin1String("M_E"));
        keywords.append(QLatin1String("M_LN10"));
        keywords.append(QLatin1String("M_LN2"));
        keywords.append(QLatin1String("M_LOG10E"));
        keywords.append(QLatin1String("M_LOG2E"));
        keywords.append(QLatin1String("M_PI"));
        keywords.append(QLatin1String("M_PI_2"));
        keywords.append(QLatin1String("M_PI_4"));
        keywords.append(QLatin1String("M_SQRT1_2"));
        keywords.append(QLatin1String("M_SQRT2"));
        keywords.append(QLatin1String("mutable"));
        keywords.append(QLatin1String("namespace"));
        keywords.append(QLatin1String("new"));
        keywords.append(QLatin1String("not"));
        keywords.append(QLatin1String("not_eq"));
        keywords.append(QLatin1String("NPOS"));
        keywords.append(QLatin1String("NULL"));
        keywords.append(QLatin1String("operator"));
        keywords.append(QLatin1String("or"));
        keywords.append(QLatin1String("or_eq"));
        keywords.append(QLatin1String("__PRETTY_FUNCTION__"));
        keywords.append(QLatin1String("private"));
        keywords.append(QLatin1String("protected"));
        keywords.append(QLatin1String("PTRDIFF_MAX"));
        keywords.append(QLatin1String("PTRDIFF_MIN"));
        keywords.append(QLatin1String("ptrdiff_t"));
        keywords.append(QLatin1String("public"));
        keywords.append(QLatin1String("RAND_MAX"));
        keywords.append(QLatin1String("__real__"));
        keywords.append(QLatin1String("register"));
        keywords.append(QLatin1String("reinterpret_cast"));
        keywords.append(QLatin1String("restrict"));
        keywords.append(QLatin1String("return"));
        keywords.append(QLatin1String("SCHAR_MAX"));
        keywords.append(QLatin1String("SCHAR_MIN"));
        keywords.append(QLatin1String("SEEK_CUR"));
        keywords.append(QLatin1String("SEEK_END"));
        keywords.append(QLatin1String("SEEK_SET"));
        keywords.append(QLatin1String("short"));
        keywords.append(QLatin1String("SHRT_MAX"));
        keywords.append(QLatin1String("SHRT_MIN"));
        keywords.append(QLatin1String("SIGABRT"));
        keywords.append(QLatin1String("SIGALRM"));
        keywords.append(QLatin1String("SIG_ATOMIC_MAX"));
        keywords.append(QLatin1String("SIG_ATOMIC_MIN"));
        keywords.append(QLatin1String("sig_atomic_t"));
        keywords.append(QLatin1String("SIGCHLD"));
        keywords.append(QLatin1String("SIGCONT"));
        keywords.append(QLatin1String("SIG_DFL"));
        keywords.append(QLatin1String("SIG_ERR"));
        keywords.append(QLatin1String("SIGFPE"));
        keywords.append(QLatin1String("SIGHUP"));
        keywords.append(QLatin1String("SIG_IGN"));
        keywords.append(QLatin1String("SIGILL"));
        keywords.append(QLatin1String("SIGINT"));
        keywords.append(QLatin1String("SIGKILL"));
        keywords.append(QLatin1String("signed"));
        keywords.append(QLatin1String("SIGPIPE"));
        keywords.append(QLatin1String("SIGQUIT"));
        keywords.append(QLatin1String("SIGSEGV"));
        keywords.append(QLatin1String("SIGSTOP"));
        keywords.append(QLatin1String("SIGTERM"));
        keywords.append(QLatin1String("SIGTRAP"));
        keywords.append(QLatin1String("SIGTSTP"));
        keywords.append(QLatin1String("SIGTTIN"));
        keywords.append(QLatin1String("SIGTTOU"));
        keywords.append(QLatin1String("SIGUSR1"));
        keywords.append(QLatin1String("SIGUSR2"));
        keywords.append(QLatin1String("SINT_MAX"));
        keywords.append(QLatin1String("SINT_MIN"));
        keywords.append(QLatin1String("SIZE_MAX"));
        keywords.append(QLatin1String("sizeof"));
        keywords.append(QLatin1String("size_t"));
        keywords.append(QLatin1String("SLONG_MAX"));
        keywords.append(QLatin1String("SLONG_MIN"));
        keywords.append(QLatin1String("SSHRT_MAX"));
        keywords.append(QLatin1String("SSHRT_MIN"));
        keywords.append(QLatin1String("ssize_t"));
        keywords.append(QLatin1String("static"));
        keywords.append(QLatin1String("constexpr"));
        keywords.append(QLatin1String("static_cast"));
        keywords.append(QLatin1String("__STDC__"));
        keywords.append(QLatin1String("__STDC_VERSION__"));
        keywords.append(QLatin1String("stderr"));
        keywords.append(QLatin1String("stdin"));
        keywords.append(QLatin1String("stdout"));
        keywords.append(QLatin1String("struct"));
        keywords.append(QLatin1String("switch"));
        keywords.append(QLatin1String("template"));
        keywords.append(QLatin1String("this"));
        keywords.append(QLatin1String("throw"));
        keywords.append(QLatin1String("__TIME__"));
        keywords.append(QLatin1String("time_t"));
        keywords.append(QLatin1String("TMP_MAX"));
        keywords.append(QLatin1String("true"));
        keywords.append(QLatin1String("try"));
        keywords.append(QLatin1String("typedef"));
        keywords.append(QLatin1String("typeid"));
        keywords.append(QLatin1String("typename"));
        keywords.append(QLatin1String("typeof"));
        keywords.append(QLatin1String("UCHAR_MAX"));
        keywords.append(QLatin1String("UINT16_MAX"));
        keywords.append(QLatin1String("uint16_t"));
        keywords.append(QLatin1String("UINT32_MAX"));
        keywords.append(QLatin1String("uint32_t"));
        keywords.append(QLatin1String("UINT64_MAX"));
        keywords.append(QLatin1String("uint64_t"));
        keywords.append(QLatin1String("UINT8_MAX"));
        keywords.append(QLatin1String("uint8_t"));
        keywords.append(QLatin1String("UINT_FAST16_MAX"));
        keywords.append(QLatin1String("uint_fast16_t"));
        keywords.append(QLatin1String("UINT_FAST32_MAX"));
        keywords.append(QLatin1String("uint_fast32_t"));
        keywords.append(QLatin1String("UINT_FAST64_MAX"));
        keywords.append(QLatin1String("uint_fast64_t"));
        keywords.append(QLatin1String("UINT_FAST8_MAX"));
        keywords.append(QLatin1String("uint_fast8_t"));
        keywords.append(QLatin1String("UINT_LEAST16_MAX"));
        keywords.append(QLatin1String("uint_least16_t"));
        keywords.append(QLatin1String("UINT_LEAST32_MAX"));
        keywords.append(QLatin1String("uint_least32_t"));
        keywords.append(QLatin1String("UINT_LEAST64_MAX"));
        keywords.append(QLatin1String("uint_least64_t"));
        keywords.append(QLatin1String("UINT_LEAST8_MAX"));
        keywords.append(QLatin1String("uint_least8_t"));
        keywords.append(QLatin1String("UINT_MAX"));
        keywords.append(QLatin1String("UINTMAX_MAX"));
        keywords.append(QLatin1String("uintmax_t"));
        keywords.append(QLatin1String("UINTPTR_MAX"));
        keywords.append(QLatin1String("uintptr_t"));
        keywords.append(QLatin1String("ULLONG_MAX"));
        keywords.append(QLatin1String("ULONG_MAX"));
        keywords.append(QLatin1String("union"));
        keywords.append(QLatin1String("unsigned"));
        keywords.append(QLatin1String("USHRT_MAX"));
        keywords.append(QLatin1String("using"));
        keywords.append(QLatin1String("va_list"));
        keywords.append(QLatin1String("virtual"));
        keywords.append(QLatin1String("void"));
        keywords.append(QLatin1String("__volatile__"));
        keywords.append(QLatin1String("volatile"));
        keywords.append(QLatin1String("WCHAR_MAX"));
        keywords.append(QLatin1String("WCHAR_MIN"));
        keywords.append(QLatin1String("wchar_t"));
        keywords.append(QLatin1String("wctrans_t"));
        keywords.append(QLatin1String("wctype_t"));
        keywords.append(QLatin1String("WEOF"));
        keywords.append(QLatin1String("while"));
        keywords.append(QLatin1String("WINT_MAX"));
        keywords.append(QLatin1String("WINT_MIN"));
        keywords.append(QLatin1String("wint_t"));
        keywords.append(QLatin1String("xor"));
        keywords.append(QLatin1String("xor_eq"));
    }

    return keywords;
}

/**
 * Add C++ stereotypes.
 */
void createCppStereotypes()
{
    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->findOrCreateStereotype(QLatin1String("constructor"));
    // declares an operation as friend
    umldoc->findOrCreateStereotype(QLatin1String("friend"));
    // to use in methods that aren't abstract
    umldoc->findOrCreateStereotype(QLatin1String("virtual"));
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

