/*-----------------------------------------.---------------------------------.
| Filename: SubFeatureGenerator.h          | Sub Feature-Generator           |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_SUB_H_
# define LBCPP_FEATURE_GENERATOR_SUB_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/FeatureGenerator/impl/FeatureGeneratorDefaultImplementations.hpp>

namespace lbcpp
{

class SubFeatureGenerator :
  public FeatureGeneratorDefaultImplementations<SubFeatureGenerator, EditableFeatureGenerator>
{
public:
  typedef FeatureGeneratorDefaultImplementations<SubFeatureGenerator, EditableFeatureGenerator> BaseClass;
  
  SubFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator)
    : BaseClass(dictionary), index(index), featureGenerator(featureGenerator)
  {
    dictionary->ensureSubDictionary(index, featureGenerator->getDictionary());
  }
  
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
  void staticFeatureGenerator(FeatureVisitor& visitor) const
    {if (exists()) visitor.featureCall_(featureGenerator, dictionary, index);}

  virtual SparseVectorPtr toSparseVector() const
  {
    SparseVectorPtr res = new SparseVector(getDictionary());
    if (exists())
      res->setSubVector(index, featureGenerator->toSparseVector());
    return res;
  }
  
  virtual DenseVectorPtr toDenseVector() const
  {
    DenseVectorPtr res = new DenseVector(getDictionary());
    if (exists())
      res->setSubVector(index, featureGenerator->toDenseVector());
    return res;
  }
  
  virtual String toString() const
  {
    String res = "subFeatureGenerator";
    if (exists())
      res += "(" + getDictionary()->getScopes()->getString(index) + ", " + featureGenerator->toString() + ")";
    return res;
  }
  
  virtual size_t l0norm() const
    {return exists() ? featureGenerator->l0norm() : 0;}
    
  virtual double l1norm() const
    {return exists() ? featureGenerator->l1norm() : 0;}
    
  virtual double sumOfSquares() const
    {return exists() ? featureGenerator->sumOfSquares() : 0;}
    
  virtual void addTo(DenseVectorPtr target) const
    {if (exists()) featureGenerator->addTo(getSubVector(target));}
  
  virtual void addTo(SparseVectorPtr target) const
    {if (exists()) featureGenerator->addTo(getSubVector(target));}

  virtual void substractFrom(DenseVectorPtr target) const
    {if (exists()) featureGenerator->substractFrom(getSubVector(target));}

  virtual void substractFrom(SparseVectorPtr target) const
    {if (exists()) featureGenerator->substractFrom(getSubVector(target));}

  virtual void addWeightedTo(DenseVectorPtr target, double weight) const
    {if (exists()) featureGenerator->addWeightedTo(getSubVector(target), weight);}

  virtual void addWeightedTo(SparseVectorPtr target, double weight) const
    {if (exists()) featureGenerator->addWeightedTo(getSubVector(target), weight);}

  virtual void addWeightedSignsTo(DenseVectorPtr target, double weight) const
    {if (exists()) featureGenerator->addWeightedSignsTo(getSubVector(target), weight);}

  virtual double dotProduct(const DenseVectorPtr vector) const
    {return exists() ? featureGenerator->dotProduct(getSubVector(vector)) : 0.0;}
  
private:
  size_t index;
  FeatureGeneratorPtr featureGenerator;

  DenseVectorPtr getSubVector(DenseVectorPtr vector) const
  {
    jassert(exists());
    DenseVectorPtr& res = vector->getSubVector(index);
    if (!res)
      res = new DenseVector(featureGenerator->getDictionary());
    return res;
  }
  
  SparseVectorPtr getSubVector(SparseVectorPtr vector) const
  {
    jassert(exists());
    SparseVectorPtr& res = vector->getSubVector(index);
    if (!res)
      res = new SparseVector(featureGenerator->getDictionary());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_SUB_H_
