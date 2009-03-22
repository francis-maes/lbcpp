/*-----------------------------------------.---------------------------------.
| Filename: LazyVector.cpp                 | Lazy Vectors                    |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 18:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/LazyFeatureGenerators.h>
#include <cralgo/impl/impl.h> // FeatureGeneratorDefaultImplementations
using namespace cralgo;

/*
** EmptyFeatureGenerator
*/
class EmptyFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<EmptyFeatureGenerator, FeatureGenerator>
{
public:
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
    {}
    
  FeatureDictionaryPtr getDictionary() const
  {
    static FeatureDictionaryPtr dictionary = new FeatureDictionary("empty");
    return dictionary;
  }

  virtual size_t getNumSubGenerators() const
    {return 0;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t index) const
    {return FeatureGeneratorPtr();}
};

FeatureGeneratorPtr FeatureGenerator::getEmptyGenerator()
{
  static FeatureGeneratorPtr instance = new EmptyFeatureGenerator();
  return instance;
}

/*
** UnitFeatureGenerator
*/
class UnitFeatureGenerator : public FeatureGeneratorDefaultImplementations<UnitFeatureGenerator, FeatureGenerator>
{
public:
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
    {visitor.featureSense_(dictionary, (size_t)0);}
    
  FeatureDictionaryPtr getDictionary() const
  {
    static FeatureDictionaryPtr dictionary = createDictionary();
    return dictionary;
  }
  
  virtual size_t getNumSubGenerators() const
    {return 0;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t index) const
    {return FeatureGeneratorPtr();}

private:
  static FeatureDictionaryPtr createDictionary()
  {
    FeatureDictionaryPtr res = new FeatureDictionary("unit");
    res->getFeatures().add("unit");
    return res;
  }
};

FeatureGeneratorPtr FeatureGenerator::getUnitGenerator()
{
  static FeatureGeneratorPtr instance = new UnitFeatureGenerator();
  return instance;
}

/*
** WeightedFeatureGenerator
*/
FeatureGeneratorPtr cralgo::multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight)
{
  assert(featureGenerator);
  
  // x * 1 = x
  if (weight == 1)
    return featureGenerator;

  // (k1 * x) * k2 = ((k1 * k2) * x)
  WeightedFeatureGeneratorPtr weighted = featureGenerator.dynamicCast<WeightedFeatureGenerator>();
  if (weighted)
    return weighted->exists()
      ? multiplyByScalar(weighted->getFeatureGenerator(), weight * weighted->getWeight())
      : FeatureGenerator::getEmptyGenerator();
    
  // k * (sum_i w_i * x_i) = sum_i (w_i * k) * x_i
  LinearCombinationFeatureGeneratorPtr linearCombination = featureGenerator.dynamicCast<LinearCombinationFeatureGenerator>();
  if (linearCombination)
  {
    if (linearCombination->getNumElements())
    {
      LinearCombinationFeatureGeneratorPtr res = new LinearCombinationFeatureGenerator(featureGenerator->getDictionary());
      for (LinearCombinationFeatureGenerator::const_iterator it = linearCombination->begin(); it != linearCombination->end(); ++it)
        res->addWeighted(it->first, it->second * weight);
      return res;
    }
    else
      return FeatureGenerator::getEmptyGenerator();
  }
  
  // k * (composite(x_1, ..., x_n)) = composite(k * x_1, ... k * x_n)
  CompositeFeatureGeneratorPtr composite = featureGenerator.dynamicCast<CompositeFeatureGenerator>();
  if (composite)
  {
    size_t n = composite->getNumSubGenerators();
    if (n)
    {
      CompositeFeatureGeneratorPtr res = new CompositeFeatureGenerator(n, featureGenerator->getDictionary());
      for (size_t i = 0; i < n; ++i)
        res->setSubGenerator(i, multiplyByScalar(composite->getSubGenerator(i), weight));
      return res;
    }
    else
      return FeatureGenerator::getEmptyGenerator();
  }
  
  // k * sub(index, x) = sub(index, k * x)
  SubFeatureGeneratorPtr sub = featureGenerator.dynamicCast<SubFeatureGenerator>();
  if (sub)
    return sub->exists()
      ? FeatureGeneratorPtr(new SubFeatureGenerator(sub->getIndex(), multiplyByScalar(sub->getFeatureGenerator(), weight)))
      : FeatureGenerator::getEmptyGenerator();

  // k * <empty> = <empty>
  if (featureGenerator.dynamicCast<EmptyFeatureGenerator>())
    return FeatureGenerator::getEmptyGenerator();

  // default: k * x 
  return new WeightedFeatureGenerator(featureGenerator, weight);
}

#if 0
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
        thisSubVector = new LazyVector(otherSubVector->getDictionary());
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
        subTarget = new DenseVector(subVector->getDictionary());
      subVector->addWeightedTo(subTarget, weight);
    }
  }
}

size_t LazyVector::l0norm() const
{
  if (isStoredWithFeatureGenerator())
    return featureGenerator->l0norm();
  else if (guessIfDense())
  {
    const_cast<LazyVector* >(this)->storeWithDenseVector();
    return denseVector->l0norm();
  }
  else
  {
    const_cast<LazyVector* >(this)->storeWithSparseVector();
    return sparseVector->l0norm();
  }
}

double LazyVector::l1norm() const
{
  if (isStoredWithFeatureGenerator())
    return featureGenerator->l1norm();
  else if (guessIfDense())
  {
    const_cast<LazyVector* >(this)->storeWithDenseVector();
    return denseVector->l1norm();
  }
  else
  {
    const_cast<LazyVector* >(this)->storeWithSparseVector();
    return sparseVector->l1norm();
  }
}

double LazyVector::sumOfSquares() const
{
  if (isStoredWithFeatureGenerator())
    return featureGenerator->sumOfSquares();
  else if (guessIfDense())
  {
    const_cast<LazyVector* >(this)->storeWithDenseVector();
    return denseVector->sumOfSquares();
  }
  else
  {
    const_cast<LazyVector* >(this)->storeWithSparseVector();
    return sparseVector->sumOfSquares();
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
    SparseVectorPtr sub = new SparseVector(getDictionary());
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
    sparseVector = new SparseVector(getDictionary());
    
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
#endif // 0
