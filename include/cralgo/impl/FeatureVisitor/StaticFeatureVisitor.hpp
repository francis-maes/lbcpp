/*-----------------------------------------.---------------------------------.
| Filename: StaticFeatureVisitor.hpp       | Static Feature Visitor          |
| Author  : Francis Maes                   |   Base Class                    |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_FEATURE_VISITOR_HPP_
# define CRALGO_STATIC_FEATURE_VISITOR_HPP_

# include "../../FeatureGenerator.h"
# include "../../DoubleVector.h"
# include "../Bridge/StaticInterface.hpp"

namespace cralgo
{

template<class StaticFeatureVisitor>
class StaticToDynamicFeatureVisitor : public FeatureVisitor
{
public:
  StaticToDynamicFeatureVisitor(StaticFeatureVisitor& staticVisitor)
    : staticVisitor(staticVisitor) {}
    
  StaticFeatureVisitor& staticVisitor;
  
  virtual bool featureEnter(FeatureDictionary& dictionary, size_t index)
    {return staticVisitor.featureEnter(dictionary, index);}
    
  virtual void featureSense(FeatureDictionary& dictionary, size_t index, double value)
    {staticVisitor.featureSense(dictionary, index, value);}

  virtual void featureCall(FeatureDictionary& dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
    {staticVisitor.featureCall(dictionary, scopeIndex, featureGenerator);}

  virtual void featureCall(FeatureDictionary& dictionary, FeatureGeneratorPtr featureGenerator)
    {staticVisitor.featureCall(dictionary, featureGenerator);}

  virtual void featureLeave()
    {staticVisitor.featureLeave();}
};
/*
namespace impl
{
  template<class StaticFeatureVisitor>
  FeatureVisitor* instantiateFeatureVisitor(StaticFeatureVisitor& staticVisitor)
    {return new StaticToDynamicFeatureVisitor<StaticFeatureVisitor>(staticVisitor);}
};
*/
template<class ExactType>
inline void StaticFeatureVisitor<ExactType>::featureCall(cralgo::FeatureDictionary& dictionary, cralgo::FeatureGeneratorPtr featureGenerator)
{
  StaticToDynamicFeatureVisitor<ExactType> dynamicVisitor(_this());
  featureGenerator->accept(dynamicVisitor, &dictionary);
}

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_FEATURE_VISITOR_HPP_
