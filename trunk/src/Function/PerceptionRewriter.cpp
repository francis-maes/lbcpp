/*-----------------------------------------.---------------------------------.
| Filename: PerceptionRewriter.cpp         | Perception Rewriter             |
| Author  : Francis Maes                   |                                 |
| Started : 15/09/2010 20:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/PerceptionRewriter.h>
#include "Perception/RewritedPerception.h"
using namespace lbcpp;

TypeAndStackBasedPerceptionRewriteRule::TypeAndStackBasedPerceptionRewriteRule(TypePtr type, const String& stack, PerceptionPtr target)
  : stack(vector(stringType()))
{
  StringArray tokens;
  tokens.addTokens(stack, T("."), NULL);
  for (int i = 0; i < tokens.size(); ++i)
    this->stack->append(tokens[i]);
}


PerceptionPtr TypeAndStackBasedPerceptionRewriteRule::compute(TypePtr type, const std::vector<String>& stack) const
{
  if (!type->inheritsFrom(this->type))
    return PerceptionPtr();
  size_t n = this->stack->getNumElements();
  if (stack.size() < n)
    return PerceptionPtr();
  for (size_t i = 0; i < n; ++i)
    if (this->stack->getElement(i).getString() != stack[i])
      return PerceptionPtr();
  return target;
}

/*
** PerceptionRewriter
*/
PerceptionPtr PerceptionRewriter::applyRules(TypePtr type, const std::vector<String>& stack) const
{
  for (size_t i = 0; i < rules->getNumElements(); ++i)
  {
    PerceptionRewriteRulePtr rule = rules->getAndCast<PerceptionRewriteRule>(i);
    PerceptionPtr res = rule->compute(type, stack);
    if (res)
      return res;
  }
  return PerceptionPtr();
}

PerceptionPtr PerceptionRewriter::rewriteRecursively(PerceptionPtr perception, std::vector<String>& stack) const
  {return new RewritedPerception(perception, refCountedPointerFromThis(this), stack);}

PerceptionPtr PerceptionRewriter::rewrite(PerceptionPtr perception) const
  {std::vector<String> stack; return rewriteRecursively(perception, stack);}

void PerceptionRewriter::addRule(PerceptionRewriteRulePtr rule)
  {rules->append(rule);}

void PerceptionRewriter::addRule(TypePtr type, PerceptionPtr target)
  {rules->append(new PerceptionRewriteRule(type, target));}

void PerceptionRewriter::addRule(TypePtr type, const String& stack, PerceptionPtr target)
  {rules->append(new PerceptionRewriteRule(type, stack, target));}

PerceptionPtr lbcpp::perceptionToFeatures(PerceptionPtr perception)
{
  PerceptionRewriterPtr rewriter = new PerceptionRewriter();
  //rewriter->addRule(
  return rewriter->rewrite(perception);
}
