/*-----------------------------------------.---------------------------------.
| Filename: ExamplesCreatorCallback.h      | A callback that creates learning|
| Author  : Francis Maes                   |  examples                       |
| Started : 09/04/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_EXAMPLES_CREATOR_H_
# define LBCPP_INFERENCE_CONTEXT_EXAMPLES_CREATOR_H_

# include "InferenceCallback.h"
# include "InferenceStack.h"
# include "../InferenceStep/ClassificationInferenceStep.h"
# include "../InferenceStep/RegressionInferenceStep.h"
# include "../InferenceLearner/InferenceLearnerCallback.h"

namespace lbcpp
{

class ExamplesCreatorCallback : public InferenceCallback
{
public:
  ExamplesCreatorCallback(InferenceLearnerCallbackPtr learnerCallback)
    : learnerCallback(learnerCallback), enableExamplesCreation(true) {}

  virtual void classificationCallback(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (!classifier)
    {
      FeatureDictionaryPtr labels = stack->getCurrentInference().dynamicCast<ClassificationInferenceStep>()->getLabels();
      if (!labels)
      {
        returnCode = InferenceStep::errorReturnCode;
        return;
      }
      classifier = learnerCallback->createClassifier(stack->getCurrentInference(), labels);
    }

    if (supervision && enableExamplesCreation)
    {
      LabelPtr label = supervision.dynamicCast<Label>();
      jassert(classifier && label);
      addExample(classifier, new ClassificationExample(input, label->getIndex()));
    }
  }
  
  virtual void regressionCallback(InferenceStackPtr stack, RegressorPtr& regressor, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (!regressor)
      regressor = learnerCallback->createRegressor(stack->getCurrentInference());
    if (supervision && enableExamplesCreation)
    {
      ScalarPtr scalar = supervision.dynamicCast<Scalar>();
      if (scalar)
        addExample(regressor, new RegressionExample(input, scalar->getValue()));
      else
      {
        ScalarFunctionPtr lossFunction = supervision.dynamicCast<ScalarFunction>();
        jassert(lossFunction);
        addExample(regressor, new ObjectPair(input, lossFunction));
      }
    }    
  }

  void trainStochasticIteration()
  {
    for (LearningMachineMap::iterator it = learningMachines.begin(); it != learningMachines.end(); ++it)
    {
      LearningMachinePtr machine = it->first;
      ObjectContainerPtr trainingData = it->second.examples->randomize();
      std::cout << "Training with " << trainingData->size() << " examples... " << std::flush;
      jassert(machine);
      machine->trainStochastic(trainingData);
      GradientBasedLearningMachine* gbm = machine.dynamicCast<GradientBasedLearningMachine>().get();
      jassert(gbm);
      std::cout << "ok\n ==> empirical risk = " << std::flush;
      double empiricalRisk = gbm->computeEmpiricalRisk(trainingData);
      if (it->second.updateEmpiricalRisk(gbm->getParameters(), empiricalRisk))
        std::cout << "!" << String(empiricalRisk, 4) << "!";
      else
        std::cout << empiricalRisk;
      std::cout << ", num. params = " << gbm->getParameters()->l0norm() << " L2 norm = " << gbm->getParameters()->l2norm() << std::endl;
    }
  }

  void clearExamples()
  {
    for (LearningMachineMap::iterator it = learningMachines.begin(); it != learningMachines.end(); ++it)
      it->second.examples = VectorObjectContainerPtr();
  }

  void trainAndFlushExamples()
    {trainStochasticIteration(); clearExamples();}

  void restoreBestParameters()
  {
    for (LearningMachineMap::const_iterator it = learningMachines.begin(); it != learningMachines.end(); ++it)
    {
      GradientBasedLearningMachine* gbm = it->first.dynamicCast<GradientBasedLearningMachine>().get();
      jassert(gbm);
      gbm->setParameters(it->second.bestParameters);
    }
  }

protected:
  InferenceLearnerCallbackPtr learnerCallback;
  bool enableExamplesCreation;

  struct LearningMachineInfo
  {
    LearningMachineInfo() : bestEmpiricalRisk(DBL_MAX) {}

    VectorObjectContainerPtr examples;
    DenseVectorPtr bestParameters;
    double bestEmpiricalRisk;

    void addExample(ObjectPtr example)
    {
      if (!examples)
        examples = new VectorObjectContainer(T("Learning Examples"));
      examples->append(example);
    }

    bool updateEmpiricalRisk(DenseVectorPtr parameters, double empiricalRisk)
    {
      if (empiricalRisk >= bestEmpiricalRisk)
        return false;
      bestEmpiricalRisk = empiricalRisk;
      bestParameters = parameters->cloneAndCast<DenseVector>();
      return true;
    }
  };

  typedef std::map<LearningMachinePtr, LearningMachineInfo> LearningMachineMap;
  LearningMachineMap learningMachines;

  void addExample(LearningMachinePtr learningMachine, ObjectPtr example)
    {learningMachines[learningMachine].addExample(example);}
};

typedef ReferenceCountedObjectPtr<ExamplesCreatorCallback> ExamplesCreatorCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_EXAMPLES_CREATOR_H_
