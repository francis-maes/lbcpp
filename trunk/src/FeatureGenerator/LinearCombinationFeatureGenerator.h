/*-----------------------------------------.---------------------------------.
| Filename: LinearCombinationFeatureGen...h| Linear combination of           |
| Author  : Francis Maes                   |     feature generators          |
| Started : 22/03/2009 17:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_LINEAR_COMBINATION_H_
# define LBCPP_FEATURE_GENERATOR_LINEAR_COMBINATION_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace lbcpp
{

class LinearCombinationFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<LinearCombinationFeatureGenerator, LazyFeatureVector>
{
public:
  LinearCombinationFeatureGenerator(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights)
    : compositeFeatureGenerator(compositeFeatureGenerator), weights(weights)
  {
    jassert(compositeFeatureGenerator && compositeFeatureGenerator->getNumSubGenerators() && compositeFeatureGenerator->getSubGenerator(0));
# ifdef NDEBUG
    setDictionary(compositeFeatureGenerator->getSubGenerator(0)->getDictionary());
# else
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
    {
      FeatureGeneratorPtr sub = compositeFeatureGenerator->getSubGenerator(i);
      jassert(sub);
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

  virtual String toString() const
  {
    String res = "LinearCombination:\n";
    size_t n = compositeFeatureGenerator->getNumSubGenerators();
    for (size_t i = 0; i < n; ++i)
    {
      double weight = weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i));
      if (weight)
        res += "\t" + lbcpp::toString(weight) + " times " + compositeFeatureGenerator->getSubGenerator(i)->toString() + "\n";
    }
    return res;
  }

  virtual void addTo(DenseVectorPtr target) const
    {addWeightedTo(target, 1.0);}
    
  virtual void addTo(SparseVectorPtr target) const
    {addWeightedTo(target, 1.0);}
    
  virtual void substractFrom(DenseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  virtual void substractFrom(SparseVectorPtr target) const
    {addWeightedTo(target, -1.0);}

  virtual void addWeightedTo(DenseVectorPtr target, double weight) const
  {
    if (!weight)
      return;
    if (result)
      result->addWeightedTo(target, weight);
    else
    {
      // y += k * (sum w_i x_i) <==> for each i, y += w_i x_i
      size_t n = compositeFeatureGenerator->getNumSubGenerators();
      for (size_t i = 0; i < n; ++i)
      {
        double w = weight * (weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i)));
        if (w)
          compositeFeatureGenerator->getSubGenerator(i)->addWeightedTo(target, w);
      }
    }
  }
  
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const
  {
    if (!weight)
      return;
    if (result)
      result->addWeightedTo(target, weight);
    else
    {
      // y += k * (sum w_i x_i) <==> for each i, y += w_i x_i
      size_t n = compositeFeatureGenerator->getNumSubGenerators();
      for (size_t i = 0; i < n; ++i)
      {
        double w = weight * weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i));
        if (w)
          compositeFeatureGenerator->getSubGenerator(i)->addWeightedTo(target, w);
      }
    }
  }
  
  virtual double dotProduct(const DenseVectorPtr otherVector) const
  {
    if (result)
      return result->dotProduct(otherVector);
    else
    { 
      // < sum w_i x_i , y > = sum w_i <x_i, y>
      double res = 0.0;
      size_t n = compositeFeatureGenerator->getNumSubGenerators();
      for (size_t i = 0; i < n; ++i)
      {
        double weight = weights->get(compositeFeatureGenerator->getSubGeneratorIndex(i));
        if (weight)
          res += compositeFeatureGenerator->getSubGenerator(i)->dotProduct(otherVector) * weight;
      }
      return res;
    }
  }  
    
private:
  FeatureGeneratorPtr compositeFeatureGenerator;
  DenseVectorPtr weights;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_LINEAR_COMBINATION_H_
