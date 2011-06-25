/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      David Hugh-Jones  <hughjonesd@yahoo.co.uk>    *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "perlwriter.h"

#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "operation.h"
#include "umldoc.h"
#include "uml.h"

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QTextStream>

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
    0
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
  findObjectsRelated(c,includes);

  QString AV = QChar('@');
  QString SV = QChar('$');
  QString HV = QChar('%');
  foreach (UMLPackage* conc, includes ) {
    if (conc->baseType() == UMLObject::ot_Datatype)
        continue;
    QString neatName = cleanName(conc->name());
    if (neatName != AV && neatName != SV && neatName != HV) {
      QString OtherPkgName =  conc->package(".");
      OtherPkgName.replace(QRegExp("\\."),"::");
      QString OtherName = OtherPkgName + "::" + cleanName(conc->name());

      // Only print out the use statement if the other package isn't the
      // same as the one we are working on. (This happens for the
      // "Singleton" design pattern.)
      if (OtherName != ThisPkgName){
        Ret += "use ";
        Ret += OtherName;
        Ret +=  ';';
        Ret += m_endl;
      }
    }
  }
  UMLClassifierList  superclasses = c->getSuperClasses();
  if (superclasses.count()) {
    Ret += m_endl;
    Ret += "use base qw( ";
    foreach (UMLClassifier *obj , superclasses ) {
      QString packageName =  obj->package(".");
      packageName.replace(QRegExp("\\."),"::");

      Ret += packageName + "::" + cleanName(obj->name()) + ' ';
    }
    Ret += ");" + m_endl;
  }

  return(true);
}

void PerlWriter::writeClass(UMLClassifier *c)
{
  /*  if (!c) {
      uDebug()<<"Cannot write class of NULL concept!";
      return;
      }
  */
  QString classname = cleanName(c->name());// this is fine: cleanName is "::-clean"
  QString packageName =  c->package(".");
  QString fileName;

  // Replace all white spaces with blanks
  packageName.simplified();

  // Replace all blanks with underscore
  packageName.replace(QRegExp(" "), "_");

  // Replace all dots (".") with double colon scope resolution operators
  // ("::")
  packageName.replace(QRegExp("\\."),"::");

  // Store complete package name
  QString ThisPkgName = packageName + "::" + classname;

  fileName = findFileName(c, ".pm");
  // the above lower-cases my nice class names. That is bad.
  // correct solution: refactor,
  // split massive findFileName up, reimplement
  // parts here
  // actual solution: shameful ".pm" hack in codegenerator

  CodeGenerationPolicy *pol = UMLApp::app()->commonPolicy();
  QString curDir = pol->getOutputDirectory().absolutePath();
  if (fileName.contains("::")) {
    // create new directories for each level
    QString newDir;
    newDir = curDir;
    QString fragment = fileName;
    QDir* existing = new QDir (curDir);
    QRegExp regEx("(.*)(::)");
    regEx.setMinimal(true);
    while (regEx.indexIn(fragment) > -1) {
      newDir = regEx.cap(1);
      fragment.remove(0, (regEx.pos(2) + 2)); // get round strange minimal matching bug
      existing->setPath(curDir + '/' + newDir);
      if (! existing->exists()) {
        existing->setPath(curDir);
        if (! existing->mkdir(newDir)) {
          emit codeGenerated(c, false);
          return;
        }
      }
      curDir += '/' + newDir;
    }
    fileName = fragment + ".pm";
  }
  if (fileName.isEmpty()) {
    emit codeGenerated(c, false);
    return;
  }
  QString oldDir = pol->getOutputDirectory().absolutePath();
  pol->setOutputDirectory(curDir);
  QFile fileperl;
  if (!openFile(fileperl, fileName)) {
    emit codeGenerated(c, false);
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

  str = getHeadingFile(".pm");   // what this mean?
  if (!str.isEmpty()) {
    str.replace(QRegExp("%filename%"),fileName);
    str.replace(QRegExp("%filepath%"),fileperl.fileName());
    str.replace(QRegExp("%year%"),QDate::currentDate().toString("yyyy"));
    str.replace(QRegExp("%date%"),QDate::currentDate().toString());
    str.replace(QRegExp("%time%"),QTime::currentTime().toString());
    str.replace(QRegExp("%package-name%"),ThisPkgName);
    if(str.indexOf(QRegExp("%PACKAGE-DECLARE%"))){
      str.replace(QRegExp("%PACKAGE-DECLARE%"),
                  "package " + ThisPkgName + ';'
                  + m_endl + m_endl
                  + "#UML_MODELER_BEGIN_PERSONAL_VARS_" + classname
                  + m_endl + m_endl
                  + "#UML_MODELER_END_PERSONAL_VARS_" + classname
                  + m_endl
                  );
      bPackageDeclared = true;
    }

    if (str.indexOf(QRegExp("%USE-STATEMENTS%"))){
      QString UseStms;
      if(GetUseStatements(c,UseStms,ThisPkgName)){
        str.replace(QRegExp("%USE-STATEMENTS%"), UseStms);
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
    if (GetUseStatements(c,UseStms,ThisPkgName)){
      perl<<UseStms<<m_endl;
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
  writeOperations(c,perl);

  perl << m_endl;

  //finish file
  //perl << m_endl << m_endl << "=cut" << m_endl;
  perl << m_endl << m_endl << "return 1;" << m_endl;

  //close files and notify we are done
  fileperl.close();
  emit codeGenerated(c, true);
}

Uml::ProgrammingLanguage PerlWriter::language() const
{
    return Uml::ProgrammingLanguage::Perl;
}

////////////////////////////////////////////////////////////////////////////////////
//  Helper Methods

void PerlWriter::writeOperations(UMLClassifier *c, QTextStream &perl)
{
    //Lists to store operations  sorted by scope
    UMLOperationList oppub,opprot,oppriv;

    //sort operations by scope first and see if there are abstract methods
    //keep this for documentation only!
    UMLOperationList opl(c->getOpList());
    foreach (UMLOperation *op , opl ) {
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
        writeOperations(classname,oppub,perl);
        perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    }

    if (forceSections() || !opprot.isEmpty()) {
        perl << m_endl << "=head1 METHODS FOR SUBCLASSING" << m_endl << m_endl ;
        //perl << "=pod "  << m_endl << m_endl << "=head3 " ;
        writeOperations(classname,opprot,perl);
        perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    }

    if (forceSections() || !oppriv.isEmpty()) {
        perl << m_endl << "=head1 PRIVATE METHODS" << m_endl << m_endl ;
        //perl << "=pod "  << m_endl << m_endl << "=head3 " ;
        writeOperations(classname,oppriv,perl);
        perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    }

    // moved here for perl
    if (!c->isInterface() && hasDefaultValueAttr(c)) {
        UMLAttributeList atl = c->getAttributeList();

        perl << m_endl;
        perl << m_endl << "=head2 _init" << m_endl << m_endl << m_endl;
        perl << "_init sets all " + classname + " attributes to their default values unless already set" << m_endl << m_endl << "=cut" << m_endl << m_endl;
        perl << "sub _init {" << m_endl << m_indentation << "my $self = shift;" << m_endl<<m_endl;

        foreach (UMLAttribute *at , atl ) {
            if (!at->getInitialValue().isEmpty())
                perl << m_indentation << "defined $self->{" << cleanName(at->name())<<"}"
                << " or $self->{" << cleanName(at->name()) << "} = "
                << at->getInitialValue() << ";" << m_endl;
        }

        perl << " }" << m_endl;
    }

    perl << m_endl << m_endl;
}

void PerlWriter::writeOperations(const QString &classname, UMLOperationList &opList, QTextStream &perl)
{
    Q_UNUSED(classname);
    foreach (UMLOperation* op , opList ) {
        UMLAttributeList atl = op->getParmList();
        //write method doc if we have doc || if at least one of the params has doc
        bool writeDoc = forceDoc() || !op->doc().isEmpty();
        foreach (UMLAttribute* at , atl )
            writeDoc |= !at->doc().isEmpty();

        if ( writeDoc )  //write method documentation
        {
            perl << "=pod "  << m_endl << m_endl << "=head3 " ;
            perl << cleanName(op->name()) << m_endl << m_endl;

            perl << "   Parameters :" << m_endl ;
          //write parameter documentation
          foreach (UMLAttribute* at , atl ) {
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
        foreach (UMLAttribute* at , atl ) {
          if (!bStartPrinted) {
              bStartPrinted = true;
              perl << "," << m_endl;
          }
          perl << "     $"<< cleanName(at->name()) << ", # "
               << at->getTypeName() << " : " << at->doc() << m_endl;
        }

        perl << "    ) = @_;" << m_endl;

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


void PerlWriter::writeAttributes(UMLClassifier *c, QTextStream &perl)
{
    UMLAttributeList  atpub, atprot, atpriv, atdefval;

    //sort attributes by scope and see if they have a default value
    UMLAttributeList atl = c->getAttributeList();

    foreach (UMLAttribute* at , atl ) {
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
        writeAttributes(atpub,perl);
    }
    /* not needed as writeAttributes only writes documentation
    if (forceSections() || atprot.count()) {
    writeAttributes(atprot,perl);
    }

    if (forceSections() || atpriv.count()) {
    writeAttributes(atpriv,perl);
    }
    */
}

void PerlWriter::writeAttributes(UMLAttributeList &atList, QTextStream &perl)
{
    perl << m_endl << "=head1 PUBLIC ATTRIBUTES" << m_endl << m_endl;
    perl << "=pod "  << m_endl << m_endl ;
    foreach (UMLAttribute *at , atList ) {
        if (forceDoc() || !at->doc().isEmpty())
        {
            perl  << "=head3 " << cleanName(at->name()) << m_endl << m_endl ;
            perl  << "   Description : " << at->doc() << m_endl << m_endl;
        }
    } // end for
    perl << m_endl << m_endl << "=cut" << m_endl << m_endl;
    return;
}

QStringList PerlWriter::defaultDatatypes()
{
    QStringList l;
    l.append("$");
    l.append("@");
    l.append("%");
    return l;
}

QStringList PerlWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        for (int i = 0; reserved_words[i]; ++i) {
            keywords.append(reserved_words[i]);
        }
    }

    return keywords;
}

#include "perlwriter.moc"
