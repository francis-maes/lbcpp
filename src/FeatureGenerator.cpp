/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 18:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lcpp/EditableFeatureGenerator.h>
#include <lcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>
#include <lcpp/impl/Bridge/DoubleVector.hpp>

#include "FeatureGenerator/EmptyFeatureGenerator.h"
#include "FeatureGenerator/UnitFeatureGenerator.h"
#include "FeatureGenerator/SubFeatureGenerator.h"
#include "FeatureGenerator/WeightedFeatureGenerator.h"
#include "FeatureGenerator/LinearCombinationFeatureGenerator.h"
#include "FeatureGenerator/ExplicitLinearCombinationFeatureGenerator.h"
using namespace lcpp;

/*
** EditableFeatureGenerator
*/
void EditableFeatureGenerator::setDictionary(FeatureDictionaryPtr dictionary)
  {this->dictionary = dictionary;}
  
void EditableFeatureGenerator::ensureDictionary(FeatureDictionaryPtr dictionary)
{
  if (!dictionary)
    return;
  if (this->dictionary)
  {
    if (this->dictionary != dictionary)
    {
      std::cerr << "Error: dictionary mismatch. This dictionary = '" << this->dictionary->getName() << "', "
                   "required dictionary = '" << dictionary->getName() << "'" << std::endl;
      assert(false);
    }
  }
  else
    this->dictionary = dictionary;
}

FeatureDictionaryPtr EditableFeatureGenerator::getDictionary() const
{
  if (!dictionary)
    Object::error(getClassName() + "::getDictionary", "Missing dictionary");
  assert(dictionary);
  return dictionary;
}

/*
** CompositeFeatureGenerator
*/
void CompositeFeatureGenerator::setSubGenerator(size_t index, FeatureGeneratorPtr featureGenerator)
{
  if (featureGenerators.size() < index + 1)
    featureGenerators.resize(index + 1, FeatureGeneratorPtr());
  featureGenerators[index] = featureGenerator;
}

void CompositeFeatureGenerator::appendSubGenerator(FeatureGeneratorPtr featureGenerator)
  {featureGenerators.push_back(featureGenerator);}

void CompositeFeatureGenerator::clear()
  {featureGenerators.clear();}

size_t CompositeFeatureGenerator::getNumSubGenerators() const
  {return featureGenerators.size();}
  
FeatureGeneratorPtr CompositeFeatureGenerator::getSubGenerator(size_t num) const
  {return getSubGeneratorWithIndex(num);}

size_t CompositeFeatureGenerator::getSubGeneratorIndex(size_t num) const
  {assert(num < featureGenerators.size()); return num;}

FeatureGeneratorPtr CompositeFeatureGenerator::getSubGeneratorWithIndex(size_t index) const
  {assert(index < featureGenerators.size()); return featureGenerators[index];}
    
/*
** LazyFeatureVector
*/
size_t LazyFeatureVector::getNumSubGenerators() const
  {return getResult()->getNumSubGenerators();}
  
FeatureGeneratorPtr LazyFeatureVector::getSubGenerator(size_t num) const
  {return getResult()->getSubGenerator(num);}

size_t LazyFeatureVector::getSubGeneratorIndex(size_t num) const
  {return getResult()->getSubGeneratorIndex(num);}

FeatureGeneratorPtr LazyFeatureVector::getSubGeneratorWithIndex(size_t index) const
  {return getResult()->getSubGeneratorWithIndex(index);}

FeatureVectorPtr LazyFeatureVector::getResult() const
{
  if (!result)
    const_cast<LazyFeatureVector* >(this)->result = computeVector();
  return result;
}

/*
** Lazy vectors
*/    
FeatureGeneratorPtr FeatureGenerator::emptyGenerator()
{
  static FeatureGeneratorPtr instance = new EmptyFeatureGenerator();
  return instance;
}
FeatureGeneratorPtr FeatureGenerator::unitGenerator()
{
  static FeatureGeneratorPtr instance = new UnitFeatureGenerator();
  return instance;
}

FeatureGeneratorPtr FeatureGenerator::subFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator)
{
  return new SubFeatureGenerator(dictionary, index, featureGenerator);
}

FeatureGeneratorPtr FeatureGenerator::multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight)
{
  assert(featureGenerator);
  
  // x * 0 = <empty>
  if (weight == 0)
    return FeatureGenerator::emptyGenerator();
  
  // x * 1 = x
  if (weight == 1)
    return featureGenerator;
    
  if (featureGenerator.isInstanceOf<EditableFeatureGenerator>())
  {
    // (k1 * x) * k2 = ((k1 * k2) * x)
    WeightedFeatureGeneratorPtr weighted = featureGenerator.dynamicCast<WeightedFeatureGenerator>();
    if (weighted)
      return weighted->exists()
        ? multiplyByScalar(weighted->getFeatureGenerator(), weight * weighted->getWeight())
        : FeatureGenerator::emptyGenerator();
      
    // k * (sum_i w_i * x_i) = sum_i (w_i * k) * x_i
    LinearCombinationFeatureGeneratorPtr linearCombination = featureGenerator.dynamicCast<LinearCombinationFeatureGenerator>();
    if (linearCombination)
    {
      if (linearCombination->exists())
        return FeatureGenerator::linearCombination(linearCombination->getCompositeFeatureGenerator(),
          multiplyByScalar(linearCombination->getWeights(), weight)->toDenseVector());
      else
        return FeatureGenerator::emptyGenerator();
    }
    
    // k * (composite(x_1, ..., x_n)) = composite(k * x_1, ... k * x_n)
    CompositeFeatureGeneratorPtr composite = featureGenerator.dynamicCast<CompositeFeatureGenerator>();
    if (composite)
    {
      size_t n = composite->getNumSubGenerators();
      if (n)
      {
        CompositeFeatureGeneratorPtr res = new CompositeFeatureGenerator(featureGenerator->getDictionary(), n);
        for (size_t i = 0; i < n; ++i)
          res->setSubGenerator(composite->getSubGeneratorIndex(i), multiplyByScalar(composite->getSubGenerator(i), weight));
        return res;
      }
      else
        return FeatureGenerator::emptyGenerator();
    }
    
    // k * sub(index, x) = sub(index, k * x)
    SubFeatureGeneratorPtr sub = featureGenerator.dynamicCast<SubFeatureGenerator>();
    if (sub)
      return sub->exists()
        ? FeatureGenerator::subFeatureGenerator(sub->getDictionary(), sub->getIndex(), multiplyByScalar(sub->getFeatureGenerator(), weight))
        : FeatureGenerator::emptyGenerator();
  }

  // k * <empty> = <empty>
  if (featureGenerator.dynamicCast<EmptyFeatureGenerator>())
    return FeatureGenerator::emptyGenerator();

  // default: k * x 
  return new WeightedFeatureGenerator(featureGenerator, weight);
}

FeatureGeneratorPtr FeatureGenerator::weightedSum(FeatureGeneratorPtr featureGenerator1, double weight1, FeatureGeneratorPtr featureGenerator2, double weight2, bool computeNow)
{
  if (computeNow)
  {
    if (featureGenerator1->isDense() || featureGenerator2->isDense())
    {
      DenseVectorPtr res = new DenseVector(featureGenerator1->getDictionary());
      res->addWeighted(featureGenerator1, weight1);
      res->addWeighted(featureGenerator2, weight2);
      return res;
    }
    else
    {
      SparseVectorPtr res = new SparseVector(featureGenerator1->getDictionary());
      res->addWeighted(featureGenerator1, weight1);
      res->addWeighted(featureGenerator2, weight2);
      return res;
    }
  }
  else
  {
    std::vector<std::pair<FeatureGeneratorPtr, double> >* combination =
      new std::vector<std::pair<FeatureGeneratorPtr, double> >(2);
    (*combination)[0] = std::make_pair(featureGenerator1, weight1);
    (*combination)[1] = std::make_pair(featureGenerator2, weight2);
    return linearCombination(combination);
  }
}

FeatureGeneratorPtr FeatureGenerator::linearCombination(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights)
{
  return new LinearCombinationFeatureGenerator(compositeFeatureGenerator, weights);
}

FeatureGeneratorPtr FeatureGenerator::linearCombination(std::vector< std::pair<FeatureGeneratorPtr, double> >* newTerms)
{
  return new ExplicitLinearCombinationFeatureGenerator(newTerms);
}
