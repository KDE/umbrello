/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef DRIVER_H
#define DRIVER_H

#include "ast.h"
#include "position.h"

#include <QtCore/QPair>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QList>

#include <list>
#include <map>

class Lexer;
class Parser;

class Problem
{
public:
    enum
    {
        Level_Error = 0,
        Level_Warning,
        Level_Todo,
        Level_Fixme
    };

public:
    Problem() {}
    Problem( const Problem& source )
      : m_text( source.m_text ), m_position( source.m_position ),
        m_level( source.m_level ) {}
    Problem( const QString& text, Position const& position,
             int level = Level_Error )
      : m_text( text ), m_position( position ), m_level(level) {}

    Problem& operator = ( const Problem& source )
    {
        if (this != &source) {
            m_text = source.m_text;
            m_position = source.m_position;
            m_level = source.m_level;
        }
        return( *this );
    }

    bool operator == ( const Problem& p ) const
    {
        return ((m_text == p.m_text) && (m_position == p.m_position)
                && (m_level == p.m_level));
    }

    QString text() const { return m_text; }
    Position const& position() const { return m_position; }
    int level() const { return m_level; }

private:
    QString m_text;
    Position m_position;
    int m_level;
};

enum
  {
    Dep_Global,
    Dep_Local
  };

typedef QPair<QString, int> Dependence;

class Macro
{
public:
    typedef QString Argument;
    typedef std::list<Argument> ArgumentList;

    Macro() {}
    Macro( const QString &n) : m_name( n) {}
    Macro( const QString &n, const QString &b ) : m_name( n ), m_body( b ) {}

    Macro( const Macro& source )
      : m_name( source.m_name),
        m_fileName( source.m_fileName ),
        m_body( source.m_body ),
        m_arguments( source.m_arguments) {}

    Macro& operator = ( const Macro& source )
    {
        m_name = source.m_name;
        m_body = source.m_body;
        m_fileName = source.m_fileName;
        m_arguments = source.m_arguments;
        return *this;
    }

    bool operator == ( const Macro& source ) const
    {
        return
          m_name == source.m_name &&
          m_fileName == source.m_fileName &&
          m_body == source.m_body &&
          m_arguments == source.m_arguments;
    }

    QString const& name() const {return m_name;}
    void setName( const QString& name ) { m_name = name; }

    QString fileName() const { return m_fileName; }
    void setFileName( const QString& fileName ) { m_fileName = fileName; }

    QString body() const { return m_body; }
    void setBody( const QString& body ) { m_body = body; }

    bool hasArguments() const { return !m_arguments.empty();}
    ArgumentList const& arguments() const {return m_arguments;}

    void clearArgumentList() {m_arguments.clear();}
    void push_back( Argument const& argument) {m_arguments.push_back( argument);}

private:
    QString m_name;
    QString m_fileName;
    QString m_body;
    ArgumentList m_arguments;
};

class SourceProvider
{
public:
    SourceProvider() {}
    virtual ~SourceProvider() {}

    virtual QString contents( const QString& fileName ) = 0;
    virtual bool isModified( const QString& fileName ) = 0;

private:
    SourceProvider( const SourceProvider& source );
    void operator = ( const SourceProvider& source );
};

class Driver
{
  typedef std::map<QString, Macro> MacroMap;
public:
    Driver();
    virtual ~Driver();

    SourceProvider* sourceProvider();
    void setSourceProvider( SourceProvider* sourceProvider );

    virtual void reset();

    virtual void parseFile( const QString& fileName, bool onlyPreProcesss=false, bool force=false );
    virtual void fileParsed( const QString& fileName );
    virtual void addDependence( const QString& fileName, const Dependence& dep );
    virtual void addMacro( const Macro& macro) {m_macroManager.addMacro( macro);}
    virtual void addProblem( const QString& fileName, const Problem& problem );

    QString currentFileName() const { return m_currentFileName; }
    TranslationUnitAST::Node takeTranslationUnit( const QString& fileName );
    TranslationUnitAST* translationUnit( const QString& fileName ) const;
    QMap<QString, Dependence> dependences( const QString& fileName ) const;
private:
    MacroMap macros() const {return m_macroManager.macros();}
public:
    QList<Problem> problems( const QString& fileName ) const;
    bool hasMacro( const QString& name ) const {return m_macroManager.hasMacro( name);}
    Macro& macro( const QString& name) {return m_macroManager.macro( name);}
    virtual void removeMacro( const QString& macroName ) {m_macroManager.removeMacro( macroName);}
    QStringList includePaths() const { return m_includePaths; }
    virtual void addIncludePath( const QString &path );

    /// @todo remove
    const QMap<QString, TranslationUnitAST*> &parsedUnits() const { return m_parsedUnits; }

    virtual void setResolveDependencesEnabled( bool enabled );
    bool isResolveDependencesEnabled() const { return depresolv; }

protected:
    virtual void setupLexer( Lexer* lexer );
    virtual void setupParser( Parser* parser );
    virtual void setupPreProcessor();

private:
    QMap<QString, Dependence>& findOrInsertDependenceList( const QString& fileName );
    QList<Problem>& findOrInsertProblemList( const QString& fileName );
    QString findIncludeFile( const Dependence& dep ) const;

private:

    class MacroManager
    {
    public:
        void addMacro( const Macro& macro);
        bool hasMacro( const QString& name) const
        {return (m_macros.find( name) != m_macros.end());}
        Macro& macro( const QString& name) {return m_macros[ name];}
        MacroMap const& macros() const;
        void removeAllMacrosInFile( const QString& fileName);
        void removeMacro( const QString& macroName);
        void reset();
    private:
        MacroMap m_macros;
    };

    QString m_currentFileName;
    QMap< QString, QMap<QString, Dependence> > m_dependences;
    MacroManager m_macroManager;
    QMap< QString, QList<Problem> > m_problems;
    QMap<QString, TranslationUnitAST*> m_parsedUnits;
    QStringList m_includePaths;
    uint depresolv : 1;
    Lexer *lexer;
    SourceProvider* m_sourceProvider;

private:
    Driver( const Driver& source );
    void operator = ( const Driver& source );
};

#endif
