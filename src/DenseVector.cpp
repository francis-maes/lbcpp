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
using namespace cralgo;

DenseVector& DenseVector::operator =(const DenseVector& otherVector)
{
  features = otherVector.features;
  subVectors = otherVector.subVectors;
  dictionary = otherVector.dictionary;
  return *this;
}

size_t DenseVector::size() const
{
  size_t res = features.size();
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
  lazyVector->addWeightedTo(DenseVectorPtr(this, null_deleter()), weight);
}

void DenseVector::multiplyByScalar(double scalar)
{
  // todo: operation class
  for (size_t i = 0; i < features.size(); ++i)
    features[i] *= scalar;
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
  
  const std::vector<double>& otherFeatures = otherVector->features;
  double res = 0.0;
  size_t numFeatures = std::min(features.size(), otherFeatures.size());
  for (size_t i = 0; i < numFeatures; ++i)
    res += features[i] * otherFeatures[i];
    
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
  const DenseVectorPtr otherVector = boost::dynamic_pointer_cast<DenseVector>(featureGenerator);
  if (otherVector)
    return denseDotProduct(otherVector);
  else
    return featureGenerator->dotProduct(DenseVectorPtr(const_cast<DenseVector* >(this), null_deleter()));
}

bool DenseVector::load(std::istream& istr)
{
  size_t numSubVectors;
  if (!read(istr, features) || !read(istr, numSubVectors))
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
  write(ostr, features);
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
