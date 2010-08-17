/*-----------------------------------------.---------------------------------.
| Filename: SparseVector.cpp               | Composite sparse vectors        |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 21:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/SparseVector.h>
#include <lbcpp/FeatureGenerator/impl/DoubleVector.hpp>
#include "DoubleVectorHelpers.hpp" 
using namespace lbcpp;

SparseVector::SparseVector(const SparseVector& otherVector)
  : BaseClass(otherVector.dictionary), values(otherVector.values), subVectors(otherVector.subVectors.size())
{
  for (size_t i = 0; i < subVectors.size(); ++i)
    subVectors[i] = std::make_pair(otherVector.subVectors[i].first,
      new SparseVector(*otherVector.subVectors[i].second));
}

SparseVector::SparseVector(FeatureDictionaryPtr dictionary, size_t reserveNumValues, size_t reserveNumSubVectors)
  : BaseClass(dictionary)
{
  if (reserveNumValues)
    values.reserve(reserveNumValues);
  if (reserveNumSubVectors)
    subVectors.reserve(reserveNumSubVectors);
}

void SparseVector::clear()
{
  values.clear();
  subVectors.clear();
  dictionary = FeatureDictionaryPtr();
}

FeatureDictionaryPtr SparseVector::getDictionary() const
{
  if (dictionary)
    return dictionary;
  static FeatureDictionaryPtr defaultDictionary;
  if (defaultDictionary)
    defaultDictionary = FeatureDictionaryManager::getInstance().getOrCreateRootDictionary(T("SparseVector"), true, true);
  return defaultDictionary;
}

SparseVector& SparseVector::operator =(const SparseVector& otherVector)
{
  clear();
  values = otherVector.values;
  subVectors = otherVector.subVectors;
  dictionary = otherVector.dictionary;
  return *this;
}

size_t SparseVector::size() const
{
  size_t res = values.size();
  for (size_t i = 0; i < subVectors.size(); ++i)
    res += subVectors[i].second->size();
  return res;
}

void SparseVector::set(size_t index, double value)
{
  if (value)
    SortedFeatureArrayHelper::set(values, index, value);
  else
    SortedFeatureArrayHelper::remove(values, index);
}

void SparseVector::set(const String& name, double value)
{
  jassert(dictionary);
  set(dictionary->getFeatures()->add(name), value);
}

void SparseVector::set(const std::vector<String>& path, double value)
{
  jassert(dictionary);
  SparseVectorPtr ptr(this);
  FeatureDictionaryPtr dictionary = this->dictionary;
  for (size_t i = 0; i < path.size() - 1; ++i)
  {
    size_t subVectorIndex = dictionary->getScopes()->add(path[i]);
    dictionary = dictionary->getSubDictionary(subVectorIndex);
    SparseVectorPtr subVector = ptr->getSubVector(subVectorIndex);
    if (!subVector)
    {
      subVector = new SparseVector(dictionary);
      ptr->setSubVector(subVectorIndex, subVector);
    }
    else
      subVector->ensureDictionary(dictionary);
    ptr = subVector;
  }
  ptr->set(dictionary->getFeatures()->add(path.back()), value);
}

double SparseVector::get(size_t index) const
{
  const double* res = SortedFeatureArrayHelper::get(values, index);
  return res ? *res : 0.0;
}

double& SparseVector::get(size_t index)
{
  return SortedFeatureArrayHelper::get(values, index, 0.0);
}

SparseVectorPtr& SparseVector::getSubVector(size_t index)
{
  return SortedSubVectorArrayHelper::get(subVectors, index);
}

SparseVectorPtr SparseVector::getSubVector(size_t index) const
{
  const SparseVectorPtr* res = SortedSubVectorArrayHelper::get(subVectors, index);
  return res ? *res : SparseVectorPtr();
}

SparseVectorPtr SparseVector::getSubVector(const String& name) const
{
  jassert(dictionary);
  int index = dictionary->getScopes()->getIndex(name);
  return index >= 0 ? getSubVector((size_t)index) : SparseVectorPtr();
}

ObjectPtr SparseVector::multiplyByScalar(double scalar)
{
  if (scalar == 0)
    clear();
  else
  {
    for (size_t i = 0; i < values.size(); ++i)
      values[i].second *= scalar;
    for (size_t i = 0 ; i < subVectors.size(); ++i)
      subVectors[i].second->multiplyByScalar(scalar);
  }
  return ObjectPtr(this);
}

bool SparseVector::load(InputStream& istr)
{
  dictionary = FeatureDictionaryManager::getInstance().readDictionaryNameAndGet(istr);
  if (!dictionary)
    return false;
  StringDictionaryPtr features = dictionary->getFeatures();
  StringDictionaryPtr scopes = dictionary->getScopes();

  size_t numValues;
  if (!read(istr, numValues))
    return false;

  values.clear();
  values.reserve(numValues);
  for (size_t i = 0; i < numValues; ++i)
  {
    size_t featureIndex;
    double featureValue;
    if (!features->readIdentifier(istr, featureIndex) || !lbcpp::read(istr, featureValue))
      return false;
    jassert(isNumberValid(featureValue));
    values.push_back(std::make_pair(featureIndex, featureValue));
  }

  size_t numSubVectors;
  if (!read(istr, numSubVectors))
    return false;

  subVectors.clear();
  subVectors.reserve(numSubVectors);
  for (size_t i = 0; i < numSubVectors; ++i)
  {
    size_t scopeIndex;
    SparseVectorPtr subVector;
    if (!scopes->readIdentifier(istr, scopeIndex) || !read(istr, subVector))
      return false;
    subVectors.push_back(std::make_pair(scopeIndex, subVector));
  }
  return true;
}

void SparseVector::save(OutputStream& ostr) const
{
  jassert(dictionary);
  write(ostr, dictionary->getName());
  StringDictionaryPtr features = dictionary->getFeatures();
  StringDictionaryPtr scopes = dictionary->getScopes();  

  write(ostr, values.size());
  for (size_t i = 0; i < values.size(); ++i)
  {
    features->writeIdentifier(ostr, values[i].first);
    write(ostr, values[i].second);
  }

  write(ostr, subVectors.size());
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    scopes->writeIdentifier(ostr, subVectors[i].first);
    write(ostr, subVectors[i].second);
  }
}

double SparseVector::getValueByPosition(size_t position) const
{
  jassert(position < values.size());
  return values[position].second;
}

String SparseVector::getValueNameByPosition(size_t position) const
{
  jassert(position < values.size() && dictionary);
  return dictionary->getFeatures()->getString(values[position].first);
}
