/*-----------------------------------------.---------------------------------.
| Filename: ValueFunctionStatic.hpp        | Static Value Functions Interface|
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_VALUE_FUNCTION_STATIC_H_
# define CRALGO_IMPL_VALUE_FUNCTION_STATIC_H_

# include "../../ValueFunction.h"
# include "../../Variable.h"
# include "../../Choose.h"

namespace cralgo {
namespace impl {

template<class ExactType>
struct ChooseFunction : public Object<ExactType>
{
  void setChoose(ChoosePtr choose)
    {assert(false);}
};

/*
** Values
*/
template<class ExactType>
struct StateValueFunction : public ChooseFunction<ExactType>
{
  double compute() const
    {assert(false); return 0.0;}
};

template<class ExactType, class ChoiceType_>
struct ActionValueFunction : public ChooseFunction<ExactType>
{
  typedef ChoiceType_ ChoiceType;
  
  double compute(const ChoiceType& choice) const
    {assert(false); return 0.0;}
};

/*
** Features
*/
template<class ExactType>
struct StateFeaturesFunction : public ChooseFunction<ExactType>
{
  FeatureGeneratorPtr compute() const
    {assert(false); return FeatureGeneratorPtr();}
};

template<class ExactType, class ChoiceType_>
struct ActionFeaturesFunction : public ChooseFunction<ExactType>
{
  typedef ChoiceType_ ChoiceType;

  FeatureGeneratorPtr compute(const ChoiceType& choice) const
    {assert(false); return FeatureGeneratorPtr();}
};

/*
** String Descriptions
*/
template<class ExactType>
struct StateDescriptionFunction : public ChooseFunction<ExactType>
{
  std::string compute() const
    {assert(false); return "";}
};

template<class ExactType, class ChoiceType_>
struct ActionDescriptionFunction : public ChooseFunction<ExactType>
{
  typedef ChoiceType_ ChoiceType;

  std::string compute(const ChoiceType& choice) const
    {assert(false); return "";}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_VALUE_FUNCTION_STATIC_H_
