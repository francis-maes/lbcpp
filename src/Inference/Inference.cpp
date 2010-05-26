/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp              | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
#include <lbcpp/Inference/InferenceResultCache.h>
using namespace lbcpp;

/*
** SharedParallelInference
*/
SharedParallelInference::SharedParallelInference(const String& name, InferencePtr subInference)
  : ParallelInference(name), subInference(subInference) {}

void SharedParallelInference::accept(InferenceVisitorPtr visitor)
  {visitor->visit(SharedParallelInferencePtr(this));}

ObjectPtr SharedParallelInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  subInference->beginRunSession();
  ObjectPtr res = ParallelInference::run(context, input, supervision, returnCode);
  subInference->endRunSession();
  return res;
}

String SharedParallelInference::toString() const
{
  jassert(subInference);
  return getClassName() + T("(") + subInference->toString() + T(")");
}

bool SharedParallelInference::loadFromFile(const File& file)
{
  if (!loadFromDirectory(file))
    return false;

  subInference = createFromFileAndCast<Inference>(file.getChildFile(T("shared.inference")));
  return subInference != InferencePtr();
}

bool SharedParallelInference::saveToFile(const File& file) const
  {return saveToDirectory(file) && subInference->saveToFile(file.getChildFile(T("shared.inference")));}

/*
** ParallelSharedMultiRegressionInference
*/
ParallelSharedMultiRegressionInference::ParallelSharedMultiRegressionInference(const String& name, FeatureDictionaryPtr outputDictionary)
  : SharedParallelInference(name, new RegressionInferenceStep(name + T("Regression"))), outputDictionary(outputDictionary) {}

size_t ParallelSharedMultiRegressionInference::getNumSubInferences(ObjectPtr input) const
{
  ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
  jassert(container);
  return container->size();
}

ObjectPtr ParallelSharedMultiRegressionInference::getSubInput(ObjectPtr input, size_t index) const
  {return getInputFeatures(input, index);}

ObjectPtr ParallelSharedMultiRegressionInference::getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
{
  if (!supervision)
    return ObjectPtr();

  DenseVectorPtr vector = supervision.dynamicCast<DenseVector>();
  jassert(vector);
  return new Scalar(vector->get(index));
}

ObjectPtr ParallelSharedMultiRegressionInference::createEmptyOutput(ObjectPtr input) const
  {return new DenseVector(outputDictionary, getNumSubInferences(input));}

void ParallelSharedMultiRegressionInference::setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
{
  DenseVectorPtr vector = output.dynamicCast<DenseVector>();
  jassert(vector);
  ScalarPtr scalar = subOutput.dynamicCast<Scalar>();
  jassert(scalar);
  vector->set(index, scalar->getValue());
}

/*
** DecoratorInference
*/
String DecoratorInference::toString() const
  {return getClassName() + T("(") + (decorated ? decorated->toString() : T("<null>")) + T(")");}

bool DecoratorInference::loadFromFile(const File& file)
{
  if (!loadFromDirectory(file))
    return false;
  decorated = createFromFileAndCast<Inference>(file.getChildFile(T("decorated.inference")));
  return decorated != InferencePtr();
}

bool DecoratorInference::saveToFile(const File& file) const
{
  return saveToDirectory(file) &&
    decorated->saveToFile(file.getChildFile(T("decorated.inference")));
}

ObjectPtr DecoratorInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  if (decorated)
    return context->runInference(decorated, input, supervision, returnCode);
  else
  {
    returnCode = Inference::errorReturnCode;
    return ObjectPtr();
  }
}

void DecoratorInference::accept(InferenceVisitorPtr visitor)
{
  if (decorated)
    decorated->accept(visitor);
}

/*
** SequentialInference
*/
String SequentialInference::toString() const
{
  String res = getClassName() + T("(");
  size_t n = getNumSubSteps();
  for (size_t i = 0; i < n; ++i)
  {
    InferencePtr step = getSubStep(i);
    res += step->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res + T(")");
}

void SequentialInference::accept(InferenceVisitorPtr visitor)
  {visitor->visit(SequentialInferencePtr(this));}

ObjectPtr SequentialInference::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  size_t n = getNumSubSteps();
  ObjectPtr currentData = input;
  for (size_t i = 0; i < n; ++i)
  {
    InferencePtr step = getSubStep(i);
    ObjectPtr currentSupervision = supervision ? getSubSupervision(supervision, i) : ObjectPtr();
    currentData = context->runInference(step, currentData, currentSupervision, returnCode);
    if (returnCode != finishedReturnCode)
      return ObjectPtr();
    jassert(currentData);
  }
  return currentData;
}

/*
** LearnableAtomicInference
*/
void LearnableAtomicInference::accept(InferenceVisitorPtr visitor)
  {visitor->visit(LearnableAtomicInferencePtr(this));}

DenseVectorPtr LearnableAtomicInference::getOrCreateParameters(FeatureDictionaryPtr dictionary)
{
  if (!parameters)
    parameters = new DenseVector(dictionary);
  else
    parameters->ensureDictionary(dictionary);
  return parameters;
}

/*
** VectorBasedInferenceHelper
*/
int VectorBasedInferenceHelper::findStepNumber(InferencePtr step) const
{
  for (size_t i = 0; i < subInferences.size(); ++i)
    if (subInferences[i] == step)
      return (int)i;
  return -1;
}

File VectorBasedInferenceHelper::getSubInferenceFile(size_t index, const File& directory) const
{
  jassert(index < subInferences.size());
  InferencePtr step = subInferences[index];
  jassert(step);
  return directory.getChildFile(lbcpp::toString(index) + T("_") + step->getName() + T(".inference"));
}

bool VectorBasedInferenceHelper::saveSubInferencesToDirectory(const File& directory) const
{
  for (size_t i = 0; i < getNumSubSteps(); ++i)
    getSubStep(i)->saveToFile(getSubInferenceFile(i, directory));
  return true;
}

bool VectorBasedInferenceHelper::loadSubInferencesFromDirectory(const File& file)
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
    if (number >= (int)subInferences.size())
      subInferences.resize(number + 1);
    subInferences[number] = step;
  }
  for (size_t i = 0; i < subInferences.size(); ++i)
    if (!subInferences[i])
    {
      Object::error(T("VectorSequentialInference::loadFromFile"), T("Inference steps are not contiguous"));
      return false;
    }
  return true;
}

/*
** InferenceStepResultCache
*/
ObjectPtr InferenceStepResultCache::get(ObjectPtr input) const
{
  InputOutputMap::const_iterator it = cache.find(input->getName());
  return it == cache.end() ? ObjectPtr() : it->second;
}

/*
** InferenceResultCache
*/
InferenceStepResultCachePtr InferenceResultCache::getCacheForInferenceStep(InferencePtr step) const
{
  CacheMap::const_iterator it = cache.find(step);
  return it == cache.end() ? InferenceStepResultCachePtr() : it->second;
}

ObjectPtr InferenceResultCache::get(InferencePtr step, ObjectPtr input) const
{
  InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(step);
  ObjectPtr res = stepCache ? stepCache->get(input) : ObjectPtr();
  //if (res)
  //  std::cout << "Use: " << step->getName() << " input: " << input->getName() << std::endl;
  return res;
}

void InferenceResultCache::addStepCache(InferenceStepResultCachePtr stepCache)
{
  cache[stepCache->getInference()] = stepCache;
}

void InferenceResultCache::add(InferencePtr inference, ObjectPtr input, ObjectPtr output)
{
  //std::cout << "Add: " << step->getName() << " input: " << input->getName() << " output: " << output->toString() << std::endl;
  InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(inference);
  if (!stepCache)
    stepCache = cache[inference] = new InferenceStepResultCache(inference);
  stepCache->add(input, output);
}


#include "Inference/LinearScalarInference.h"
#include "Inference/CallbackBasedDecoratorInference.h"
#include "Inference/TransferFunctionDecoratorInference.h"

LearnableAtomicInferencePtr lbcpp::linearScalarInference(const String& name)
  {return new LinearScalarInference(name);}

InferencePtr lbcpp::transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction)
  {return new TransferFunctionDecoratorInference(name, decoratedInference, transferFunction);}

InferencePtr lbcpp::callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback)
  {return new CallbackBasedDecoratorInference(name, decoratedInference, callback);}

void declareInferenceClasses()
{
  LBCPP_DECLARE_CLASS(ClassificationInferenceStep);
  LBCPP_DECLARE_CLASS(RegressionInferenceStep);
  LBCPP_DECLARE_CLASS(TransferFunctionDecoratorInference);
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInference);
}
