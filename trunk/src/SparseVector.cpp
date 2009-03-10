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
  : values(otherVector.values), subVectors(subVectors)
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
  values.clear();
  subVectors.clear();
  dictionary = NULL;
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

void SparseVector::set(const std::string& name, double value)
{
  assert(dictionary);
  set(dictionary->getFeatures().add(name), value);
}

void SparseVector::set(const std::vector<std::string>& path, double value)
{
  assert(dictionary);
  SparseVectorPtr ptr(this);
  FeatureDictionary* dictionary = this->dictionary;
  for (size_t i = 0; i < path.size() - 1; ++i)
  {
    size_t subVectorIndex = dictionary->getScopes().add(path[i]);
    dictionary = &dictionary->getSubDictionary(subVectorIndex);
    SparseVectorPtr subVector = ptr->getSubVector(subVectorIndex);
    if (!subVector)
      subVector = new SparseVector();
    subVector->ensureDictionary(*dictionary);
    ptr = subVector;
  }
  ptr->set(dictionary->getFeatures().add(path.back()), value);
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

void SparseVector::multiplyByScalar(double scalar)
{
  if (scalar == 0)
    {clear(); return;}
  else if (scalar == 1)
    return;
  for (size_t i = 0; i < values.size(); ++i)
    values[i].second *= scalar;
  for (size_t i = 0 ; i < subVectors.size(); ++i)
    subVectors[i].second->multiplyByScalar(scalar);
}

bool SparseVector::load(std::istream& istr)
{
  size_t numSubVectors;
  if (!read(istr, values) || !read(istr, numSubVectors))
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
  write(ostr, values);
  write(ostr, subVectors.size());
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    const std::pair<size_t, SparseVectorPtr>& e = subVectors[i];
    write(ostr, e.first);
    assert(e.second);
    e.second->save(ostr);
  }
}
