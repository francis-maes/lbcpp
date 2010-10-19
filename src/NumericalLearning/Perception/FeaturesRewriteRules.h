/*-----------------------------------------.---------------------------------.
| Filename: FeaturesRewriteRules.h         | Features Rewrite Rules          |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 15:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_PERCEPTION_REWRITE_RULES_H_
# define LBCPP_NUMERICAL_LEARNING_PERCEPTION_REWRITE_RULES_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class EnumValueFeaturesPerceptionRewriteRule : public PerceptionRewriteRule
{
public:
  virtual bool match(TypePtr type, const std::vector<String>& stack) const
    {return type->inheritsFrom(enumValueType);}

  virtual PerceptionPtr computeRule(TypePtr type) const
    {return enumValueFeatures(type);}

  juce_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_PERCEPTION_REWRITE_RULES_H_
