/*-----------------------------------------.---------------------------------.
| Filename: BatchGradientBasedLearner.h    | Batch Learner                   |
| Author  : Francis Maes                   |                                 |
| Started : 15/05/2009 18:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNER_BATCH_H_
# define LBCPP_GRADIENT_BASED_LEARNER_BATCH_H_

# include <lbcpp/GradientBasedLearner.h>

namespace lbcpp
{

class BatchGradientBasedLearner : public GradientBasedLearner
{
public:
  BatchGradientBasedLearner(VectorOptimizerPtr optimizer, StoppingCriterionPtr stoppingCriterion)
    : optimizer(optimizer), stoppingCriterion(stoppingCriterion) {}
  BatchGradientBasedLearner(const BatchGradientBasedLearner& other)
    : optimizer(other.optimizer), stoppingCriterion(other.stoppingCriterion) {}
  BatchGradientBasedLearner() {}
    
  virtual String toString() const
    {return "BatchGradientBasedLearner(" + lbcpp::toString(optimizer) + ", " + lbcpp::toString(stoppingCriterion) + ")";}

  virtual void trainStochasticBegin()
  {
    Object::error("Batch::trainStochasticBegin", "This is not a stochastic learner");
    jassert(false);
  }

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    Object::error("Batch::trainStochasticExample", "This is not a stochastic learner");
    jassert(false);
  }
  
  virtual void trainStochasticEnd()
  {
    Object::error("Batch::trainStochasticEnd", "This is not a stochastic learner");
    jassert(false);
  }

  virtual bool trainBatch(GradientBasedLearningMachine& learningMachine, ObjectContainerPtr examples, ProgressCallbackPtr progress)
  {
    ScalarVectorFunctionPtr objective = learningMachine.getRegularizedEmpiricalRisk(examples);
    jassert(parameters);
    if (progress)
      progress->progressStart("BatchLearner::trainBatch");
    FeatureGeneratorPtr params = parameters;
    if (!optimizer->optimize(objective, params, stoppingCriterion, progress))
      return false;
    parameters = params;
    if (progress)
      progress->progressEnd();
    return true;
  }
  
  virtual void save(OutputStream& ostr) const
  {
    write(ostr, optimizer);
    write(ostr, stoppingCriterion);
  }
  
  virtual bool load(InputStream& istr)
    {return read(istr, optimizer) && read(istr, stoppingCriterion);}

  virtual ObjectPtr clone() const
    {return new BatchGradientBasedLearner(*this);}
  
protected:
  VectorOptimizerPtr optimizer;
  StoppingCriterionPtr stoppingCriterion;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_BATCH_H_
