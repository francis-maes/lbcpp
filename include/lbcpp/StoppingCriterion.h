/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterion.h            | Stopping Criterion              |
| Author  : Francis Maes                   |                                 |
| Started : 16/06/2009 12:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   StoppingCriterion.h
**@author Francis MAES
**@date   Fri Jun 12 19:16:47 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_STOPPING_CRITERION_H_
# define LBCPP_STOPPING_CRITERION_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

/*!
** @class StoppingCriterion
** @brief
*/
class StoppingCriterion : public Object
{
public:
  /*!
  **
  **
  */
  virtual void reset() = 0;

  /*!
  **
  **
  ** @param value
  **
  ** @return
  */
  virtual bool shouldOptimizerStop(double value) = 0;

    
  /*!
  **
  **
  ** @param policy
  ** @param examples
  **
  ** @return
  */
  virtual bool shouldCRAlgorithmLearnerStop(PolicyPtr policy, ObjectContainerPtr examples) = 0;
};

/*!
**
**
** @param maxIterations
**
** @return
*/
extern StoppingCriterionPtr maxIterationsStoppingCriterion(size_t maxIterations);

/*!
**
**
** @param tolerance
** @param relativeImprovement
**
** @return
*/
extern StoppingCriterionPtr averageImprovementStoppingCriterion(double tolerance, bool relativeImprovment = false);

/*!
**
**
** @param criterion1
** @param criterion2
**
** @return
*/
extern StoppingCriterionPtr logicalOr(StoppingCriterionPtr criterion1, StoppingCriterionPtr criterion2);

}; /* namespace lbcpp */

#endif // !LBCPP_STOPPING_CRITERION_H_

