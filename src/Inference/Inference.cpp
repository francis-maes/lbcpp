/*-----------------------------------------.---------------------------------.
| Filename: InferenceStep.cpp              | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBaseClasses.h>
#include <lbcpp/Inference/InferenceResultCache.h>
using namespace lbcpp;

/*
** VectorBasedInferenceHelper
*/
int VectorBasedInferenceHelper::findStepNumber(InferenceStepPtr step) const
{
  for (size_t i = 0; i < subInferences.size(); ++i)
    if (subInferences[i] == step)
      return (int)i;
  return -1;
}

File VectorBasedInferenceHelper::getSubInferenceFile(size_t index, const File& directory) const
{
  jassert(index < subInferences.size());
  InferenceStepPtr step = subInferences[index];
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
    if (number >= (int)subInferences.size())
      subInferences.resize(number + 1);
    subInferences[number] = step;
  }
  for (size_t i = 0; i < subInferences.size(); ++i)
    if (!subInferences[i])
    {
      Object::error(T("VectorSequentialInferenceStep::loadFromFile"), T("Inference steps are not contiguous"));
      return false;
    }
  return true;
}

/*
** LearnableAtomicInferenceStep
*/
void LearnableAtomicInferenceStep::accept(InferenceVisitorPtr visitor)
  {visitor->visit(LearnableAtomicInferenceStepPtr(this));}


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
InferenceStepResultCachePtr InferenceResultCache::getCacheForInferenceStep(InferenceStepPtr step) const
{
  CacheMap::const_iterator it = cache.find(step);
  return it == cache.end() ? InferenceStepResultCachePtr() : it->second;
}

ObjectPtr InferenceResultCache::get(InferenceStepPtr step, ObjectPtr input) const
{
  InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(step);
  ObjectPtr res = stepCache ? stepCache->get(input) : ObjectPtr();
  //if (res)
  //  std::cout << "Use: " << step->getName() << " input: " << input->getName() << std::endl;
  return res;
}

void InferenceResultCache::addStepCache(InferenceStepResultCachePtr stepCache)
{
  cache[stepCache->getStep()] = stepCache;
}

void InferenceResultCache::add(InferenceStepPtr step, ObjectPtr input, ObjectPtr output)
{
  //std::cout << "Add: " << step->getName() << " input: " << input->getName() << " output: " << output->toString() << std::endl;
  InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(step);
  if (!stepCache)
    stepCache = cache[step] = new InferenceStepResultCache(step);
  stepCache->add(input, output);
}
