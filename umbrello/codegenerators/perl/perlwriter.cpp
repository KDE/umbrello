/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 David Hugh-Jones <hughjonesd@yahoo.co.uk>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "perlwriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "operation.h"
#include "umldoc.h"
#include "uml.h"

#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

static const char *reserved_words[] = {
    "abs",
    "accept",
    "alarm",
    "and",
    "atan2",
    "BEGIN",
    "bind",
    "binmode",
    "bless",
    "byte",
    "caller",
    "carp",
    "chdir",
    "chmod",
    "chomp",
    "chop",
    "chown",
    "chr",
    "chroot",
    "close",
    "closedir",
    "cmp",
    "confess",
    "connect",
    "continue",
    "cos",
    "croak",
    "crypt",
    "dbmclose",
    "dbmopen",
    "defined",
    "delete",
    "die",
    "do",
    "dump",
    "each",
    "else",
    "elsif",
    "END",
    "endgrent",
    "endhostent",
    "endnetent",
    "endprotoent",
    "endpwent",
    "endservent",
    "eof",
    "eq",
    "eval",
    "exec",
    "exists",
    "exit",
    "exp",
    "fcntl",
    "fileno",
    "flock",
    "for",
    "foreach",
    "fork",
    "format",
    "formline",
    "ge",
    "getc",
    "getgrent",
    "getgrgid",
    "getgrnam",
    "gethostbyaddr",
    "gethostbyname",
    "gethostent",
    "getlogin",
    "getnetbyaddr",
    "getnetbyname",
    "getnetent",
    "getpeername",
    "getpgrp",
    "getppid",
    "getpriority",
    "getprotobyname",
    "getprotobynumber",
    "getprotoent",
    "getpwent",
    "getpwnam",
    "getpwuid",
    "getservbyname",
    "getservbyport",
    "getservent",
    "getsockname",
    "getsockopt",
    "glob",
    "gmtime",
    "goto",
    "grep",
    "gt",
    "hex",
    "if",
    "import",
    "index",
    "int",
    "integer",
    "ioctl",
    "join",
    "keys",
    "kill",
    "last",
    "lc",
    "lcfirst",
    "le",
    "length",
    "lib",
    "link",
    "listen",
    "local",
    "localtime",
    "lock",
    "log",
    "lstat",
    "lt",
    "map",
    "mkdir",
    "msgctl",
    "msgget",
    "msgrcv",
    "msgsnd",
    "my",
    "ne",
    "new",
    "next",
    "no",
    "not",
    "oct",
    "open",
    "opendir",
    "or",
    "ord",
    "our",
    "pack",
    "package",
    "pipe",
    "pop",
    "pos",
    "print",
    "printf",
    "prototype",
    "push",
    "quotemeta",
    "rand",
    "read",
    "readdir",
    "readline",
    "readlink",
    "readpipe",
    "recv",
    "redo",
    "ref",
    "rename",
    "require",
    "reset",
    "return",
    "reverse",
    "rewinddir",
    "rindex",
    "rmdir",
    "scalar",
    "seek",
    "seekdir",
    "select",
    "semctl",
    "semget",
    "semop",
    "send",
    "setgrent",
    "sethostent",
    "setnetent",
    "setpgrp",
    "setpriority",
    "setprotoent",
    "setpwent",
    "setservent",
    "setsockopt",
    "shift",
    "shmctl",
    "shmget",
    "shmread",
    "shmwrite",
    "shutdown",
    "sigtrap",
    "sin",
    "sleep",
    "socket",
    "socketpair",
    "sort",
    "splice",
    "split",
    "sprintf",
    "sqrt",
    "srand",
    "stat",
    "strict",
    "study",
    "sub",
    "subs",
    "substr",
    "switch",
    "symlink",
    "syscall",
    "sysopen",
    "sysread",
    "sysseek",
    "system",
    "syswrite",
    "tell",
    "telldir",
    "tie",
    "tied",
    "time",
    "times",
    "truncate",
    "uc",
    "ucfirst",
    "umask",
    "undef",
    "unless",
    "unlink",
    "unpack",
    "unshift",
    "untie",
    "until",
    "use",
    "utf8",
    "utime",
    "values",
    "vars",
    "vec",
    "wait",
    "waitpid",
    "wantarray",
    "warn",
    "warnings",
    "while",
    "write",
    "xor",
    nullptr
};

PerlWriter::PerlWriter()
{
}

PerlWriter::~PerlWriter()
{
}

bool PerlWriter::GetUseStatements(UMLClassifier *c, QString &Ret,
                                  QString &ThisPkgName)
{
  if (!c){
    return(false);
  }

  UMLPackageList includes;
  findObjectsRelated(c, includes);

  QString AV = QChar(QLatin1Char('@'));
  QString SV = QChar(QLatin1Char('$'));
  QString HV = QChar(QLatin1Char('%'));
  for(UMLPackage *conc : includes) {
    if (conc->isUMLDatatype())
        continue;
    QString neatName = cleanName(conc->name());
    if (neatName != AV && neatName != SV && neatName != HV) {
      QString OtherPkgName =  conc->package(QStringLiteral("."));
      OtherPkgName.replace(QRegularExpression(QStringLiteral("\\.")), QStringLiteral("::"));
      QString OtherName = OtherPkgName + QStringLiteral("::") + cleanName(conc->name());

      // Only print out the use statement if the other package isn't the
      // same as the one we are working on. (This happens for the
      // "Singleton" design pattern.)
      if (OtherName != ThisPkgName){
        Ret += QStringLiteral("use ");
        Ret += OtherName;
        Ret +=  QLatin1Char(';');
        Ret += m_endl;
      }
    }
  }
  UMLClassifierList  superclasses = c->getSuperClasses();
  if (superclasses.count()) {
    Ret += m_endl;
    Ret += QStringLiteral("use base qw(");
    for(UMLClassifier  *obj : superclasses) {
      QString packageName =  obj->package(QStringLiteral("."));
      packageName.replace(QRegularExpression(QStringLiteral("\\.")), QStringLiteral("::"));

      Ret += packageName + QStringLiteral("::") + cleanName(obj->name()) + QLatin1Char(' ');
    }
    Ret += QStringLiteral(");") + m_endl;
  }

  return(true);
}

/**
 * Call this method to generate Perl code for a UMLClassifier.
 * @param c   the class you want to generate code for
 */
void PerlWriter::writeClass(UMLClassifier *c)
{
  if (!c) {
      logWarn0("PerlWriter::writeClass: Cannot write class of NULL classifier");
      return;
  }
  QString classname = cleanName(c->name());// this is fine: cleanName is "::-clean"
  QString packageName =  c->package(QStringLiteral("."));
  QString fileName;

  // Replace all white spaces with blanks
  packageName = packageName.simplified();

  // Replace all blanks with underscore
  packageName.replace(QRegularExpression(QStringLiteral(" ")), QStringLiteral("_"));

  // Replace all dots (".") with double colon scope resolution operators
  // ("::")
  packageName.replace(QRegularExpression(QStringLiteral("\\.")), QStringLiteral("::"));

  // Store complete package name
  QString ThisPkgName = packageName + QStringLiteral("::") + classname;

  fileName = findFileName(c, QStringLiteral(".pm"));
  // the above lower-cases my nice class names. That is bad.
  // correct solution: refactor,
  // split massive findFileName up, reimplement
  // parts here
  // actual solution: shameful ".pm" hack in codegenerator

  CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();
  QString curDir = pol->getOutputDirectory().absolutePath();
  if (fileName.contains(QStringLiteral("::"))) {
    // create new directories for each level
    QString newDir;
    newDir = curDir;
    QString fragment = fileName;
    QDir* existing = new QDir (curDir);
    QRegularExpression regEx(QStringLiteral("(.*)(::)"));
    QRegularExpressionMatch regMat = regEx.match(fragment);
    
    while (fragment.indexOf(regEx) > -1) {
      newDir = regMat.captured(1);
      fragment.remove(0, (regMat.capturedStart(2) + 2)); // get round strange minimal matching bug
      existing->setPath(curDir + QLatin1Char('/') + newDir);
      if (! existing->exists()) {
        existing->setPath(curDir);
        if (! existing->mkdir(newDir)) {
          Q_EMIT codeGenerated(c, false);
          return;
        }
      }
      curDir += QLatin1Char('/') + newDir;
    }
    fileName = fragment + QStringLiteral(".pm");
  }
  if (fileName.isEmpty()) {
    Q_EMIT codeGenerated(c, false);
    return;
  }
  QString oldDir = pol->getOutputDirectory().absolutePath();
  pol->setOutputDirectory(curDir);
  QFile fileperl;
  if (!openFile(fileperl, fileName)) {
    Q_EMIT codeGenerated(c, false);
    return;
  }
  QTextStream perl(&fileperl);
  pol->setOutputDirectory(oldDir);

  //======================================================================
  // Start generating the code!!
  //======================================================================

  // try to find a heading file (license, comments, etc)
  QString str;
  bool bPackageDeclared = false;
  bool bUseStmsWritten  = false;

  str = getHeadingFile(QStringLiteral(".pm"));   // what this mean?
  if (!str.isEmpty()) {
    str.replace(QRegularExpression(QStringLiteral("%filename%")), fileName);
    str.replace(QRegularExpression(QStringLiteral("%filepath%")), fileperl.fileName());
    str.replace(QRegularExpression(QStringLiteral("%year%")), QDate::currentDate().toString(QStringLiteral("yyyy")));
    str.replace(QRegularExpression(QStringLiteral("%date%")), QDate::currentDate().toString());
    str.replace(QRegularExpression(QStringLiteral("%time%")), QTime::currentTime().toString());
    str.replace(QRegularExpression(QStringLiteral("%package-name%")), ThisPkgName);
    if(str.indexOf(QRegularExpression(QStringLiteral("%PACKAGE-DECLARE%")))){
      str.replace(QRegularExpression(QStringLiteral("%PACKAGE-DECLARE%")),
                  QStringLiteral("package ") + ThisPkgName + QLatin1Char(';')
                  + m_endl + m_endl
                  + QStringLiteral("#UML_MODELER_BEGIN_PERSONAL_VARS_") + classname
                  + m_endl + m_endl
                  + QStringLiteral("#UML_MODELER_END_PERSONAL_VARS_") + classname
                  + m_endl
                 );
      bPackageDeclared = true;
    }

    if (str.indexOf(QRegularExpression(QStringLiteral("%USE-STATEMENTS%")))){
      QString UseStms;
      if(GetUseStatements(c, UseStms, ThisPkgName)){
        str.replace(QRegularExpression(QStringLiteral("%USE-STATEMENTS%")), UseStms);
        bUseStmsWritten = true;
      }
    }

    perl << str << m_endl;
  }

  // if the package wasn't declared above during keyword substitution,
  // add it now. (At the end of the file.)
  if (! bPackageDeclared){
    perl << m_endl << m_endl << "package " <<ThisPkgName << ";" << m_endl
         << m_endl;
    //write includes
    perl << m_endl << "#UML_MODELER_BEGIN_PERSONAL_VARS_" << classname
         << m_endl ;
    perl << m_endl << "#UML_MODELER_END_PERSONAL_VARS_" << classname
         << m_endl << m_endl ;
  }

  if (! bUseStmsWritten){
    QString UseStms;
    if (GetUseStatements(c, UseStms, ThisPkgName)){
      perl << UseStms << m_endl;
    }
  }

  perl << m_endl;

  // Do we really need these for anything???
  UMLAssociationList aggregations = c->getAggregations();
  UMLAssociationList compositions = c->getCompositions();

    //Write class Documentation
  if (forceDoc() || !c->doc().isEmpty()) {
    perl << m_endl << "=head1";
    perl << " " << classname.toUpper() << m_endl << m_endl;
    perl << c->doc();
    perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
  }

  //check if class is abstract and / or has abstract methods
  if (c->isAbstract())
    perl << "=head1 ABSTRACT CLASS" << m_endl << m_endl << "=cut" << m_endl;

  //attributes
  if (! c->isInterface())
    writeAttributes(c, perl);      // keep for documentation's sake

  //operations
  writeOperations(c, perl);

  perl << m_endl;

  //finish file
  //perl << m_endl << m_endl << "=cut" << m_endl;
  perl << m_endl << m_endl << "return 1;" << m_endl;

  //close files and notify we are done
  fileperl.close();
  Q_EMIT codeGenerated(c, true);
  Q_EMIT showGeneratedFile(fileperl.fileName());
}

/**
 * Returns "Perl".
 * @return   the programming language identifier
 */
Uml::ProgrammingLanguage::Enum PerlWriter::language() const
{
    return Uml::ProgrammingLanguage::Perl;
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

/**
 * Write all operations for a given class.
 * @param c      the classifier we are generating code for
 * @param perl   output stream for the Perl file
 */
void PerlWriter::writeOperations(UMLClassifier *c, QTextStream &perl)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub, opprot, oppriv;

    //sort operations by scope first and see if there are abstract methods
    //keep this for documentation only!
    UMLOperationList opl(c->getOpList());
    for(UMLOperation  *op : opl) {
        switch(op->visibility()) {
          case Uml::Visibility::Public:
            oppub.append(op);
            break;
          case Uml::Visibility::Protected:
            opprot.append(op);
            break;
          case Uml::Visibility::Private:
            oppriv.append(op);
            break;
          default:
            break;
        }
    }

    QString classname(cleanName(c->name()));

    //write operations to file
    if (forceSections() || !oppub.isEmpty()) {
        perl << m_endl << "=head1 PUBLIC METHODS" << m_endl << m_endl ;
        writeOperations(classname, oppub, perl);
        perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    }

    if (forceSections() || !opprot.isEmpty()) {
        perl << m_endl << "=head1 METHODS FOR SUBCLASSING" << m_endl << m_endl ;
        //perl << "=pod "  << m_endl << m_endl << "=head3 " ;
        writeOperations(classname, opprot, perl);
        perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    }

    if (forceSections() || !oppriv.isEmpty()) {
        perl << m_endl << "=head1 PRIVATE METHODS" << m_endl << m_endl ;
        //perl << "=pod "  << m_endl << m_endl << "=head3 " ;
        writeOperations(classname, oppriv, perl);
        perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    }

    // moved here for perl
    if (!c->isInterface() && hasDefaultValueAttr(c)) {
        UMLAttributeList atl = c->getAttributeList();

        perl << m_endl;
        perl << m_endl << "=head2 _init" << m_endl << m_endl << m_endl;
        perl << "_init sets all " << classname << " attributes to their default values unless already set" << m_endl << m_endl << "=cut" << m_endl << m_endl;
        perl << "sub _init {" << m_endl << m_indentation << "my $self = shift;" << m_endl << m_endl;

        for(UMLAttribute  *at : atl) {
            if (!at->getInitialValue().isEmpty())
                perl << m_indentation << "defined $self->{" << cleanName(at->name()) << "}"
                << " or $self->{" << cleanName(at->name()) << "} = "
                << at->getInitialValue() << ";" << m_endl;
        }

        perl << " }" << m_endl;
    }

    perl << m_endl << m_endl;
}

/**
 * Write a list of class operations.
 * @param classname   the name of the class
 * @param opList      the list of operations
 * @param perl        output stream for the Perl file
 */
void PerlWriter::writeOperations(const QString &classname, UMLOperationList &opList, QTextStream &perl)
{
    Q_UNUSED(classname);
    for(UMLOperation *op : opList) {
        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        for(UMLAttribute *at : atl)
            writeDoc |= !at->doc().isEmpty();

        if (writeDoc)  //write method documentation
        {
            perl << "=pod "  << m_endl << m_endl << "=head3 " ;
            perl << cleanName(op->name()) << m_endl << m_endl;

            perl << "   Parameters :" << m_endl ;
          //write parameter documentation
          for(UMLAttribute *at : atl) {
            if (forceDoc() || !at->doc().isEmpty()) {
              perl << "      "
                   << cleanName(at->name()) << " : "
                   << at->getTypeName() << " : "
                   << at->doc()
                   << m_endl;
                }
            }//end for : write parameter documentation

            perl << m_endl;
            perl << "   Return : " << m_endl;
            perl << "      " << op->getTypeName();
            perl << m_endl << m_endl;
            perl << "   Description : " << m_endl;
            perl << "      " << op->doc();
            perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
        }//end if : write method documentation

        perl <<  "sub " << cleanName(op->name()) << m_endl << "{" << m_endl;
        perl << "  my($self";

        bool bStartPrinted = false;
        //write parameters
        for(UMLAttribute *at : atl) {
          if (!bStartPrinted) {
              bStartPrinted = true;
              perl << "," << m_endl;
          }
          perl << "     $" <<  cleanName(at->name()) << ", # "
               << at->getTypeName() << " : " << at->doc() << m_endl;
        }

        perl << "   ) = @_;" << m_endl;

        perl << "#UML_MODELER_BEGIN_PERSONAL_CODE_" << cleanName(op->name()) << m_endl;
        QString sourceCode = op->getSourceCode();
        if (!sourceCode.isEmpty()) {
            perl << formatSourceCode(sourceCode, m_indentation);
        }
        perl << "#UML_MODELER_END_PERSONAL_CODE_" << cleanName(op->name()) << m_endl;
        perl << "}" << m_endl;
        perl << m_endl << m_endl;
    }//end for
}

/**
 * Write all the attributes of a class.
 * @param c      the class we are generating code for
 * @param perl   output stream for the Perl file
 */
void PerlWriter::writeAttributes(UMLClassifier *c, QTextStream &perl)
{
    UMLAttributeList  atpub, atprot, atpriv, atdefval;

    //sort attributes by scope and see if they have a default value
    UMLAttributeList atl = c->getAttributeList();

    for(UMLAttribute *at : atl) {
        if (!at->getInitialValue().isEmpty())
            atdefval.append(at);
        switch(at->visibility()) {
          case Uml::Visibility::Public:
            atpub.append(at);
            break;
          case Uml::Visibility::Protected:
            atprot.append(at);
            break;
          case Uml::Visibility::Private:
            atpriv.append(at);
            break;
          default:
            break;
        }
    }

    if (forceSections() || atpub.count()) {
        writeAttributes(atpub, perl);
    }
    /* not needed as writeAttributes only writes documentation
    if (forceSections() || atprot.count()) {
    writeAttributes(atprot, perl);
    }

    if (forceSections() || atpriv.count()) {
    writeAttributes(atpriv, perl);
    }
    */
}

/**
 * Write a list of class attributes.
 * @param atList   the list of attributes
 * @param perl     output stream for the Perl file
 */
void PerlWriter::writeAttributes(UMLAttributeList &atList, QTextStream &perl)
{
    perl << m_endl << "=head1 PUBLIC ATTRIBUTES" << m_endl << m_endl;
    perl << "=pod "  << m_endl << m_endl ;
    for(UMLAttribute  *at : atList) {
        if (forceDoc() || !at->doc().isEmpty())
        {
            perl  << "=head3 " << cleanName(at->name()) << m_endl << m_endl ;
            perl  << "   Description : " << at->doc() << m_endl << m_endl;
        }
    } // end for
    perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    return;
}

/**
 * Get list of default datatypes.
 * @return   the list of default datatypes
 */
QStringList PerlWriter::defaultDatatypes() const
{
    QStringList l;
    l.append(QStringLiteral("$"));
    l.append(QStringLiteral("@"));
    l.append(QStringLiteral("%"));
    return l;
}

/**
 * Get list of reserved keywords.
 * @return   the list of reserved keywords
 */
QStringList PerlWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(QLatin1String(reserved_words[i]));
        }
    }

    return keywords;
}

