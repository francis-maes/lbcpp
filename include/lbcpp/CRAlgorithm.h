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
**@brief  #FIXME: all
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

  /*!
  **
  **
  ** @param policy
  **
  ** @return
  */
  virtual bool run(PolicyPtr policy) = 0; // run a policy from the initial state

  /*!
  **
  **
  ** @param policy
  ** @param choice
  */
  virtual void run(PolicyPtr policy, VariablePtr choice) = 0; // run a policy from the current state

  /*}
  **
  */


  /*{
  ** Step by step
  */

  /*!
  **
  **
  ** @param callback
  ** @param choice
  **
  ** @return
  */
  virtual bool step(Callback& callback, VariablePtr choice) = 0; // returns false if the new state is a final state

  /*!
  **
  **
  ** @param reward
  **
  ** @return
  */
  virtual ChoosePtr runUntilFirstChoose(double* reward = NULL) = 0; // returns false if there was no choose

  /*!
  **
  **
  ** @param choice
  ** @param reward
  **
  ** @return
  */
  virtual ChoosePtr runUntilNextChoose(VariablePtr choice, double* reward = NULL) = 0; // returns false if there was no remaining chooses

  /*}
  **
  */

  /*{
  ** Result
  */

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool hasReturn() const = 0;

  /*!
  **
  **
  **
  ** @return
  */
  virtual VariablePtr getReturn() const = 0;

  /*}
  **
  */

};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_H_
