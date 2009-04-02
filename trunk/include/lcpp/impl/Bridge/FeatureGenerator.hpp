/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.hpp           | Static to Dynamic Features      |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 13/02/2009 17:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_STATIC_FEATURE_GENERATOR_HPP_
# define LCPP_STATIC_FEATURE_GENERATOR_HPP_

# include "FeatureGeneratorDefaultImplementations.hpp"
# include "../../SparseVector.h"
# include "../../DenseVector.h"

namespace lcpp
{
  
template<class ImplementationType>
class StaticToDynamicFeatureGenerator : public 
  FeatureGeneratorDefaultImplementations< StaticToDynamicFeatureGenerator< ImplementationType >, FeatureGenerator >
{
public:
  typedef StaticToDynamicFeatureGenerator<ImplementationType> ThisClass;
  typedef FeatureGeneratorDefaultImplementations< StaticToDynamicFeatureGenerator< ImplementationType >, FeatureGenerator > BaseClass;

  StaticToDynamicFeatureGenerator(const ImplementationType& impl)
    : impl(impl) {}
    
  virtual std::string getName() const
    {return ImplementationType::getName();}

  virtual FeatureDictionaryPtr getDictionary() const
    {return ImplementationType::getDictionary();}

  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor) const
    {const_cast<ImplementationType& >(impl).featureGenerator(visitor, getDictionary());}

  virtual SparseVectorPtr toSparseVector() const
  {
    if (!sparseVector)
      const_cast<ThisClass* >(this)->sparseVector = BaseClass::toSparseVector();
    return sparseVector;
  }

  virtual DenseVectorPtr toDenseVector() const
  {
    if (!denseVector)
      const_cast<ThisClass* >(this)->denseVector = BaseClass::toDenseVector();
    return denseVector;
  }

  virtual size_t getNumSubGenerators() const
  {
    if (denseVector)
      return denseVector->getNumSubGenerators();
    else
      return toSparseVector()->getNumSubGenerators();
  }
  
  virtual FeatureGeneratorPtr getSubGenerator(size_t num) const
  {
    if (denseVector)
      return denseVector->getSubGenerator(num);
    else
      return toSparseVector()->getSubGenerator(num);
  }

  virtual size_t getSubGeneratorIndex(size_t num) const
  {
    if (denseVector)
      return denseVector->getSubGeneratorIndex(num);
    else
      return toSparseVector()->getSubGeneratorIndex(num);
  }
  
  virtual FeatureGeneratorPtr getSubGeneratorWithIndex(size_t index) const
  {
    if (denseVector)
      return denseVector->getSubGeneratorWithIndex(index);
    else
      return toSparseVector()->getSubGeneratorWithIndex(index);
  }

private:
  ImplementationType impl;
  SparseVectorPtr sparseVector;
  DenseVectorPtr denseVector;
};

template<class ImplementationType>
FeatureGeneratorPtr staticToDynamicFeatureGenerator(const ImplementationType& impl)
  {return FeatureGeneratorPtr(new StaticToDynamicFeatureGenerator<ImplementationType>(impl));}

}; /* namespace lcpp */

#endif // !LCPP_STATIC_FEATURE_GENERATOR_HPP_
