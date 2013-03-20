/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: Pair.h                         | Pair Object                     |
| Author  : Francis Maes                   |                                 |
| Started : 06/10/2010 15:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_CORE_PAIR_H_
# define OIL_CORE_PAIR_H_

# include "Object.h"

namespace lbcpp
{
  
extern ClassPtr pairClass(ClassPtr firstClass, ClassPtr secondClass);

class Pair : public Object
{
public:
  Pair(ClassPtr thisClass)
    : Object(thisClass) {}
  Pair(ClassPtr thisClass, const ObjectPtr& first, const ObjectPtr& second)
    : Object(thisClass), first(first), second(second) {}
  Pair(const ObjectPtr& first, const ObjectPtr& second)
    : Object(pairClass(first ? first->getClass() : objectClass, second ? second->getClass() : objectClass)), first(first), second(second) {}
  Pair(const std::pair<ObjectPtr, ObjectPtr>& pair)
    : Object(pairClass(pair.first ? pair.first->getClass() : objectClass, pair.second ? pair.second->getClass() : objectClass)), first(pair.first), second(pair.second) {}
  Pair() {}
  
  virtual string toString() const
    {return JUCE_T("(") + first->toString() + JUCE_T(", ") + second->toString() + JUCE_T(")");}

  virtual string toShortString() const
    {return JUCE_T("(") + first->toShortString() + JUCE_T(", ") + second->toShortString() + JUCE_T(")");}
 
  virtual int compare(const ObjectPtr& otherObject) const
    {return compareVariables(otherObject);}

  virtual ObjectPtr clone(ExecutionContext& context) const
    {return new Pair(thisClass, first, second);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {PairPtr targetPair = target.staticCast<Pair>(); targetPair->first = first; targetPair->second = second;}
 
  const ObjectPtr& getFirst() const
    {return first;}

  const ObjectPtr& getSecond() const
    {return second;}
  
  void setFirst(const ObjectPtr& v)
    {first = v;}
  
  void setSecond(const ObjectPtr& v)
    {second = v;}

  std::pair<ObjectPtr, ObjectPtr> getValue() const
    {return std::make_pair(first, second);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PairClass;

  ObjectPtr first;
  ObjectPtr second;
};

}; /* namespace lbcpp */

#endif // !OIL_CORE_PAIR_H_
