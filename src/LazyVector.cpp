/*-----------------------------------------.---------------------------------.
| Filename: LazyVector.cpp                 | Lazy Vectors                    |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 18:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/LazyVector.h>
using namespace cralgo;

LazyVector::LazyVector(FeatureDictionaryPtr dictionary)
{
  this->dictionary = dictionary;
}

void LazyVector::clear()
{
  featureGenerator = FeatureGeneratorPtr();
  sparseVector = SparseVectorPtr();
  denseVector = DenseVectorPtr();
  linearCombination.clear();
  subVectors.clear();
}

FeatureDictionaryPtr LazyVector::getDictionary() const
{
  if (dictionary)
    return dictionary;
  static FeatureDictionaryPtr defaultDictionary = new FeatureDictionary("LazyVector");
  return defaultDictionary;
}

LazyVectorPtr& LazyVector::getSubVector(size_t i, bool createIfMissing)
{
  if (i >= subVectors.size())
    subVectors.resize(i + 1, LazyVectorPtr());
  LazyVectorPtr& res = subVectors[i];
  if (!res && createIfMissing)
    res = new LazyVector();
  return res;
}

void LazyVector::addWeighted(const LazyVectorPtr vector, double weight)
{
  assert(vector);
  if (!weight)
    return;
  if (vector->featureGenerator)
    addInCombination(vector->featureGenerator, weight);
  if (vector->sparseVector)
    addInCombination(vector->sparseVector, weight);
  if (vector->denseVector)
    addInCombination(vector->denseVector, weight);
  for (LinearCombinationMap::const_iterator it = vector->linearCombination.begin(); it != vector->linearCombination.end(); ++it)
    addInCombination(it->first, it->second * weight);
  for (size_t i = 0; i < vector->subVectors.size(); ++i)
  {
    LazyVectorPtr otherSubVector = vector->subVectors[i];
    if (otherSubVector)
    {
      LazyVectorPtr& thisSubVector = getSubVector(i);
      if (!thisSubVector)
      {
        thisSubVector = new LazyVector();
        if (otherSubVector->hasDictionary())
          thisSubVector->setDictionary(otherSubVector->getDictionary());
      }
      thisSubVector->addWeighted(otherSubVector, weight);
    }
  }
}

void LazyVector::multiplyByScalar(double scalar)
{
  if (scalar == 0)
    {clear(); return;}
  else if (scalar == 1)
    return;

  ensureInCombination(featureGenerator); // feature generators cannot be multiplied by scalars directly
  for (LinearCombinationMap::iterator it = linearCombination.begin(); it != linearCombination.end(); ++it)
    it->second *= scalar;
  if (sparseVector)
    sparseVector->multiplyByScalar(scalar);
  if (denseVector)
    denseVector->multiplyByScalar(scalar);
  for (size_t i = 0; i < subVectors.size(); ++i)
    subVectors[i]->multiplyByScalar(scalar);
}

void LazyVector::addWeightedTo(DenseVectorPtr target, double weight) const
{
  if (featureGenerator)
    featureGenerator->addWeightedTo(target, weight);
  if (sparseVector)
    sparseVector->addWeightedTo(target, weight);
  if (denseVector)
    denseVector->addWeightedTo(target, weight);
  for (LinearCombinationMap::const_iterator it = linearCombination.begin(); it != linearCombination.end(); ++it)
    it->first->addWeightedTo(target, weight * it->second);
  for (size_t i = 0; i < subVectors.size(); ++i)
  {
    LazyVectorPtr subVector = subVectors[i];
    if (subVector)
    {
      DenseVectorPtr& subTarget = target->getSubVector(i);
      if (!subTarget)
      {
        subTarget = new DenseVector();
        if (subVector->hasDictionary())
          subTarget->setDictionary(subVector->getDictionary());
      }
      subVector->addWeightedTo(subTarget, weight);
    }
  }
}

void LazyVector::storeWithDenseVector()
{
  // put the sparse vector and the feature generator into the combination
  ensureInCombination(sparseVector);
  ensureInCombination(featureGenerator);
  
  // put the sub vectors into the combination
  if (subVectors.size())
  {
    DenseVectorPtr sub = dictionary ? new DenseVector(dictionary) : new DenseVector();
    for (size_t i = 0; i < subVectors.size(); ++i)
    {
      LazyVectorPtr subLazy = subVectors[i];
      if (subLazy)
        sub->setSubVector(i, subLazy->toDenseVector());
    }
    addInCombination(sub);
    subVectors.clear();
  }
  
  // eventually create the dense vector
  if (!denseVector)
    denseVector = dictionary ? new DenseVector(dictionary) : new DenseVector();
    
  // compute the linear combination
  for (LinearCombinationMap::const_iterator it = linearCombination.begin(); it != linearCombination.end(); ++it)
    denseVector->addWeighted(it->first, it->second);
  linearCombination.clear();
}

void LazyVector::storeWithSparseVector()
{
  // put the feature generator and the dense vector into the combination
  ensureInCombination(featureGenerator);
  if (denseVector)
  {
    std::cerr << "Performance warning: converting a dense vector into a sparse vector, this is not very efficient." << std::endl;
    ensureInCombination(denseVector);
    denseVector = DenseVectorPtr();
  }
  
  // put the sub vectors into the combination
  if (subVectors.size())
  {
    SparseVectorPtr sub = dictionary ? new SparseVector(dictionary) : new SparseVector();
    for (size_t i = 0; i < subVectors.size(); ++i)
    {
      LazyVectorPtr subLazy = subVectors[i];
      if (subLazy)
      {
        subLazy->storeWithSparseVector();
        sub->setSubVector(i, subLazy->sparseVector);
      }
    }
    addInCombination(sub);
    subVectors.clear();
  }
  
  // eventually create the sparse vector
  if (!sparseVector)
    sparseVector = dictionary ? new SparseVector(dictionary) : new SparseVector();
    
  // compute the linear combination
  for (LinearCombinationMap::const_iterator it = linearCombination.begin(); it != linearCombination.end(); ++it)
    sparseVector->addWeighted(it->first, it->second);
  linearCombination.clear();
}

bool LazyVector::isStoredWithFeatureGenerator() const
{
  return featureGenerator && !sparseVector && !denseVector && linearCombination.empty() && subVectors.empty();
}

bool LazyVector::isStoredWithDenseVector() const
{
  return !featureGenerator && !sparseVector && denseVector && linearCombination.empty() && subVectors.empty();
}

bool LazyVector::isStoredWithSparseVector() const
{
  return !featureGenerator && sparseVector && !denseVector && linearCombination.empty() && subVectors.empty();
}

bool LazyVector::guessIfDense() const
{
  if (denseVector || isStoredWithDenseVector())
    return true;
  if (isStoredWithSparseVector() || isStoredWithFeatureGenerator())
    return false;    
  if (linearCombination.size())
  {
    for (LinearCombinationMap::const_iterator it = linearCombination.begin(); it != linearCombination.end(); ++it)
      if (it->first.isInstanceOf<DenseVector>())
        return true;
  }
  else
  {
    for (size_t i = 0; i < subVectors.size(); ++i)
      if (subVectors[i] && subVectors[i]->guessIfDense())
        return true;
  }
  return false;
}
