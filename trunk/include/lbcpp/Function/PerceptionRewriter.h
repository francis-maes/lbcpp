/*-----------------------------------------.---------------------------------.
| Filename: PerceptionRewriter.h           | Perception Rewriter             |
| Author  : Francis Maes                   |                                 |
| Started : 15/09/2010 20:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_REWRITER_H_
# define LBCPP_FUNCTION_PERCEPTION_REWRITER_H_

# include "Perception.h"

namespace lbcpp
{

class PerceptionRewriteRule : public Object
{
public:
  virtual bool match(TypePtr type, const std::vector<String>& stack) const = 0;
  PerceptionPtr compute(TypePtr type) const;

  juce_UseDebuggingNewOperator

protected:
  virtual PerceptionPtr computeRule(TypePtr type) const = 0;

  std::map<TypePtr, PerceptionPtr> cache;
};

typedef ReferenceCountedObjectPtr<PerceptionRewriteRule> PerceptionRewriteRulePtr;

extern ClassPtr perceptionRewriteRuleClass();

extern PerceptionRewriteRulePtr typeBasedPerceptionRewriteRule(TypePtr type, PerceptionPtr target);
extern PerceptionRewriteRulePtr typeAndStackBasedPerceptionRewriteRule(TypePtr type, const String& stack, PerceptionPtr target);
extern PerceptionRewriteRulePtr enumValueFeaturesPerceptionRewriteRule();
extern PerceptionRewriteRulePtr biVariableFeaturesPerceptionRewriteRule(PerceptionPtr perception);

class PerceptionRewriter : public NameableObject
{
public:
  PerceptionRewriter();

  void addRule(PerceptionRewriteRulePtr rule);
  void addRule(TypePtr type, PerceptionPtr target);
  void addRule(TypePtr type, const String& stack, PerceptionPtr target);
  void addEnumValueFeaturesRule();
  size_t getNumRules() const;

  PerceptionPtr rewrite(PerceptionPtr perception) const;
  PerceptionPtr rewriteRecursively(PerceptionPtr perception, std::vector<String>& stack) const;
  PerceptionPtr applyRules(TypePtr type, const std::vector<String>& stack) const;

  juce_UseDebuggingNewOperator

private:
  friend class PerceptionRewriterClass;

  ObjectVectorPtr rules;
  typedef std::map<PerceptionPtr, PerceptionPtr> RewritedPerceptionsMap;
  RewritedPerceptionsMap rewritedPerceptions;
};

typedef ReferenceCountedObjectPtr<PerceptionRewriter> PerceptionRewriterPtr;

extern PerceptionPtr perceptionToFeatures(PerceptionPtr perception);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_REWRITER_H_
