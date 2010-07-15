/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
using namespace lbcpp;

/*
** InferenceVector
*/
int InferenceVector::find(InferencePtr inference) const
{
  for (size_t i = 0; i < v.size(); ++i)
    if (v[i] == inference)
      return (int)i;
  return -1;
}

File InferenceVector::getSubInferenceFile(size_t index, const File& directory) const
{
  jassert(index < v.size());
  InferencePtr step = v[index];
  jassert(step);
  return directory.getChildFile(lbcpp::toString(index) + T("_") + step->getName() + T(".inference"));
}

bool InferenceVector::saveToDirectory(const File& directory) const
{
  for (size_t i = 0; i < v.size(); ++i)
    v[i]->saveToFile(getSubInferenceFile(i, directory));
  return true;
}

bool InferenceVector::loadFromDirectory(const File& file)
{
  juce::OwnedArray<File> stepFiles;
  file.findChildFiles(stepFiles, File::findFilesAndDirectories, false, T("*.inference"));
  for (int i = 0; i < stepFiles.size(); ++i)
  {
    File stepFile = *stepFiles[i];
    String fileName = stepFile.getFileName();
    int n = fileName.indexOfChar('_');
    if (n < 0)
    {
      Object::error(T("VectorSequentialInference::loadFromFile"), T("Could not parse file name ") + fileName);
      return false;
    }
    String numberString = fileName.substring(0, n);
    if (!numberString.containsOnly(T("0123456789")))
    {
      Object::error(T("VectorSequentialInference::loadFromFile"), T("Could not parse file name ") + fileName);
      return false;
    }
    int number = numberString.getIntValue();
    if (number < 0)
    {
      Object::error(T("VectorSequentialInference::loadFromFile"), T("Invalid step number ") + fileName);
      return false;
    }
    InferencePtr step = Object::createFromFileAndCast<Inference>(stepFile);
    if (!step)
      return false;
    if (number >= (int)v.size())
      v.resize(number + 1);
    v[number] = step;
  }
  for (size_t i = 0; i < v.size(); ++i)
    if (!v[i])
    {
      Object::error(T("VectorSequentialInference::loadFromFile"), T("Inference steps are not contiguous"));
      return false;
    }
  return true;
}

/*
** Numerical
*/
#include "NumericalInference/LinearInference.h"
#include "NumericalInference/TransferFunctionDecoratorInference.h"
#include "NumericalInference/BinaryClassificationInference.h"
#include "NumericalInference/RegressionInference.h"

InferencePtr lbcpp::linearScalarInference(const String& name)
  {return new LinearInference(name);}

InferencePtr lbcpp::transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction)
  {return new TransferFunctionDecoratorInference(name, decoratedInference, transferFunction);}

InferencePtr lbcpp::binaryLinearSVMInference(InferencePtr scoreInference)
  {return new BinaryLinearSVMInference(scoreInference);}

InferencePtr lbcpp::binaryLinearSVMInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new BinaryLinearSVMInference(learner, name);}

InferencePtr lbcpp::binaryLogisticRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new BinaryLogisticRegressionInference(learner, name);}

InferencePtr lbcpp::squareRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new SquareRegressionInference(learner, name);}

InferencePtr lbcpp::absoluteRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new AbsoluteRegressionInference(learner, name);}

InferencePtr lbcpp::dihedralAngleRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
  {return new AngleRegressionInference(learner, name);}

/*
** Decision Tree
*/
#include "DecisionTreeInference/ExtraTreeInference.h"
#include "DecisionTreeInference/ExtraTreeInferenceLearner.h"

inline InferencePtr extraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  {return new SingleExtraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting);}

InferencePtr lbcpp::regressionExtraTreeInference(const String& name, TypePtr inputType, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  InferencePtr decisionTreeModel = new RegressionBinaryDecisionTreeInference(name, inputType);
  return new ParallelVoteInference(name, numTrees, decisionTreeModel, extraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

InferencePtr lbcpp::binaryClassificationExtraTreeInference(const String& name, TypePtr inputType, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  InferencePtr decisionTreeModel = new BinaryClassificationBinaryDecisionTreeInference(name, inputType);
  return new ParallelVoteInference(name, numTrees, decisionTreeModel, extraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

InferencePtr lbcpp::classificationExtraTreeInference(const String& name, TypePtr inputType, EnumerationPtr classes, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
{
  InferencePtr decisionTreeModel = new ClassificationBinaryDecisionTreeInference(name, inputType, classes);
  return new ParallelVoteInference(name, numTrees, decisionTreeModel, extraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting));
}

/*
** Reduction
*/
#include "ReductionInference/OneAgainstAllClassificationInference.h"
#include "ReductionInference/ParallelVoteInference.h"
#include "ReductionInference/SharedParallelVectorInference.h"

InferencePtr lbcpp::oneAgainstAllClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
  {return new OneAgainstAllClassificationInference(name, classes, binaryClassifierModel);}

InferencePtr lbcpp::parallelVoteInference(const String& name, size_t numVoters, InferencePtr voteInferenceModel, InferencePtr voteLearner)
  {return new ParallelVoteInference(name, numVoters, voteInferenceModel, voteLearner);}

// sizeFunction: input -> size
// perception: (input, position) pair -> object
InferencePtr lbcpp::sharedParallelVectorInference(const String& name, FunctionPtr sizeFunction, PerceptionPtr perception, InferencePtr elementInference)
  {return new SharedParallelVectorInference(name, sizeFunction, perception, elementInference);}

/*
** Meta Inference
*/
#include "MetaInference/DummyInferenceLearner.h"
#include "MetaInference/StaticSequentialInferenceLearner.h"
#include "MetaInference/StaticParallelInferenceLearner.h"
#include "MetaInference/SharedParallelInferenceLearner.h"
#include "MetaInference/DecoratorInferenceLearner.h"
#include "MetaInference/OnlineToBatchInferenceLearner.h"

InferencePtr lbcpp::dummyInferenceLearner()
  {return new DummyInferenceLearner();}

InferencePtr lbcpp::staticSequentialInferenceLearner()
  {return new StaticSequentialInferenceLearner();}

InferencePtr lbcpp::staticParallelInferenceLearner()
  {return new StaticParallelInferenceLearner();}

InferencePtr lbcpp::sharedParallelInferenceLearner(bool filterUnsupervisedExamples)
  {return new SharedParallelInferenceLearner(filterUnsupervisedExamples);}

InferencePtr lbcpp::parallelVoteInferenceLearner()
  {return new ParallelVoteInferenceLearner();}

InferencePtr lbcpp::decoratorInferenceLearner()
  {return new DecoratorInferenceLearner();}

InferencePtr lbcpp::postProcessInferenceLearner()
  {return new PostProcessInferenceLearner();}

InferencePtr lbcpp::onlineToBatchInferenceLearner()
  {return new OnlineToBatchInferenceLearner();}

#include "MetaInference/CallbackBasedDecoratorInference.h"
#include "MetaInference/RunOnSupervisedExamplesInference.h"

InferencePtr lbcpp::runOnSupervisedExamplesInference(InferencePtr inference)
  {return new RunOnSupervisedExamplesInference(inference);}

InferencePtr lbcpp::callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback)
  {return new CallbackBasedDecoratorInference(name, decoratedInference, callback);}

class PostProcessInference : public StaticDecoratorInference
{
public:
  // postProcessingFunction: from (object,any) pair to object
  PostProcessInference(InferencePtr decorated, FunctionPtr postProcessingFunction)
    : StaticDecoratorInference(postProcessingFunction->toString() + T("(") + decorated->getName() + T(")"), decorated),
        postProcessingFunction(postProcessingFunction)
    {setBatchLearner(postProcessInferenceLearner());}

  PostProcessInference() {}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return postProcessingFunction->getOutputType(pairType(inputType, decorated->getOutputType(inputType)));}

  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
    {return postProcessingFunction->compute(Variable::pair(finalState->getInput(), finalState->getSubOutput()));}

protected:
  FunctionPtr postProcessingFunction;
};

InferencePtr lbcpp::postProcessInference(InferencePtr inference, FunctionPtr postProcessingFunction)
  {return new PostProcessInference(inference, postProcessingFunction);}

ClassPtr lbcpp::inferenceClass()
  {static TypeCache cache(T("Inference")); return cache();}

ClassPtr lbcpp::decoratorInferenceClass()
  {static TypeCache cache(T("DecoratorInference")); return cache();}

ClassPtr lbcpp::staticDecoratorInferenceClass()
  {static TypeCache cache(T("StaticDecoratorInference")); return cache();}

ClassPtr lbcpp::sequentialInferenceClass()
  {static TypeCache cache(T("SequentialInference")); return cache();}

ClassPtr lbcpp::staticSequentialInferenceClass()
  {static TypeCache cache(T("StaticSequentialInference")); return cache();}

ClassPtr lbcpp::parallelInferenceClass()
  {static TypeCache cache(T("ParallelInference")); return cache();}

ClassPtr lbcpp::staticParallelInferenceClass()
  {static TypeCache cache(T("StaticParallelInference")); return cache();}

ClassPtr lbcpp::sharedParallelInferenceClass()
  {static TypeCache cache(T("SharedParallelInference")); return cache();}

void declareInferenceClasses()
{
  /*
  ** Base classes
  */
  LBCPP_DECLARE_ABSTRACT_CLASS(Inference, NameableObject);
    LBCPP_DECLARE_ABSTRACT_CLASS(DecoratorInference, Inference);
      LBCPP_DECLARE_ABSTRACT_CLASS(StaticDecoratorInference, DecoratorInference);
        LBCPP_DECLARE_CLASS(PostProcessInference, DecoratorInference);
    LBCPP_DECLARE_ABSTRACT_CLASS(ParallelInference, Inference);
      LBCPP_DECLARE_ABSTRACT_CLASS(StaticParallelInference, ParallelInference);
        LBCPP_DECLARE_ABSTRACT_CLASS(SharedParallelInference, StaticParallelInference);
        LBCPP_DECLARE_ABSTRACT_CLASS(VectorStaticParallelInference, StaticParallelInference);
    LBCPP_DECLARE_ABSTRACT_CLASS(SequentialInference, Inference);
      LBCPP_DECLARE_ABSTRACT_CLASS(StaticSequentialInference, SequentialInference);
        LBCPP_DECLARE_CLASS(VectorSequentialInference, StaticSequentialInference);
  
  /*
  ** Reduction
  */
  LBCPP_DECLARE_CLASS(OneAgainstAllClassificationInference, VectorStaticParallelInference);
  LBCPP_DECLARE_CLASS(ParallelVoteInference, VectorStaticParallelInference);
  LBCPP_DECLARE_CLASS(SharedParallelVectorInference, SharedParallelInference);

  /*
  ** Numerical
  */
  LBCPP_DECLARE_ABSTRACT_CLASS(ParameterizedInference, Inference);
    LBCPP_DECLARE_CLASS(LinearInference, ParameterizedInference);

    LBCPP_DECLARE_ABSTRACT_CLASS(BinaryClassificationInference, StaticDecoratorInference);
      LBCPP_DECLARE_CLASS(BinaryLinearSVMInference, BinaryClassificationInference);
      LBCPP_DECLARE_CLASS(BinaryLogisticRegressionInference, BinaryClassificationInference);

    LBCPP_DECLARE_ABSTRACT_CLASS(RegressionInference, StaticDecoratorInference);
      LBCPP_DECLARE_CLASS(SquareRegressionInference, RegressionInference);
      LBCPP_DECLARE_CLASS(AbsoluteRegressionInference, RegressionInference);
      LBCPP_DECLARE_CLASS(AngleRegressionInference, RegressionInference);

    LBCPP_DECLARE_CLASS(TransferFunctionDecoratorInference, StaticDecoratorInference);

  /*
  ** Decision Tree
  */
  LBCPP_DECLARE_CLASS(BinaryDecisionTree, Object);
  LBCPP_DECLARE_CLASS(BinaryDecisionTreeInference, Inference);
    LBCPP_DECLARE_CLASS(RegressionBinaryDecisionTreeInference, BinaryDecisionTreeInference);
    LBCPP_DECLARE_CLASS(BinaryClassificationBinaryDecisionTreeInference, BinaryDecisionTreeInference);
    LBCPP_DECLARE_CLASS(ClassificationBinaryDecisionTreeInference, BinaryDecisionTreeInference);
  
  /*
  ** Meta
  */
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInference, StaticDecoratorInference);  
  LBCPP_DECLARE_CLASS(OnlineToBatchInferenceLearner, Inference);
  LBCPP_DECLARE_CLASS(StaticSequentialInferenceLearner, Inference);
  LBCPP_DECLARE_CLASS(StaticParallelInferenceLearner, ParallelInference);
    LBCPP_DECLARE_CLASS(ParallelVoteInferenceLearner, StaticParallelInferenceLearner);
  LBCPP_DECLARE_CLASS(SharedParallelInferenceLearner, DecoratorInference);
  LBCPP_DECLARE_CLASS(DecoratorInferenceLearner, DecoratorInference);
    LBCPP_DECLARE_CLASS(PostProcessInferenceLearner, DecoratorInferenceLearner);
  
  LBCPP_DECLARE_CLASS(RunOnSupervisedExamplesInference, ParallelInference);
  LBCPP_DECLARE_ABSTRACT_CLASS(RunSequentialInferenceStepOnExamples, ParallelInference);
}
