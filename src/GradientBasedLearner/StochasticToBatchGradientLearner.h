/*-----------------------------------------.---------------------------------.
| Filename: StochasticToBatchGradientLearner.h| Transforms a stochastic      |
| Author  : Francis Maes                   | learner into a batch learner    |
| Started : 29/03/2010 14:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#ifndef LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_TO_BATCH_H_
# define LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_TO_BATCH_H_

# include <lbcpp/GradientBasedLearner.h>

namespace lbcpp
{

class StochasticToBatchGradientLearner : public GradientBasedLearner
{
public:
  StochasticToBatchGradientLearner(GradientBasedLearnerPtr stochasticLearner, StoppingCriterionPtr stoppingCriterion, bool randomizeExamples)
    : stochasticLearner(stochasticLearner), stoppingCriterion(stoppingCriterion), randomizeExamples(randomizeExamples) {}
  StochasticToBatchGradientLearner() : randomizeExamples(true) {}
  
  virtual String toString() const
    {return T("StochasticToBatchGradientLearner(") + lbcpp::toString(stochasticLearner) +
            T(", ") + lbcpp::toString(stoppingCriterion) +
            T(", ") + lbcpp::toString(randomizeExamples) + T(")");}

  virtual void trainStochasticBegin()
    {Object::error("trainStochasticBegin", "This is not a stochastic learner"); jassert(false);}

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
    {Object::error("trainStochasticExample", "This is not a stochastic learner"); jassert(false);}
  
  virtual void trainStochasticEnd()
    {Object::error("trainStochasticEnd", "This is not a stochastic learner"); jassert(false);}
  
  virtual void trainBatch(GradientBasedLearningMachine& learningMachine, ObjectContainerPtr examples, ProgressCallbackPtr progress)
  {
    if (progress)
      progress->progressStart(T("Batch training with ") + lbcpp::toString(examples->size()) + T(" examples"));
    
    ScalarVectorFunctionPtr objective = learningMachine.getRegularizedEmpiricalRisk(examples);
    
    stochasticLearner->setParameters(parameters);
    stochasticLearner->setRegularizer(regularizer);

    stoppingCriterion->reset();
    for (size_t iteration = 0; true; ++iteration)
    {
      if (progress && !progress->progressStep(T("Batch training"), (double)iteration))
        return;

      stochasticLearner->trainStochasticBegin();
      ObjectContainerPtr ex = randomizeExamples ? examples->randomize() : examples;
      for (size_t i = 0; i < ex->size(); ++i)
      {
        ObjectPtr example = ex->get(i);
        stochasticLearner->trainStochasticExample(example, learningMachine.getLoss(example));
      }
      stochasticLearner->trainStochasticEnd();
      parameters = stochasticLearner->getParameters();
      double energy = objective->compute(parameters);
      std::cout << "Num Params: " << parameters->l0norm() << " L2 norm = " << parameters->l2norm() << " energy = " << energy << std::endl;
      if (stoppingCriterion->shouldOptimizerStop(-energy))
        break;
    }

    if (progress)
      progress->progressEnd();
  }
  
  virtual void save(OutputStream& ostr) const
  {
    write(ostr, stochasticLearner);
    write(ostr, stoppingCriterion);
    write(ostr, randomizeExamples);
  }
  
  virtual bool load(InputStream& istr)
    {return read(istr, stochasticLearner) && read(istr, stoppingCriterion) && read(istr, randomizeExamples);}
  
  virtual ObjectPtr clone() const
    {return new StochasticToBatchGradientLearner(stochasticLearner, stoppingCriterion, randomizeExamples);}

private:
  GradientBasedLearnerPtr stochasticLearner;
  StoppingCriterionPtr stoppingCriterion;
  bool randomizeExamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_TO_BATCH_H_

