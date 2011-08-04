/*-----------------------------------------.---------------------------------.
| Filename: DenseVector.cpp                | Composite Dense Vectors         |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2009 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/FeatureGenerator/DenseVector.h>
#include <lbcpp/FeatureGenerator/impl/DoubleVector.hpp>
#include <cfloat>
using namespace lbcpp;

DenseVector::DenseVector(const DenseVector& otherVector)
  : BaseClass(otherVector.getDictionary()), values(otherVector.values), subVectors(otherVector.subVectors.size())
{
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    DenseVectorPtr otherSubVector = otherVector.subVectors[i];
    if (otherSubVector)
      subVectors[i] = new DenseVector(*otherSubVector);
  }
}

DenseVector::DenseVector(FeatureDictionaryPtr dictionary, const std::vector<double>& values)
  : BaseClass(dictionary), values(values) {}

DenseVector::DenseVector(FeatureDictionaryPtr dictionary, size_t initialNumValues, size_t initialNumSubVectors)
  : BaseClass(dictionary)
{
  if (initialNumValues > 0)
    values.resize(initialNumValues, 0.0);
  if (initialNumSubVectors > 0)
    subVectors.resize(initialNumSubVectors, DenseVectorPtr());
}

FeatureDictionaryPtr DenseVector::getDictionary() const
{
  if (dictionary)
    return dictionary;
  static FeatureDictionaryPtr defaultDictionary;
  if (defaultDictionary)
    defaultDictionary = FeatureDictionaryManager::getInstance().getOrCreateRootDictionary(T("DenseVector"), true, true);
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

void DenseVector::initializeRandomly(double mean, double standardDeviation)
{
  RandomGenerator& random = RandomGenerator::getInstance();
  
  jassert(dictionary);
  values.resize(dictionary->getNumFeatures());
  for (size_t i = 0; i < values.size(); ++i)
    values[i] = random.sampleDoubleFromGaussian(mean, standardDeviation);
  subVectors.resize(dictionary->getNumScopes());
  for (size_t i = 0; i < subVectors.size(); ++i)
  { 
    DenseVectorPtr subVector = new DenseVector(dictionary->getSubDictionary(i));
    subVector->initializeRandomly(mean, standardDeviation);
    subVectors[i] = subVector;
  }
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

ObjectPtr DenseVector::multiplyByScalar(double scalar)
{
  if (scalar == 0)
    clear();
  else
  {
    for (size_t i = 0; i < values.size(); ++i)
      values[i] *= scalar;
    for (size_t i = 0; i < subVectors.size(); ++i)
    {
      DenseVectorPtr subVector = subVectors[i];
      if (subVector)
        subVector->multiplyByScalar(scalar);
    }
  }
  return ObjectPtr(this);
}

double DenseVector::denseDotProduct(const DenseVectorPtr otherVector) const
{
  jassert(otherVector);
  
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

double DenseVector::dotProduct(const FeatureGeneratorPtr featureGenerator, DotProductCache* cache) const
{
  jassert(featureGenerator);
  const DenseVectorPtr otherVector = featureGenerator.dynamicCast<DenseVector>();
  if (otherVector)
    return denseDotProduct(otherVector); // todo: use cache also in the dense vector dot product
  else
    return featureGenerator->dotProduct(DenseVectorPtr(const_cast<DenseVector* >(this)), cache);
}

bool DenseVector::load(InputStream& istr)
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
    set(featureIndex, featureValue);
  }

  size_t numSubVectors;
  if (!read(istr, numSubVectors))
    return false;

  subVectors.clear();
  subVectors.reserve(numSubVectors);
  for (size_t i = 0; i < numSubVectors; ++i)
  {
    size_t scopeIndex;
    DenseVectorPtr subVector;
    if (!scopes->readIdentifier(istr, scopeIndex) || !read(istr, subVector))
      return false;
    setSubVector(scopeIndex, subVector);
  }
  return true;
}

void DenseVector::save(OutputStream& ostr) const
{
  jassert(dictionary);
  write(ostr, dictionary->getName());
  StringDictionaryPtr features = dictionary->getFeatures();
  StringDictionaryPtr scopes = dictionary->getScopes();  

  write(ostr, values.size());
  for (size_t i = 0; i < values.size(); ++i)
  {
    features->writeIdentifier(ostr, i);
    write(ostr, values[i]);
  }

  write(ostr, subVectors.size());
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    scopes->writeIdentifier(ostr, i);
    write(ostr, subVectors[i]);
  }
}