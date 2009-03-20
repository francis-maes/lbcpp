/*-----------------------------------------.---------------------------------.
| Filename: DenseVector.cpp                | Composite Dense Vectors         |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2009 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <cralgo/DenseVector.h>
#include <cralgo/LazyVector.h>
#include <cralgo/impl/Bridge/DoubleVector.hpp>
#include <cfloat>
using namespace cralgo;

void DenseVector::initialize(size_t initialNumValues, size_t initialNumSubVectors)
{
  if (initialNumValues > 0)
    values.resize(initialNumValues, 0.0);
  if (initialNumSubVectors > 0)
  {
    subVectors.resize(initialNumSubVectors, DenseVectorPtr());
    for (size_t i = 0; i < subVectors.size(); ++i)
      subVectors[i] = new DenseVector();
  }
}

FeatureDictionaryPtr DenseVector::getDictionary() const
{
  if (dictionary)
    return dictionary;
  static FeatureDictionaryPtr defaultDictionary = new FeatureDictionary("DenseVector");
  return defaultDictionary;
}

DenseVector& DenseVector::operator =(const DenseVector& otherVector)
{
  values = otherVector.values;
  subVectors = otherVector.subVectors;
  dictionary = otherVector.dictionary;
  return *this;
}

size_t DenseVector::size() const
{
  size_t res = values.size();
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    DenseVectorPtr subVector = subVectors[i];
    if (subVector)
      res += subVector->size();
  }
  return res;
}

void DenseVector::addWeighted(const LazyVectorPtr lazyVector, double weight)
{
  assert(lazyVector);
  lazyVector->addWeightedTo(DenseVectorPtr(this), weight);
}

// todo: factorize binary operations between dense vectors

void DenseVector::addWeighted(const DenseVectorPtr otherVector, double weight)
{
  if (!weight)
    return;
  const std::vector<double>& otherVectorValues = otherVector->getValues();
  size_t n = otherVectorValues.size();
  if (values.size() < n)
    values.resize(n, 0.0);
  for (size_t i = 0; i < n; ++i)
    values[i] += weight * otherVectorValues[i];
  for (size_t i = 0; i < otherVector->getNumSubVectors(); ++i)
  {
    DenseVectorPtr otherSubVector = otherVector->getSubVector(i);
    if (otherSubVector)
    {
      DenseVectorPtr& subVector = getSubVector(i);
      if (!subVector)
        subVector = new DenseVector(otherSubVector->getDictionary());
      subVector->addWeighted(otherSubVector, weight);
    }
  }
}

void DenseVector::add(const DenseVectorPtr otherVector)
{
  const std::vector<double>& otherVectorValues = otherVector->getValues();
  size_t n = otherVectorValues.size();
  if (values.size() < n)
    values.resize(n, 0.0);
  for (size_t i = 0; i < n; ++i)
    values[i] += otherVectorValues[i];
  for (size_t i = 0; i < otherVector->getNumSubVectors(); ++i)
  {
    DenseVectorPtr otherSubVector = otherVector->getSubVector(i);
    if (otherSubVector)
    {
      DenseVectorPtr& subVector = getSubVector(i);
      if (!subVector)
        subVector = new DenseVector(otherSubVector->getDictionary());
      subVector->add(otherSubVector);
    }
  }
}

int DenseVector::findIndexOfMaximumValue() const
{
  int res = -1;
  double max = -DBL_MAX;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i] > max)
      max = values[i], res = (int)i;
  return res;
}

double DenseVector::findMaximumValue() const
{
  double res = -DBL_MAX;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i] > res)
      res = values[i];
  return res;
}

// log(sum_i exp(x_i))
double DenseVector::computeLogSumOfExponentials() const
{
  double highestValue = findMaximumValue();
  double res = 0.0;
  for (size_t i = 0; i < values.size(); ++i)
    res += exp(values[i] - highestValue);
  return log(res) + highestValue;
}

void DenseVector::multiplyByScalar(double scalar)
{
  if (scalar == 0)
    {clear(); return;}
  else if (scalar == 1)
    return;

  // todo: operation class
  for (size_t i = 0; i < values.size(); ++i)
    values[i] *= scalar;
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    DenseVectorPtr subVector = subVectors[i];
    if (subVector)
      subVector->multiplyByScalar(scalar);
  }
}

double DenseVector::denseDotProduct(const DenseVectorPtr otherVector) const
{
  assert(otherVector);
  
  const std::vector<double>& otherFeatures = otherVector->values;
  double res = 0.0;
  size_t numFeatures = std::min(values.size(), otherFeatures.size());
  for (size_t i = 0; i < numFeatures; ++i)
    res += values[i] * otherFeatures[i];
    
  const std::vector<DenseVectorPtr>& otherSubVectors = otherVector->subVectors;
  size_t numSubVectors = std::min(subVectors.size(), otherSubVectors.size());
  for (size_t i = 0; i < numSubVectors; ++i)
  {
    DenseVectorPtr d1 = subVectors[i];
    DenseVectorPtr d2 = otherSubVectors[i];
    if (d1 && d2)
      res += d1->denseDotProduct(d2);
  }
  return res;
}

double DenseVector::dotProduct(const FeatureGeneratorPtr featureGenerator) const
{
  assert(featureGenerator);
  const DenseVectorPtr otherVector = featureGenerator.dynamicCast<DenseVector>();
  if (otherVector)
    return denseDotProduct(otherVector);
  else
    return featureGenerator->dotProduct(DenseVectorPtr(const_cast<DenseVector* >(this)));
}

bool DenseVector::load(std::istream& istr)
{
  size_t numSubVectors;
  if (!read(istr, values) || !read(istr, numSubVectors))
    return false;
  subVectors.resize(numSubVectors);
  for (size_t i = 0; i < numSubVectors; ++i)
  {
    bool exists;
    if (!read(istr, exists))
      return false;
    if (exists)
    {
      DenseVectorPtr subVector(new DenseVector());
      if (!subVector->load(istr))
        return false;
      subVectors[i] = subVector;
    }
    else
      subVectors[i] = DenseVectorPtr();
  }
  return true;
}

void DenseVector::save(std::ostream& ostr) const
{
  write(ostr, values);
  write(ostr, subVectors.size());
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    DenseVectorPtr subVector = subVectors[i];
    if (subVector)
    {
      write(ostr, true);
      subVector->save(ostr);
    }
    else
      write(ostr, false);
  }
}
