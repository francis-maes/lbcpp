/*-----------------------------------------.---------------------------------.
| Filename: SparseVector.cpp               | Composite sparse vectors        |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 21:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/SparseVector.h>
#include <cralgo/impl/Bridge/DoubleVector.hpp>
#include "DoubleVectorHelpers.hpp" 
using namespace cralgo;

SparseVector::SparseVector(const SparseVector& otherVector)
  : features(otherVector.features), subVectors(subVectors)
{
  dictionary = otherVector.dictionary;
}

SparseVector::SparseVector(FeatureDictionary& dictionary)
{
  this->dictionary = &dictionary;
}

SparseVector::SparseVector()
{
  dictionary = NULL;
}

void SparseVector::clear()
{
  features.clear();
  subVectors.clear();
  dictionary = NULL;
}

SparseVector& SparseVector::operator =(const SparseVector& otherVector)
{
  clear();
  features = otherVector.features;
  subVectors = otherVector.subVectors;
  dictionary = otherVector.dictionary;
  return *this;
}

size_t SparseVector::size() const
{
  size_t res = features.size();
  for (size_t i = 0; i < subVectors.size(); ++i)
    res += subVectors[i].second->size();
  return res;
}

void SparseVector::set(size_t index, double value)
{
  if (value)
    SortedFeatureArrayHelper::set(features, index, value);
  else
    SortedFeatureArrayHelper::remove(features, index);
}

void SparseVector::set(const std::string& name, double value)
{
  assert(dictionary);
  set(dictionary->getFeatures().add(name), value);
}

void SparseVector::set(const std::vector<std::string>& path, double value)
{
  assert(dictionary);
  SparseVectorPtr ptr = getSharedPointer();
  FeatureDictionary* dictionary = this->dictionary;
  for (size_t i = 0; i < path.size() - 1; ++i)
  {
    size_t subVectorIndex = dictionary->getScopes().add(path[i]);
    dictionary = &dictionary->getSubDictionary(subVectorIndex);
    SparseVectorPtr subVector = ptr->getSubVector(subVectorIndex);
    if (!subVector)
      subVector = SparseVectorPtr(new SparseVector());
    subVector->ensureDictionary(*dictionary);
    ptr = subVector;
  }
  ptr->set(dictionary->getFeatures().add(path.back()), value);
}
  
SparseVectorPtr& SparseVector::getSubVector(size_t index)
{
  return SortedSubVectorArrayHelper::get(subVectors, index);
}

bool SparseVector::load(std::istream& istr)
{
  size_t numSubVectors;
  if (!read(istr, features) || !read(istr, numSubVectors))
    return false;
  subVectors.resize(numSubVectors);
  for (size_t i = 0; i < numSubVectors; ++i)
  {
    size_t index;
    if (!read(istr, index))
      return false;
    SparseVectorPtr subVector(new SparseVector());
    if (!subVector->load(istr))
      return false;
    subVectors[i] = std::make_pair(index, subVector);
  }
  return true;
}

void SparseVector::save(std::ostream& ostr) const
{
  write(ostr, features);
  write(ostr, subVectors.size());
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    const std::pair<size_t, SparseVectorPtr>& e = subVectors[i];
    write(ostr, e.first);
    assert(e.second);
    e.second->save(ostr);
  }
}
