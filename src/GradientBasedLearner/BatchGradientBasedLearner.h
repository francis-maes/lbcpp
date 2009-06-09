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
  BatchGradientBasedLearner(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr stoppingCriterion)
    : optimizer(optimizer), stoppingCriterion(stoppingCriterion) {}
  BatchGradientBasedLearner() {}
    
  virtual std::string toString() const
    {return "BatchGradientBasedLearner(" + lbcpp::toString(optimizer) + ", " + lbcpp::toString(stoppingCriterion) + ")";}

  virtual void trainStochasticBegin()
  {
    Object::error("Batch::trainStochasticBegin", "This is not a stochastic learner");
    assert(false);
  }

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    Object::error("Batch::trainStochasticExample", "This is not a stochastic learner");
    assert(false);
  }
  
  virtual void trainStochasticEnd()
  {
    Object::error("Batch::trainStochasticEnd", "This is not a stochastic learner");
    assert(false);
  }

  virtual bool trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples, ProgressCallback* progress)
  {
    assert(parameters);
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
  
  virtual void save(std::ostream& ostr) const
  {
    write(ostr, optimizer);
    write(ostr, stoppingCriterion);
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, optimizer) && read(istr, stoppingCriterion);}
  
protected:
  VectorOptimizerPtr optimizer;
  OptimizerStoppingCriterionPtr stoppingCriterion;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_BATCH_H_
