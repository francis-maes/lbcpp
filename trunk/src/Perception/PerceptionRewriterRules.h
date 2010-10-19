/*-----------------------------------------.---------------------------------.
| Filename: PerceptionRewriterRules.h      | Perception Rewriter Rules       |
| Author  : Francis Maes                   |                                 |
| Started : 02/10/2010 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_REWRITER_RULES_H_
# define LBCPP_FUNCTION_PERCEPTION_REWRITER_RULES_H_

# include <lbcpp/Perception/PerceptionRewriter.h>

namespace lbcpp
{

class TypeBasedPerceptionRewriteRule : public PerceptionRewriteRule
{
public:
  TypeBasedPerceptionRewriteRule(TypePtr type, PerceptionPtr target)
    : type(type), target(target) {}
  TypeBasedPerceptionRewriteRule() {}

  virtual bool match(TypePtr type, const std::vector<String>& stack) const
    {return type->inheritsFrom(this->type);}

  virtual PerceptionPtr computeRule(TypePtr type) const
    {return target;}

  juce_UseDebuggingNewOperator

protected:
  friend class TypeBasedPerceptionRewriteRuleClass;
  TypePtr type;
  PerceptionPtr target;
};

class TypeAndStackBasedPerceptionRewriteRule : public PerceptionRewriteRule
{
public:
  TypeAndStackBasedPerceptionRewriteRule(TypePtr type, const String& stack, PerceptionPtr target)
    : type(type), stack(vector(stringType)), target(target)
  {
    StringArray tokens;
    tokens.addTokens(stack, T("."), NULL);
    for (int i = 0; i < tokens.size(); ++i)
      this->stack->append(tokens[i]);
  }

  TypeAndStackBasedPerceptionRewriteRule() {}

  virtual bool match(TypePtr type, const std::vector<String>& stack) const
  {
    if (!type->inheritsFrom(this->type))
      return false;
    size_t n = this->stack->getNumElements();
    if (stack.size() < n)
      return false;
    for (size_t i = 0; i < n; ++i)
      if (this->stack->getElement(i).getString() != stack[i])
        return false;
    return true;
  }

  virtual PerceptionPtr computeRule(TypePtr type) const
    {return target;}

  juce_UseDebuggingNewOperator

private:
  friend class TypeAndStackBasedPerceptionRewriteRuleClass;

  TypePtr type;
  VectorPtr stack;
  PerceptionPtr target;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_REWRITER_RULES_H_
