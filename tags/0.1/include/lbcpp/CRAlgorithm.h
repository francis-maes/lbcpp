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
| Filename: CRAlgorithm.h                  | CR-algorithm base class         |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/



/*!
**@file   CRAlgorithm.h
**@author Francis MAES
**@date   Fri Jun 12 16:53:50 2009
**
**@brief  CRAlgorithm class declaration.
**
**
*/

#ifndef LBCPP_CRALGORITHM_H_
# define LBCPP_CRALGORITHM_H_

# include "Choose.h"
# include "Callback.h"
# include "Policy.h"
# include "CRAlgorithmScope.h"

namespace lbcpp
{

/*!
** @class CRAlgorithm
** @brief #FIXME
**
*/
class CRAlgorithm : public CRAlgorithmScope
{
public:
  /*
  ** Assignment
  */
  CRAlgorithm& operator =(const CRAlgorithm& otherCRAlgorithm)
    {setScope(otherCRAlgorithm); return *this;}

  /*{
  ** Policy running
  */

  /**
  ** Runs a @a policy from the initial state.
  **
  ** @param policy : search policy.
  **
  ** @return a boolean.
  */
  virtual bool run(PolicyPtr policy) = 0;

  /**
  ** Runs a @a policy from the current state (@a choice).
  **
  ** @param policy : search policy.
  ** @param choice : a choice.
  */
  virtual void run(PolicyPtr policy, VariablePtr choice) = 0;

  /*}
  **
  */


  /*{
  ** Step by step
  */

  /**
  **
  **
  ** deprecated
  **
  ** @param callback
  ** @param choice
  **
  ** @return
  */
  virtual bool step(Callback& callback, VariablePtr choice) = 0; // returns false if the new state is a final state

  /**
  ** Runs the CRAlgorithm until it reachs the first @em choose
  ** keyword.
  **
  ** @param reward : a pointer to the reward value (if any, NULL by
  ** default).
  **
  ** @return False if there was no choose keyword.
  */
  virtual ChoosePtr runUntilFirstChoose(double* reward = NULL) = 0;

  /**
  ** Runs the CRAlogirthm until it reachs the next @em choose
  ** keyword.
  **
  ** @param choice : a choice.
  ** @param reward : a pointer to the reward value (if any, NULL by
  ** default).
  **
  ** @return False if there was no remaining chooses.
  */
  virtual ChoosePtr runUntilNextChoose(VariablePtr choice, double* reward = NULL) = 0;

  /*}
  **
  */

  /*{
  ** Result
  */

  /**
  ** Returns True if there was any return keyword into the CRAlgorithm.
  **
  ** @return True if there was any return keyword into the CRAlgorithm.
  */
  virtual bool hasReturn() const = 0;

  /**
  ** Returns something according to the execution of the CRALgorithm:
  ** - VariablePtr() if the execution is not yet over.
  ** - VariablePtr() if the return value is "void". 
  ** - return value if the execution is over.
  **
  ** @return something according to the execution of the CRALgorithm (see above)
  */
  virtual VariablePtr getReturn() const = 0;

  /*}
  **
  */

};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_H_
