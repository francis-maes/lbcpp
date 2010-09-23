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
  virtual PerceptionPtr compute(TypePtr type, const std::vector<String>& stack) const = 0;
};

typedef ReferenceCountedObjectPtr<PerceptionRewriteRule> PerceptionRewriteRulePtr;

extern ClassPtr perceptionRewriteRuleClass();

class TypeBasedPerceptionRewriteRule : public PerceptionRewriteRule
{
public:
  TypeBasedPerceptionRewriteRule(TypePtr type, PerceptionPtr target)
    : target(target), type(type) {}
  TypeBasedPerceptionRewriteRule() {}

  virtual PerceptionPtr compute(TypePtr type, const std::vector<String>& stack) const
    {return type->inheritsFrom(this->type) ? target : PerceptionPtr();}

protected:
  friend class TypeBasedPerceptionRewriteRuleClass;
  PerceptionPtr target;
  TypePtr type;
};

class TypeAndStackBasedPerceptionRewriteRule : public TypeBasedPerceptionRewriteRule
{
public:
  TypeAndStackBasedPerceptionRewriteRule(TypePtr type, const String& stack, PerceptionPtr target);
  TypeAndStackBasedPerceptionRewriteRule() {}

  virtual PerceptionPtr compute(TypePtr type, const std::vector<String>& stack) const;

private:
  friend class TypeAndStackBasedPerceptionRewriteRuleClass;
  VectorPtr stack;
};

class EnumValueFeaturesPerceptionRewriteRule : public PerceptionRewriteRule
{
public:
  virtual PerceptionPtr compute(TypePtr type, const std::vector<String>& stack) const
    {return type->inheritsFrom(enumValueType()) ? enumValueFeatures(type) : PerceptionPtr();}
};

class PerceptionRewriter : public Object
{
public:
  PerceptionRewriter();

  void addRule(PerceptionRewriteRulePtr rule);
  void addRule(TypePtr type, PerceptionPtr target);
  void addRule(TypePtr type, const String& stack, PerceptionPtr target);

  void addEnumValueFeaturesRule();

  PerceptionPtr rewrite(PerceptionPtr perception) const;
  PerceptionPtr rewriteRecursively(PerceptionPtr perception, std::vector<String>& stack) const;
  PerceptionPtr applyRules(TypePtr type, const std::vector<String>& stack) const;

private:
  friend class PerceptionRewriterClass;

  ObjectVectorPtr rules;
};

typedef ReferenceCountedObjectPtr<PerceptionRewriter> PerceptionRewriterPtr;

extern PerceptionPtr perceptionToFeatures(PerceptionPtr perception);
extern PerceptionPtr hardDiscretizedNumberFeature(TypePtr Type, size_t numIntervals);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_REWRITER_H_
