/*-----------------------------------------.---------------------------------.
| Filename: GPOMDPPolicy.hpp               | GPOMDP Policy Gradient          |
| Author  : Francis Maes                   |                                 |
| Started : 16/03/2009 17:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_GPOMDP_H_
# define CRALGO_IMPL_POLICY_GPOMDP_H_

# include "PolicyStatic.hpp"
# include "../../RandomVariable.h"
# include "../../GradientBasedLearningMachine.h"

namespace cralgo {
namespace impl {

struct GPOMDPPolicy : public EpisodicPolicy<GPOMDPPolicy>
{
  typedef EpisodicPolicy<GPOMDPPolicy> BaseClass;
  
  GPOMDPPolicy(GradientBasedGeneralizedClassifierPtr classifier, double beta, double exploration = 1.0)
    : classifier(classifier), beta(beta), exploration(exploration)
    {}

  VariablePtr policyStart(ChoosePtr choose)
  {
    trace = classifier->createInitialParameters();
    classifier->trainStochasticBegin();
    return processChoose(choose);
  }
  
  VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    processReward(reward);
    return processChoose(choose);
  }
  
  void policyEnd(double reward)
  {
    processReward(reward);
    classifier->trainStochasticEnd();
  }

private:
  GradientBasedGeneralizedClassifierPtr classifier;
  double beta;
  double exploration;
  DenseVectorPtr trace;
  
  std::vector<FeatureGeneratorPtr> actionFeatures;
  size_t selectedAction;
  DenseVectorPtr actionProbabilities;

  VariablePtr sampleChoice(ChoosePtr choose, DenseVectorPtr probabilities, double probabilitiesSum = 1.0)
  {
    double r = Random::getInstance().sampleDouble(probabilitiesSum);
   //std::cout << "Probabilities: " << probabilities->toString() << " r = " << r << std::endl;
  
    VariableIteratorPtr iterator = choose->newIterator();
    for (size_t i = 0; i < probabilities->getNumValues(); ++i, iterator->next())
    {
      assert(iterator->exists());
      double p = probabilities->get(i);
      if (r < p)
      {
        selectedAction = i;
       // std::cout << " => action " << i << std::endl;
        return iterator->get();
      }
      r -= p;
    }
    assert(false);
    return VariablePtr();
  }

  VariablePtr processChoose(ChoosePtr choose)
  {
    choose->computeActionFeatures(actionFeatures, true);
    actionProbabilities = classifier->predictProbabilities(actionFeatures);
    assert(actionProbabilities->getNumValues());
    if (exploration == 1.0)
      return sampleChoice(choose, actionProbabilities);
    else
    {
      double sum = 0.0;
      DenseVectorPtr probs = new DenseVector(actionProbabilities->getNumValues());
      for (size_t i = 0; i < actionProbabilities->getNumValues(); ++i)
      {
        double p = pow(actionProbabilities->get(i), exploration);
        probs->set(i, p);
        sum += p;
       // std::cout << p << " ";
      }
      VariablePtr res = sampleChoice(choose, probs, sum);
//      std::cout << " => " << res->toString() << std::endl;
      return res;
    }
  }
  
  void processReward(double reward)
  {
    if (!actionFeatures.size())
      return;
    
    // -(log p[y|x])
    ScalarVectorFunctionPtr loss = classifier->getLoss(GeneralizedClassificationExample(actionFeatures, selectedAction));
    
    // -gradient(p[y|x], parameters) / p[y|x]
    FeatureGeneratorPtr gradient = loss->computeGradient(classifier->getParameters());

/*    std::cout << "GRADIENT norm: " << std::endl << gradient->l2norm() << std::endl;
    std::cout << "TRACE norm: " << std::endl << trace->l2norm() << std::endl;
    std::cout << "PARAMS norm: " << std::endl << classifier->getParameters()->l2norm() << std::endl;*/

    classifier->pushInputSize((double)actionFeatures[Random::getInstance().sampleSize(actionFeatures.size())]->l0norm());

    // trace <- trace * beta + gradient(p[y|x], parameters) / p[y|x]
    if (beta)
    {
      trace->multiplyByScalar(beta);
      trace->substract(gradient);
      if (reward)
        classifier->trainStochasticExample(trace, -reward);
    }
    else if (reward)
    {
      // trace = -gradient
      classifier->trainStochasticExample(gradient, reward);
    }
  }
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_GPOMDP_H_
