/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithm.h                  | CR-algorithm base class         |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_CRALGORITHM_H_
# define LBCPP_CRALGORITHM_H_

# include "Choose.h"
# include "Callback.h"
# include "Policy.h"
# include "CRAlgorithmScope.h"

namespace lbcpp
{

class CRAlgorithm : public CRAlgorithmScope
{
public:
  /*
  ** Assignment
  */
  CRAlgorithm& operator =(const CRAlgorithm& otherCRAlgorithm)
    {setScope(otherCRAlgorithm); return *this;}
   
  /*
  ** Policy running
  */
  virtual bool run(PolicyPtr policy) = 0; // run a policy from the initial state
  virtual void run(PolicyPtr policy, VariablePtr choice) = 0; // run a policy from the current state
    
  /*
  ** Step by step
  */
  virtual bool step(Callback& callback, VariablePtr choice) = 0; // returns false if the new state is a final state
  virtual ChoosePtr runUntilFirstChoose(double* reward = NULL) = 0; // returns false if there was no choose
  virtual ChoosePtr runUntilNextChoose(VariablePtr choice, double* reward = NULL) = 0; // returns false if there was no remaining chooses
  
  /*
  ** Result
  */
  virtual bool hasReturn() const = 0;
  virtual VariablePtr getReturn() const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_H_
