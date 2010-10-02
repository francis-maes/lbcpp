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
 
/*
** PerceptionRewriteRule
*/
PerceptionPtr PerceptionRewriteRule::compute(TypePtr type) const
{
  PerceptionPtr& res = const_cast<PerceptionRewriteRule* >(this)->cache[type];
  if (!res)
    res = computeRule(type);
  return res;
}

/*
** PerceptionRewriter
*/
PerceptionRewriter::PerceptionRewriter() : rules(vector(perceptionRewriteRuleClass()))
{
}

PerceptionPtr PerceptionRewriter::applyRules(TypePtr type, const std::vector<String>& stack) const
{
  for (size_t i = 0; i < rules->getNumElements(); ++i)
  {
    PerceptionRewriteRulePtr rule = rules->getAndCast<PerceptionRewriteRule>(i);
    if (rule->match(type, stack))
      return rule->compute(type);
  }
  return PerceptionPtr();
}

PerceptionPtr PerceptionRewriter::rewriteRecursively(PerceptionPtr perception, std::vector<String>& stack) const
{
  RewritedPerceptionsMap::const_iterator it = rewritedPerceptions.find(perception);
  if (it == rewritedPerceptions.end())
  {
    PerceptionPtr res(new RewritedPerception(perception, refCountedPointerFromThis(this), stack));
    const_cast<PerceptionRewriter* >(this)->rewritedPerceptions[perception] = res;
    return res;
  }
  else
    return it->second;
}

PerceptionPtr PerceptionRewriter::rewrite(PerceptionPtr perception) const
  {std::vector<String> stack; return rewriteRecursively(perception, stack);}

void PerceptionRewriter::addRule(PerceptionRewriteRulePtr rule)
  {rules->append(rule);}

void PerceptionRewriter::addRule(TypePtr type, PerceptionPtr target)
  {rules->append(typeBasedPerceptionRewriteRule(type, target));}

void PerceptionRewriter::addRule(TypePtr type, const String& stack, PerceptionPtr target)
  {rules->append(typeAndStackBasedPerceptionRewriteRule(type, stack, target));}

void PerceptionRewriter::addEnumValueFeaturesRule()
  {rules->append(enumValueFeaturesPerceptionRewriteRule());}

size_t PerceptionRewriter::getNumRules() const
  {return rules->getNumElements();}

PerceptionPtr lbcpp::perceptionToFeatures(PerceptionPtr perception)
{
  PerceptionRewriterPtr rewriter = new PerceptionRewriter();
  // TODO delete after test
  rewriter->addRule(biVariableFeaturesPerceptionRewriteRule(hardDiscretizedNumberFeatures(probabilityType(), 10)));

  rewriter->addEnumValueFeaturesRule();
  rewriter->addRule(doubleType(), identityPerception());

  return rewriter->rewrite(perception);
}
