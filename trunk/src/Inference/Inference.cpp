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

InferencePtr lbcpp::extraTreeInference(const String& name, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  {return new ExtraTreeInference(name, numTrees, numAttributeSamplesPerSplit, minimumSizeForSplitting);}

/*
** Reduction
*/
#include "ReductionInference/OneAgainstAllClassificationInference.h"

InferencePtr lbcpp::oneAgainstAllClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
  {return new OneAgainstAllClassificationInference(name, classes, binaryClassifierModel);}

/*
** Meta Inference
*/
#include "MetaInference/CallbackBasedDecoratorInference.h"
#include "MetaInference/RunOnSupervisedExamplesInference.h"
#include "MetaInference/SimulationInferenceBatchLearner.h"
#include "MetaInference/SequentialInferenceBatchLearner.h"
#include "MetaInference/ParallelInferenceBatchLearner.h"

InferencePtr lbcpp::runOnSupervisedExamplesInference(InferencePtr inference)
  {return new RunOnSupervisedExamplesInference(inference);}

InferencePtr lbcpp::callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback)
  {return new CallbackBasedDecoratorInference(name, decoratedInference, callback);}

InferencePtr lbcpp::simulationInferenceLearner()
  {return new SimulationInferenceBatchLearner();}

InferencePtr lbcpp::sequentialInferenceLearner()
  {return new SequentialInferenceBatchLearner();}

InferencePtr lbcpp::parallelInferenceLearner()
  {return new ParallelInferenceBatchLearner();}

void declareInferenceClasses()
{
  /*
  ** Base classes
  */
  LBCPP_DECLARE_ABSTRACT_CLASS(Inference, Object);
    LBCPP_DECLARE_ABSTRACT_CLASS(DecoratorInference, Inference);
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

  /*
  ** Numerical
  */
  LBCPP_DECLARE_ABSTRACT_CLASS(ParameterizedInference, Inference);
    LBCPP_DECLARE_CLASS(LinearInference, ParameterizedInference);

    LBCPP_DECLARE_ABSTRACT_CLASS(BinaryClassificationInference, DecoratorInference);
      LBCPP_DECLARE_CLASS(BinaryLinearSVMInference, BinaryClassificationInference);
      LBCPP_DECLARE_CLASS(BinaryLogisticRegressionInference, BinaryClassificationInference);

    LBCPP_DECLARE_ABSTRACT_CLASS(RegressionInference, DecoratorInference);
      LBCPP_DECLARE_CLASS(SquareRegressionInference, RegressionInference);
      LBCPP_DECLARE_CLASS(AbsoluteRegressionInference, RegressionInference);
      LBCPP_DECLARE_CLASS(AngleRegressionInference, RegressionInference);

    LBCPP_DECLARE_CLASS(TransferFunctionDecoratorInference, DecoratorInference);

  /*
  ** Decision Tree
  */
  LBCPP_DECLARE_CLASS(BinaryDecisionTree, Object);
  LBCPP_DECLARE_CLASS(BinaryDecisionTreeInference, Inference);
  LBCPP_DECLARE_CLASS(ExtraTreeInference, ParallelVoteInference);
  
  /*
  ** Meta
  */
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInference, DecoratorInference);  
  LBCPP_DECLARE_CLASS(SimulationInferenceBatchLearner, Inference);
  LBCPP_DECLARE_CLASS(SequentialInferenceBatchLearner, Inference);
  LBCPP_DECLARE_CLASS(ParallelInferenceBatchLearner, ParallelInference);
  
  LBCPP_DECLARE_CLASS(RunOnSupervisedExamplesInference, ParallelInference);
  LBCPP_DECLARE_ABSTRACT_CLASS(RunSequentialInferenceStepOnExamples, ParallelInference);
}
