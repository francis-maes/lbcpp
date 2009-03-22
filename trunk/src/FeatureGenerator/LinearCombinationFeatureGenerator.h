/*-----------------------------------------.---------------------------------.
| Filename: LinearCombinationFeatureGen...h| Linear combination of           |
| Author  : Francis Maes                   |     feature generators          |
| Started : 22/03/2009 17:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_GENERATOR_LINEAR_COMBINATION_H_
# define CRALGO_FEATURE_GENERATOR_LINEAR_COMBINATION_H_

# include <cralgo/FeatureGenerator.h>
# include <cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace cralgo
{

class LinearCombinationFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<LinearCombinationFeatureGenerator, LazyFeatureVector>
{
public:
  LinearCombinationFeatureGenerator(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights)
    : compositeFeatureGenerator(compositeFeatureGenerator), weights(weights)
  {
    assert(compositeFeatureGenerator && compositeFeatureGenerator->getNumSubGenerators() && compositeFeatureGenerator->getSubGenerator(0));
# ifdef NDEBUG
    setDictionary(compositeFeatureGenerator->getSubGenerator(0)->getDictionary());
# else
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
    {
      FeatureGeneratorPtr sub = compositeFeatureGenerator->getSubGenerator(i);
      assert(sub);
      ensureDictionary(sub->getDictionary());
    }
# endif
  }
  
  /*
  ** Accessors
  */
  bool exists() const
    {return compositeFeatureGenerator && weights;}
  
  FeatureGeneratorPtr getCompositeFeatureGenerator() const
    {return compositeFeatureGenerator;}
    
  DenseVectorPtr getWeights() const
    {return weights;}
  
  
  /*
  ** LazyFeatureVector
  */
  virtual FeatureVectorPtr computeVector() const
  {
    if (isDense())
    {
      DenseVectorPtr res = new DenseVector(getDictionary());
      addTo(res);
      return res;
    }
    else
    {
      SparseVectorPtr res = new SparseVector(getDictionary());
      addTo(res);
      return res;
    }
  }
  
  /*
  ** EditableFeatureGenerator
  */
  virtual void clear()
    {compositeFeatureGenerator = FeatureGeneratorPtr(); weights = DenseVectorPtr();}
  
  /*
  ** FeatureGenerator
  */  
  virtual bool isDense() const
    {return compositeFeatureGenerator->isDense() || weights->size() > 20;}

  virtual std::string toString(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    std::string res = "LinearCombination:\n";
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
      res += "\t" + cralgo::toString(weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i)))
          + " x " + compositeFeatureGenerator->getSubGenerator(i)->toString() + "\n";
    return res;
  }

  virtual void addTo(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {addWeightedTo(target, 1.0, dictionary);}
    
  virtual void addTo(SparseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {addWeightedTo(target, 1.0, dictionary);}
    
  virtual void substractFrom(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {addWeightedTo(target, -1.0, dictionary);}

  virtual void substractFrom(SparseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {addWeightedTo(target, -1.0, dictionary);}

  virtual void addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
    {
      double w = weight * (weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i)));
      compositeFeatureGenerator->getSubGenerator(i)->addWeightedTo(target, w, dictionary);
    }
  }
  
  virtual void addWeightedTo(SparseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
    {
      double w = weight * weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i));
      compositeFeatureGenerator->getSubGenerator(i)->addWeightedTo(target, w, dictionary);
    }
  }
  
  virtual double dotProduct(const DenseVectorPtr vector, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    double res = 0.0;
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
      res += compositeFeatureGenerator->getSubGenerator(i)->dotProduct(vector, dictionary) *
        weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i));
    return res;
  }  
    
private:
  FeatureGeneratorPtr compositeFeatureGenerator;
  DenseVectorPtr weights;
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_LINEAR_COMBINATION_H_
