/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
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


#include "NumericalInference/LinearInference.h"
#include "NumericalInference/TransferFunctionDecoratorInference.h"
#include "NumericalInference/BinaryClassificationInference.h"
#include "NumericalInference/RegressionInference.h"

#include "DecisionTreeInference/ExtraTreeInference.h"

#include "ReductionInference/OneAgainstAllClassificationInference.h"

#include "MetaInference/CallbackBasedDecoratorInference.h"
#include "MetaInference/RunOnSupervisedExamplesInference.h"

InferencePtr lbcpp::linearScalarInference(const String& name)
  {return new LinearInference(name);}

InferencePtr lbcpp::transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction)
  {return new TransferFunctionDecoratorInference(name, decoratedInference, transferFunction);}

InferencePtr lbcpp::callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback)
  {return new CallbackBasedDecoratorInference(name, decoratedInference, callback);}

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

InferencePtr lbcpp::oneAgainstAllClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
  {return new OneAgainstAllClassificationInference(name, classes, binaryClassifierModel);}

InferencePtr lbcpp::runOnSupervisedExamplesInference(InferencePtr inference)
  {return new RunOnSupervisedExamplesInference(inference);}

InferencePtr lbcpp::multiClassExtraTreeInference(const String& name)
  {return new ExtraTreeInference(name);}

void declareInferenceClasses()
{
  LBCPP_DECLARE_CLASS_LEGACY(LinearInference);

  LBCPP_DECLARE_CLASS_LEGACY(BinaryLinearSVMInference);
  LBCPP_DECLARE_CLASS_LEGACY(BinaryLogisticRegressionInference);
  
  LBCPP_DECLARE_CLASS_LEGACY(SquareRegressionInference);
  LBCPP_DECLARE_CLASS_LEGACY(AbsoluteRegressionInference);
  LBCPP_DECLARE_CLASS_LEGACY(AngleRegressionInference);

  LBCPP_DECLARE_CLASS_LEGACY(OneAgainstAllClassificationInference);

  LBCPP_DECLARE_CLASS_LEGACY(TransferFunctionDecoratorInference);
  LBCPP_DECLARE_CLASS_LEGACY(CallbackBasedDecoratorInference);
}
