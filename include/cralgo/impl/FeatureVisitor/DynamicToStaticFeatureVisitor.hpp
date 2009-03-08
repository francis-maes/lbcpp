/*-----------------------------------------.---------------------------------.
| Filename: DynamicToStaticFeatureVis...hpp| Dynamic to Static wrapper       |
| Author  : Francis Maes                   |   for feature visitors          |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_FEATURE_VISITOR_DYNAMIC_TO_STATIC_HPP_
# define CRALGO_STATIC_FEATURE_VISITOR_DYNAMIC_TO_STATIC_HPP_

# include "StaticFeatureVisitor.hpp"

namespace cralgo
{

struct DynamicToStaticFeatureVisitor : public StaticFeatureVisitor<DynamicToStaticFeatureVisitor>
{
  DynamicToStaticFeatureVisitor(FeatureVisitor& target)
    : target(target) {}
  
  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
    {return target.featureEnter(dictionary, number);}

  void featureSense(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
    {target.featureSense(dictionary, number, value);}
  
  void featureCall(cralgo::FeatureDictionary& dictionary, cralgo::FeatureGeneratorPtr featureGenerator)
    {target.featureCall(dictionary, featureGenerator);}

  void featureCall(cralgo::FeatureDictionary& dictionary, size_t number, cralgo::FeatureGeneratorPtr featureGenerator)
    {StaticFeatureVisitor<DynamicToStaticFeatureVisitor>::featureCall(dictionary, number, featureGenerator);}
  
  void featureLeave()
    {target.featureLeave();}

  FeatureVisitor& getTarget()
    {return target;}

private:
  FeatureVisitor& target;
};

/*
** TODO: optimize ? 
**   -> soit clone, soit bridge, soit shared_ptr
namespace impl
{
  FeatureVisitor* staticToDynamic(DynamicToStaticFeatureVisitor& impl)
    {return impl.getTarget();}
};
*/

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_FEATURE_VISITOR_DYNAMIC_TO_STATIC_HPP_
