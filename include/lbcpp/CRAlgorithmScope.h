/*
** $PROJECT_PRESENTATION_AND_CONTACT_INFOS$
**
** Copyright (C) 2009 Francis MAES
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
| Filename: CRAlgorithmScope.h             | CR-algorithm scope base class   |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   CRAlgorithmScope.h
**@author Francis MAES
**@date   Fri Jun 12 16:58:16 2009
**
**@brief  CRAlgorithmScope class declaration.
**
**
*/

#ifndef LBCPP_CRALGORITHM_SCOPE_H_
# define LBCPP_CRALGORITHM_SCOPE_H_

# include "Variable.h"

namespace lbcpp
{

/*!
** @class CRAlgorithmScope
** @brief
*/
class CRAlgorithmScope : public Object
{
public:
  /*
  ** Introspection
  */

  /**
  ** Returns the number of variables into the CRAlgorithm.
  **
  ** @return the number of variables into the CRAlgorithm.
  */
  virtual size_t getNumVariables() const = 0;

  /**
  ** Returns the variable number @a num.
  **
  ** @param num : number of the variable.
  **
  ** @return the variable number @a num.
  */
  virtual VariablePtr getVariable(size_t num) const = 0;

  /**
  ** Returns the variable @a name.
  **
  ** @param name : name of the variable.
  **
  ** @return the variable @a name.
  */
  virtual VariablePtr getVariable(const std::string& name) const = 0;

  /**
  ** Returns a reference on the variable @a name.
  **
  ** @param name : name of the variable.
  **
  ** @return a reference on the variable @a name.
  */
  template<class T>
  const T& getVariableReference(const std::string& name) const
  {
    VariablePtr v = getVariable(name);
    if (!v)
    {
      Object::error("CRAlgorithmScope::getVariableReference", "Could not find variable called '" + name + "'");
      assert(false);
    }
    return v->getConstReference<T>();
  }

  /**
  ** Returns the type of the variable number @a num.
  **
  ** @param num : number of the variable.
  **
  ** @return the type of the variable number @a num.
  */
  virtual std::string getVariableType(size_t num) const = 0;

  /**
  ** Returns the name of the variable number @a num.
  **
  ** @param num : number of the variable.
  **
  ** @return the name of the variable number @a num.
  */
  virtual std::string getVariableName(size_t num) const = 0;

  /**
  ** Returns the value of the variable number @a num.
  **
  ** @param num : number of the variable.
  **
  ** @return the value of the variable number @a num.
  */
  virtual std::string getVariableValue(size_t num) const = 0;

  /*
  ** Clone / assignment / swap
  */

  /**
  ** Clones the current state of the CRAlgorithm.
  **
  ** @return a copy of the current state of the CRAlgorithm.
  */
  virtual ObjectPtr clone() const = 0;

  /**
  ** Sets the current CRAlgorithm scope to another CRAlgorithm scope.
  **
  ** @param otherScope : another CRAlgorithm scope.
  */
  virtual void setScope(const CRAlgorithmScope& otherScope) = 0;

  /**
  ** Swaps the current CRAlgorithm scope with @a otherScope.
  **
  ** @param otherScope : another CRAlgorithm scope.
  */
  virtual void swapScope(CRAlgorithmScope& otherScope) = 0;

  /*
  ** Current State
  */

  /**
  **
  **
  **
  ** @return
  */
  virtual int getState() const = 0;

  /**
  ** Returns a pointer on the current inner CRAlgorithm scope.
  **
  ** @return a pointer on the current inner CRAlgorithm scope.
  */
  virtual CRAlgorithmScopePtr getCurrentInnerScope() = 0;

  /*
  ** To string
  */
  /**
  ** Converts the current CRAlgorithm state to a string, ie the state of all the variables.
  **
  ** @return the state of all the variables ("variable name = variable value\n...").
  */
  virtual std::string toString() const
  {
    std::string res;
    for (size_t i = 0; i < getNumVariables(); ++i)
      res += getVariableName(i) + " = " + getVariableValue(i) + "\n";
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_SCOPE_H_
