/*-----------------------------------------.---------------------------------.
| Filename: SubFeatureGenerator.h          | Sub Feature-Generator           |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_GENERATOR_SUB_H_
# define CRALGO_FEATURE_GENERATOR_SUB_H_

# include <cralgo/FeatureGenerator.h>
# include <cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace cralgo
{

class SubFeatureGenerator :
  public FeatureGeneratorDefaultImplementations<SubFeatureGenerator, EditableFeatureGenerator>
{
public:
  typedef FeatureGeneratorDefaultImplementations<SubFeatureGenerator, EditableFeatureGenerator> BaseClass;
  
  SubFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator)
    : BaseClass(dictionary), index(index), featureGenerator(featureGenerator) {}
  
  /*
  ** Accessors
  */
  bool exists() const
    {return index != (size_t)-1;}
  
  size_t getIndex() const
    {return index;}
    
  FeatureGeneratorPtr getFeatureGenerator() const
    {return featureGenerator;}

  /*
  ** EditableFeatureGenerator
  */
  virtual void clear()
    {index = (size_t)-1; featureGenerator = FeatureGeneratorPtr();}
  
  /*
  ** FeatureGeneraor
  */
  virtual bool isDense() const
    {return featureGenerator->isDense();}

  virtual size_t getNumSubGenerators() const
    {return 1;}
    
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
    {return num == 0 ? featureGenerator : FeatureGeneratorPtr();}

  virtual size_t getSubGeneratorIndex(size_t num) const
    {return num == 0 ? index : (size_t)-1;}

  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
    {return index == this->index ? featureGenerator : FeatureGeneratorPtr();}

  template<class FeatureVisitor>
  void staticFeatureGenerator(FeatureVisitor& visitor, FeatureDictionaryPtr featureDictionary) const
    {if (exists()) visitor.featureCall(dictionary, index, featureGenerator);}

  virtual SparseVectorPtr toSparseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    SparseVectorPtr res = new SparseVector(getDictionary());
    if (exists())
      res->setSubVector(index, featureGenerator->toSparseVector());
    return res;
  }
  
  virtual DenseVectorPtr toDenseVector(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    DenseVectorPtr res = new DenseVector(getDictionary());
    if (exists())
      res->setSubVector(index, featureGenerator->toDenseVector());
    return res;
  }
  
  virtual std::string toString(FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
  {
    std::string res = "subFeatureGenerator";
    if (exists())
      res += "(" + getDictionary()->getFeatures().getString(0) + ", " + featureGenerator->toString() + ")";
    return res;
  }
  
  virtual size_t l0norm() const
    {return exists() ? featureGenerator->l0norm() : 0;}
    
  virtual double l1norm() const
    {return exists() ? featureGenerator->l1norm() : 0;}
    
  virtual double sumOfSquares() const
    {return exists() ? featureGenerator->sumOfSquares() : 0;}
    
  virtual void addTo(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->addTo(getSubVector(target));}
  
  virtual void addTo(SparseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->addTo(getSubVector(target));}

  virtual void substractFrom(DenseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->substractFrom(getSubVector(target));}

  virtual void substractFrom(SparseVectorPtr target, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->substractFrom(getSubVector(target));}

  virtual void addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->addWeightedTo(getSubVector(target), weight);}

  virtual void addWeightedTo(SparseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->addWeightedTo(getSubVector(target), weight);}

  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {if (exists()) featureGenerator->addWeightedSignsTo(getSubVector(target), weight);}

  virtual double dotProduct(const DenseVectorPtr vector, FeatureDictionaryPtr dictionary = FeatureDictionaryPtr()) const
    {return exists() ? featureGenerator->dotProduct(getSubVector(vector)) : 0.0;}
  
private:
  size_t index;
  FeatureGeneratorPtr featureGenerator;

  DenseVectorPtr getSubVector(DenseVectorPtr vector) const
  {
    assert(exists());
    DenseVectorPtr& res = vector->getSubVector(index);
    if (!res)
      res = new DenseVector(featureGenerator->getDictionary());
    return res;
  }
  
  SparseVectorPtr getSubVector(SparseVectorPtr vector) const
  {
    assert(exists());
    SparseVectorPtr& res = vector->getSubVector(index);
    if (!res)
      res = new SparseVector(featureGenerator->getDictionary());
    return res;
  }
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_SUB_H_
