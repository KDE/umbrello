/***************************************************************************
                          ParsedStruct.h  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _CPARSEDCLASS_H_INCLUDED
#define _CPARSEDCLASS_H_INCLUDED

#include "ParsedClassContainer.h"
#include "ParsedParent.h"
#include "ParsedSignalSlot.h"

/** This is the representation of a class that has been parsed by 
 * the classparser.
 * @author Jonas Nordin
 */
class CParsedClass : public CParsedClassContainer
{
public: // Constructor & Destructor

  CParsedClass();
  ~CParsedClass();

private: // Private attributes

  /** List of all slots. */
  QPtrList<CParsedMethod> slotList;

  /** All slots ordered by name and argument. */
  QDict<CParsedMethod> slotsByNameAndArg;

  /** List of all signals. */
  QPtrList<CParsedMethod> signalList;

  /** All signals ordered by name and argument. */
  QDict<CParsedMethod> signalsByNameAndArg;

public: // Public attributes

  /** List with names of parentclasses(if any). */
  QPtrList<CParsedParent> parents;

  /** List with names of frientclasses(if any). */
  QStrList friends;

  /** List of slots. */
  QPtrListIterator<CParsedMethod> slotIterator;

  /** List of signals. */
  QPtrListIterator<CParsedMethod> signalIterator;

  /** List of signal<->slot mappings. */
  QPtrList<CParsedSignalSlot> signalMaps;

  /** Tells if this class is declared inside another class. */
  bool isSubClass;
  
public: // Metods to set attribute values

  /** Remove all items in the store with references to the file.
   * @param aFile The file to check references to.
   */
  void removeWithReferences( const char *aFile );

  /** Remove a method matching the specification (from either 'methods'
   * or 'slotList').
   * @param aMethod Specification of the method.
   */
  void removeMethod( CParsedMethod *aMethod );

  /** Clear all attribute values.
   */
  void clearDeclaration();

  /** Add a parent.
   * @param aParent A parent of this class.
   */
  void addParent( CParsedParent *aParent );

  /** Add a friend. 
   * @param aName A friendclass of this class.
   */
  void addFriend( const char *aName )      { friends.append( aName ); }

  /** Add a signal. 
   * @param aMethod The signal to add.
   */
  void addSignal( CParsedMethod *aMethod );

  /** Add a slot. 
   * @param aMethod The slot to add.
   */
  void addSlot( CParsedMethod *aMethod );

  /** Add a signal->slot mapping. */
  void addSignalSlotMap( CParsedSignalSlot *aSS );

  /** 
   * Set the state if this is a subclass. 
   *
   * @param aState The new state.
   */
  inline void setIsSubClass( bool aState ) { isSubClass = aState; }

public: // Public queries

  /** Get a method by comparing with another method. 
   * @param aMethod Method to compare with.
   */
  CParsedMethod *getMethod( CParsedMethod &aMethod );

  /** Get a signal by using its' name and arguments. 
   * @param aName Name and arguments of the signal to fetch.
   */
  CParsedMethod *getSignalByNameAndArg( const char *aName );

  /** Get a slot by using its' name and arguments. 
   * @param aName Name and arguments of the slot to fetch.
   */
  CParsedMethod *getSlotByNameAndArg( const char *aName );

  /** Get all signals in sorted order. */
  QPtrList<CParsedMethod> *getSortedSignalList();

  /** Get all slots in sorted order. */
  QPtrList<CParsedMethod> *getSortedSlotList();

  /** Get all virtual methods. */
  QPtrList<CParsedMethod> *getVirtualMethodList();

  /** Check if this class has the named parent. 
   * @param aName Name of the parent to check.
   */
  bool hasParent( const char *aName );

  /** Check if the class has any virtual methods. */
  bool hasVirtual();

public: // Implementation of virtual methods

  /** Return a string made for persistant storage. 
   * @param str String to store the result in.
   * @return Pointer to str.
   */
  virtual QString asPersistantString( QString &dataStr );

  /** Initialize the object from a persistant string. 
   * @param str String to initialize from.
   * @param startPos Position(0-based) at which to start.
   */
  virtual int fromPersistantString( const char *dataStr, int startPos );

  /** Output the class as text on stdout. */
  void out();
};

#endif
