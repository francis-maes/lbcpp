/*-----------------------------------------.---------------------------------.
| Filename: VariableSetModel.cpp           | VariableSet Model               |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2010 17:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "VariableSetModel.h"
#include "VariableSetModel/IndependantClassificationVariableSetModel.h"
#include "VariableSetModel/IterativeClassificationVariableSetModel.h"
#include "VariableSetModel/SimulatedIterativeClassificationVariableSetModel.h"
//#include "VariableSetModel/StackedClassificationVariableSetModel.h"
using namespace lbcpp;

class VariableSetScoreFunction : public Object
{
public:
  virtual void reset() = 0;
  virtual void addPrediction(VariableSetPtr prediction, VariableSetPtr correct) = 0;
  virtual double compute() const = 0;
};

class LabelAccuracyVariableSetScoreFunction : public VariableSetScoreFunction
{
public:
  LabelAccuracyVariableSetScoreFunction()
    {reset();}

  virtual void reset()
    {numCorrect = numVariables = 0;}

  virtual void addPrediction(VariableSetPtr prediction, VariableSetPtr correct)
  {
    size_t n = prediction->getNumVariables();
    jassert(n == correct->getNumVariables());
    for (size_t i = 0; i < n; ++i)
    {
      size_t correctLabel;
      if (correct->getVariable(i, correctLabel))
      {
        ++numVariables;
        size_t predictedLabel;
        if (prediction->getVariable(i, predictedLabel) && predictedLabel == correctLabel)
          ++numCorrect;
      }
    }
  }
  
  virtual double compute() const
    {return numVariables ? numCorrect / (double)numVariables : 0.0;}

private:
  size_t numCorrect;
  size_t numVariables;
};

double VariableSetModel::evaluate(ObjectContainerPtr examples) const
{
  LabelAccuracyVariableSetScoreFunction scoreFunction;
  for (size_t i = 0; i < examples->size(); ++i)
  {
    VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
    jassert(example);
    VariableSetPtr prediction = example->createInitialPrediction();
    predict(example, prediction);
    scoreFunction.addPrediction(prediction, example->getTargetVariables());
  }
  return scoreFunction.compute();
}

VariableSetModelPtr lbcpp::independantClassificationVariableSetModel(ClassifierPtr classifier)
  {return new IndependantClassificationVariableSetModel(classifier);}

VariableSetModelPtr lbcpp::optimisticClassificationVariableSetModel(ClassifierPtr classifier)
  {return new OptimisticClassificationVariableSetModel(classifier);}

VariableSetModelPtr lbcpp::iterativeClassificationVariableSetModel(ClassifierPtr initialClassifier, ClassifierPtr iterativeClassifier)
  {return new IterativeClassificationVariableSetModel(initialClassifier, iterativeClassifier);}

VariableSetModelPtr lbcpp::simulatedIterativeClassificationVariableSetModel(ClassifierPtr stochasticClassifier, StoppingCriterionPtr stoppingCriterion)
  {return new SimulatedIterativeClassificationVariableSetModel(stochasticClassifier, stoppingCriterion);}

void declareInterdependantVariableSetClasses()
{
  LBCPP_DECLARE_CLASS(IndependantClassificationVariableSetModel);
  LBCPP_DECLARE_CLASS(OptimisticClassificationVariableSetModel);
  LBCPP_DECLARE_CLASS(IterativeClassificationVariableSetModel);
  LBCPP_DECLARE_CLASS(SimulatedIterativeClassificationVariableSetModel);
}
