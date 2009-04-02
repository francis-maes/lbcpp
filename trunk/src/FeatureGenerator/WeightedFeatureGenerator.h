/*-----------------------------------------.---------------------------------.
| Filename: WeightedFeatureGenerator.h     | Weighted Feature-Generator      |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_FEATURE_GENERATOR_WEIGHTED_H_
# define LCPP_FEATURE_GENERATOR_WEIGHTED_H_

# include <lcpp/FeatureGenerator.h>
# include <lcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace lcpp
{

class WeightedFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<WeightedFeatureGenerator, LazyFeatureVector>
{
public:
  typedef FeatureGeneratorDefaultImplementations<WeightedFeatureGenerator, LazyFeatureVector> BaseClass;
  
  WeightedFeatureGenerator(FeatureGeneratorPtr featureGenerator, double weight)
    : BaseClass(featureGenerator->getDictionary()), featureGenerator(featureGenerator), weight(weight) {}
  
  /*
  ** Accessors
  */
  bool exists() const
    {return featureGenerator && weight;}
    
  FeatureGeneratorPtr getFeatureGenerator() const
    {return featureGenerator;}
  
  double getWeight() const
    {return weight;}

  /*
  ** LazyFeatureVector
  */
  virtual FeatureVectorPtr computeVector() const
  {
    if (isDense())
    {
      DenseVectorPtr res = new DenseVector(getDictionary());
      res->addWeighted(featureGenerator, weight);
      return res;
    }
    else
    {
      SparseVectorPtr res = new SparseVector(getDictionary());
      res->addWeighted(featureGenerator, weight);
      return res;
    }
  }
  
  /*
  ** EditableFeatureGenerator
  */
  virtual void clear()
    {featureGenerator = FeatureGeneratorPtr(); weight = 0.0;}

  /*
  ** FeatureGenerator
  */
  virtual bool isDense() const
    {return featureGenerator->isDense();}

  virtual size_t getNumSubGenerators() const
    {return exists() ? featureGenerator->getNumSubGenerators() : 0;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {return exists() ? FeatureGenerator::multiplyByScalar(featureGenerator->getSubGenerator(num), weight) : FeatureGenerator::emptyGenerator();}

  virtual size_t getSubGeneratorIndex(size_t num) const
    {return exists() ? featureGenerator->getSubGeneratorIndex(num) : (size_t)-1;}

  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return exists() ? FeatureGenerator::multiplyByScalar(featureGenerator->getSubGeneratorWithIndex(index), weight) : FeatureGenerator::emptyGenerator();}

  virtual std::string toString() const
    {return lcpp::toString(weight) + " * " + featureGenerator->toString();}
    
  virtual size_t l0norm() const
    {return exists() ? featureGenerator->l0norm() : 0;}
    
  virtual double l1norm() const
    {return exists() ? featureGenerator->l1norm() * fabs(weight) : 0.0;}
  
  virtual double sumOfSquares() const
    {return exists() ? featureGenerator->l1norm() * weight * weight : 0.0;}
  
  virtual void addTo(DenseVectorPtr target) const
    {if (exists()) featureGenerator->addWeightedTo(target, weight);}
  
  virtual void substractFrom(DenseVectorPtr target) const
    {if (exists()) featureGenerator->addWeightedTo(target, -weight);}
    
  virtual void addWeightedTo(DenseVectorPtr target, double weight) const
    {if (exists()) featureGenerator->addWeightedTo(target, weight * this->weight);}
    
  virtual void addWeightedTo(SparseVectorPtr target, double weight) const
    {if (exists()) featureGenerator->addWeightedTo(target, weight * this->weight);}

  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const
    {if (exists()) featureGenerator->addWeightedSignsTo(target, weight * (this->weight > 0 ? 1.0 : -1.0));}
  
  virtual double dotProduct(const DenseVectorPtr vector) const
    {return exists() ? featureGenerator->dotProduct(vector) * weight : 0.0;}

private:
  FeatureGeneratorPtr featureGenerator;
  double weight;
};

}; /* namespace lcpp */

#endif // !LCPP_FEATURE_GENERATOR_WEIGHTED_H_
