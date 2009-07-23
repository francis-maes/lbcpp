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
**@brief  #FIXME: all
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

  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumVariables() const = 0;

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual VariablePtr getVariable(size_t num) const = 0;

  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  virtual VariablePtr getVariable(const std::string& name) const = 0;

  /*!
  **
  **
  ** @param name
  **
  ** @return
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

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual std::string getVariableType(size_t num) const = 0;

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual std::string getVariableName(size_t num) const = 0;

  /*!
  **
  **
  ** @param num
  **
  ** @return
  */
  virtual std::string getVariableValue(size_t num) const = 0;

  /*
  ** Clone / assignment / swap
  */

  /*!
  **
  **
  **
  ** @return
  */
  virtual ObjectPtr clone() const = 0;

  /*!
  **
  **
  ** @param otherScope
  */
  virtual void setScope(const CRAlgorithmScope& otherScope) = 0;

  /*!
  **
  **
  ** @param otherScope
  */
  virtual void swapScope(CRAlgorithmScope& otherScope) = 0;

  /*
  ** Current State
  */

  /*!
  **
  **
  **
  ** @return
  */
  virtual int getState() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual CRAlgorithmScopePtr getCurrentInnerScope() = 0;

  /*
  ** To string
  */
  /*!
  **
  **
  **
  ** @return
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
