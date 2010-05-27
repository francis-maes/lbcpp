/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp              | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
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
  jassert(index < subInferences.size());
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


#include "Inference/LinearInference.h"
#include "Inference/CallbackBasedDecoratorInference.h"
#include "Inference/TransferFunctionDecoratorInference.h"
#include "Inference/BinaryClassificationInference.h"
#include "Inference/OneAgainstAllClassificationInference.h"

#include "Inference/RunOnSupervisedExamplesInference.h"

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

InferencePtr lbcpp::oneAgainstAllClassificationInference(const String& name, FeatureDictionaryPtr labelsDictionary, InferencePtr binaryClassifierModel)
  {return new OneAgainstAllClassificationInference(name, labelsDictionary, binaryClassifierModel);}

InferencePtr lbcpp::runOnSupervisedExamplesInference(InferencePtr inference)
  {return new RunOnSupervisedExamplesInference(inference);}

void declareInferenceClasses()
{
  LBCPP_DECLARE_CLASS(LinearInference);

  LBCPP_DECLARE_CLASS(BinaryLinearSVMInference);
  LBCPP_DECLARE_CLASS(BinaryLogisticRegressionInference);

  LBCPP_DECLARE_CLASS(OneAgainstAllClassificationInference);

  LBCPP_DECLARE_CLASS(TransferFunctionDecoratorInference);
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInference);

  // old
  LBCPP_DECLARE_CLASS(ClassificationInferenceStep);
  LBCPP_DECLARE_CLASS(RegressionInferenceStep);
  // -
}
