/*-----------------------------------------.---------------------------------.
| Filename: SequentialInferenceStep.cpp    | Base class for sequential       |
| Author  : Francis Maes                   |   inference                     |
| Started : 17/04/2010 11:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "SequentialInferenceStep.h"
#include "../InferenceContext/InferenceContext.h"
using namespace lbcpp;

/*
** SequentialInferenceStep
*/
String SequentialInferenceStep::toString() const
{
  String res = getClassName() + T("(");
  size_t n = getNumSubSteps();
  for (size_t i = 0; i < n; ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    res += step->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res + T(")");
}

void SequentialInferenceStep::accept(InferenceVisitorPtr visitor)
  {visitor->visit(SequentialInferenceStepPtr(this));}

ObjectPtr SequentialInferenceStep::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  size_t n = getNumSubSteps();
  ObjectPtr currentData = input;
  for (size_t i = 0; i < n; ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    ObjectPtr currentSupervision = supervision ? getSubSupervision(supervision, i) : ObjectPtr();
    currentData = context->runInference(step, currentData, currentSupervision, returnCode);
    if (returnCode != finishedReturnCode)
      return ObjectPtr();
    jassert(currentData);
  }
  return currentData;
}

/*
** VectorSequentialInferenceStep
*/
void VectorSequentialInferenceStep::appendStep(InferenceStepPtr inference)
  {inferenceSteps.push_back(inference);}

size_t VectorSequentialInferenceStep::getNumSubSteps() const
  {return inferenceSteps.size();}

InferenceStepPtr VectorSequentialInferenceStep::getSubStep(size_t index) const
  {jassert(index < inferenceSteps.size()); return inferenceSteps[index];}

bool VectorSequentialInferenceStep::saveToFile(const File& file) const
{
  if (!saveToDirectory(file))
    return false;

  for (size_t i = 0; i < getNumSubSteps(); ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    step->saveToFile(file.getChildFile(lbcpp::toString(i) + T("_") + step->getName() + T(".inference")));
  }
  return true;
}

bool VectorSequentialInferenceStep::loadFromFile(const File& file)
{
  if (!loadFromDirectory(file))
    return false;

  juce::OwnedArray<File> stepFiles;
  file.findChildFiles(stepFiles, File::findFilesAndDirectories, false, T("*.inference"));
  for (int i = 0; i < stepFiles.size(); ++i)
  {
    File stepFile = *stepFiles[i];
    String fileName = stepFile.getFileName();
    int n = fileName.indexOfChar('_');
    if (n < 0)
    {
      Object::error(T("VectorSequentialInferenceStep::loadFromFile"), T("Could not parse file name ") + fileName);
      return false;
    }
    String numberString = fileName.substring(0, n);
    if (!numberString.containsOnly(T("0123456789")))
    {
      Object::error(T("VectorSequentialInferenceStep::loadFromFile"), T("Could not parse file name ") + fileName);
      return false;
    }
    int number = numberString.getIntValue();
    if (number < 0)
    {
      Object::error(T("VectorSequentialInferenceStep::loadFromFile"), T("Invalid step number ") + fileName);
      return false;
    }
    InferenceStepPtr step = Object::createFromFileAndCast<InferenceStep>(stepFile);
    if (!step)
      return false;
    inferenceSteps.resize(number + 1);
    inferenceSteps[number] = step;
  }
  for (size_t i = 0; i < inferenceSteps.size(); ++i)
    if (!inferenceSteps[i])
    {
      Object::error(T("VectorSequentialInferenceStep::loadFromFile"), T("Inference steps are not contiguous"));
      return false;
    }
  return true;
}
