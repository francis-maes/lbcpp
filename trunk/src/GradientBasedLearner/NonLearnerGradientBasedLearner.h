/*-----------------------------------------.---------------------------------.
| Filename: NonLearnerGradientBasedLearner.h| A Learner which does nothing   |
| Author  : Francis Maes                   |                                 |
| Started : 15/05/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNER_NON_LEARNER_H_
# define LBCPP_GRADIENT_BASED_LEARNER_NON_LEARNER_H_

# include <lbcpp/GradientBasedLearner.h>

namespace lbcpp
{

class NonLearnerGradientBasedLearner : public GradientBasedLearner
{
public:
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
    {}
    
  virtual bool trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples, ProgressCallback* progress)
    {return true;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_NON_LEARNER_H_
