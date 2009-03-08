/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.cpp       | Gradient based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 23:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/GradientBasedLearner.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

class GradientDescentLearner : public GradientBasedLearner
{
public:
  virtual void trainStochasticExample(DenseVectorPtr parameters, const FeatureGeneratorPtr input, ScalarVectorFunctionPtr loss, ScalarVectorFunctionPtr regularizer)
  {
    double alpha = 1.0;
    parameters->addWeighted(loss->computeGradient(input), -alpha);
  }
  
  virtual void trainStochasticEnd(DenseVectorPtr parameters, ScalarVectorFunctionPtr regularizer)
  {
    if (regularizer)
    {
      // apply regularizer
      double alpha = 1.0;
      parameters->addWeighted(regularizer->computeGradient(parameters), -alpha);
    }
  }

  virtual void trainBatch(DenseVectorPtr parameters, ScalarVectorFunctionPtr objective)
  {
    double alpha = 1.0;
    for (int i = 0; i < 100; ++i)
    {
      std::cout << "Iteration " << i << " objective = " << objective->compute(parameters) << std::endl; 
      parameters->addWeighted(objective->computeGradient(parameters), -alpha);
    }
  }
};

GradientBasedLearnerPtr cralgo::createGradientDescentLearner()
  {return GradientBasedLearnerPtr(new GradientDescentLearner());}
