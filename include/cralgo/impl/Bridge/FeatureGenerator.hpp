/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.hpp           | Static to Dynamic Features      |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 13/02/2009 17:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_FEATURE_GENERATOR_HPP_
# define CRALGO_STATIC_FEATURE_GENERATOR_HPP_

# include "FeatureGeneratorDefaultImplementations.hpp"

namespace cralgo
{
  
template<class ImplementationType>
class StaticToDynamicFeatureGenerator : public 
  FeatureGeneratorDefaultImplementations< StaticToDynamicFeatureGenerator< ImplementationType >, FeatureGenerator >
{
public:
  StaticToDynamicFeatureGenerator(const ImplementationType& impl)
    : impl(impl) {}
    
  virtual std::string getName() const
    {return ImplementationType::getName();}

  virtual FeatureDictionary& getDefaultDictionary() const
    {return ImplementationType::getDefaultDictionary();}

  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionary& dictionary) const
    {const_cast<ImplementationType& >(impl).featureGenerator(visitor, dictionary);}

private:
  ImplementationType impl;
};

template<class ImplementationType>
FeatureGeneratorPtr staticToDynamicFeatureGenerator(const ImplementationType& impl)
  {return FeatureGeneratorPtr(new StaticToDynamicFeatureGenerator<ImplementationType>(impl));}

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_FEATURE_GENERATOR_HPP_
