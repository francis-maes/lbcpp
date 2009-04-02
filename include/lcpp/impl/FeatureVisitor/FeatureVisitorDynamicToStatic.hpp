/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorDynamicToSta..hpp| Dynamic to Static wrapper       |
| Author  : Francis Maes                   |   for feature visitors          |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_FEATURE_VISITOR_DYNAMIC_TO_STATIC_H_
# define LCPP_FEATURE_VISITOR_DYNAMIC_TO_STATIC_H_

# include "FeatureVisitorStatic.hpp"

namespace lcpp {
namespace impl {

struct DynamicToStaticFeatureVisitor : public FeatureVisitor<DynamicToStaticFeatureVisitor>
{
  DynamicToStaticFeatureVisitor(FeatureVisitorPtr target)
    : target(target) {}
  
  bool featureEnter(lcpp::FeatureDictionaryPtr dictionary, size_t number)
    {return target->featureEnter(dictionary, number);}

  void featureSense(lcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {target->featureSense(dictionary, number, value);}
  
  void featureCall(lcpp::FeatureDictionaryPtr dictionary, lcpp::FeatureGeneratorPtr featureGenerator)
    {target->featureCall(dictionary, featureGenerator);}

  void featureCall(lcpp::FeatureDictionaryPtr dictionary, size_t number, lcpp::FeatureGeneratorPtr featureGenerator)
    {FeatureVisitor<DynamicToStaticFeatureVisitor>::featureCall(dictionary, number, featureGenerator);}
  
  void featureLeave()
    {target->featureLeave();}

  FeatureVisitorPtr getTarget()
    {return target;}

private:
  FeatureVisitorPtr target;
};

inline DynamicToStaticFeatureVisitor dynamicToStatic(FeatureVisitorPtr visitor)
  {return DynamicToStaticFeatureVisitor(visitor);}

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_STATIC_FEATURE_VISITOR_DYNAMIC_TO_STATIC_HPP_
