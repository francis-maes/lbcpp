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

#ifndef LBCPP_CORE_PAIR_H_
# define LBCPP_CORE_PAIR_H_

# include "Variable.h"

namespace lbcpp
{

class Pair : public Object
{
public:
  Pair(ClassPtr thisClass)
    : Object(thisClass) {}
  Pair(ClassPtr thisClass, const Variable& first, const Variable& second)
    : Object(thisClass), first(first), second(second) {}
  Pair(const Variable& first, const Variable& second)
    : Object(pairClass(first.getType(), second.getType())), first(first), second(second) {}
  Pair(const std::pair<Variable, Variable>& pair)
    : Object(pairClass(pair.first.getType(), pair.second.getType())), first(pair.first), second(pair.second) {}
  Pair() {}
  
  virtual String toString() const
    {return T("(") + first.toString() + T(", ") + second.toString() + T(")");}

  virtual String toShortString() const
    {return T("(") + first.toShortString() + T(", ") + second.toShortString() + T(")");}
 
  virtual int compare(ObjectPtr otherObject) const
    {return compareVariables(otherObject);}

  virtual ObjectPtr clone(ExecutionContext& context) const
    {return new Pair(thisClass, first, second);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {PairPtr targetPair = target.staticCast<Pair>(); targetPair->first = first; targetPair->second = second;}
 
  const Variable& getFirst() const
    {return first;}

  const Variable& getSecond() const
    {return second;}

  Variable& getFirst()
    {return first;}

  Variable& getSecond()
    {return second;}
  
  void setFirst(const Variable& v)
    {first = v;}
  
  void setSecond(const Variable& v)
    {second = v;}

  std::pair<Variable, Variable> getValue() const
    {return std::make_pair(first, second);}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class PairClass;

  Variable first;
  Variable second;
};


template<class T1, class T2>
inline void variableToNative(ExecutionContext& context, std::pair<T1, T2>& dest, const Variable& source)
{
  jassert(source.isObject());
  const PairPtr& sourcePair = source.getObjectAndCast<Pair>(context);
  if (sourcePair)
  {
    lbcpp::variableToNative(context, dest.first, sourcePair->getFirst());
    lbcpp::variableToNative(context, dest.second, sourcePair->getSecond());
  }
  else
  {
    dest.first = T1();
    dest.second = T2();
  }
}

template<class T1, class T2>
inline void nativeToVariable(Variable& dest, const std::pair<T1, T2>& source, TypePtr expectedType)
{
  jassert(expectedType->getNumTemplateArguments() == 2);
  dest = Variable::create(expectedType);
  const ObjectPtr& destObject = dest.getObject();
  const PairPtr& destPair = destObject.staticCast<Pair>();
  jassert(destPair);
  nativeToVariable(destPair->getFirst(), source.first, expectedType->getTemplateArgument(0));
  nativeToVariable(destPair->getSecond(), source.second, expectedType->getTemplateArgument(1));
}

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_PAIR_H_
