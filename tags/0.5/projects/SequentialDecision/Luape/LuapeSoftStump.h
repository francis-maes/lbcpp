/*-----------------------------------------.---------------------------------.
| Filename: LuapeSoftStump.h               | Soft Stumps                     |
| Author  : Francis Maes                   |                                 |
| Started : 21/01/2012 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_SOFT_STUMP_H_
# define LBCPP_LUAPE_SOFT_STUMP_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/LuapeBatchLearner.h>
# include "../../../src/Luape/Function/SpecialLuapeFunctions.h"

namespace lbcpp
{
// y = a + b/(1+exp(-gamma * (x - threshold)))
class SoftStumpLuapeFunction : public LuapeFunction
{
public:
  SoftStumpLuapeFunction(double threshold = 0.0, double gamma = 0.0, double a = 0.0, double b = 1.0)
    : threshold(threshold), gamma(gamma), a(a), b(b) {}

  virtual String toShortString() const
    {return ">= " + String(threshold);}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(doubleType) || type->inheritsFrom(integerType);}
  
  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return probabilityType;}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {return T("softstump(") + inputs[0]->toShortString() + " >= " + String(threshold) + T(", ") + String(gamma) + T(")");}

  double compute(double x) const
    {return x == doubleMissingValue ? x : juce::jlimit(0.0, 1.0, a + (b /(1.0 + exp(- gamma * (x - threshold)))));}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return Variable(compute(inputs[0].toDouble()), probabilityType);}

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    const LuapeSampleVectorPtr& scalars = inputs[0];
    jassert(scalars->size());
    if (scalars->getElementsType() == doubleType)
    {
      DenseDoubleVectorPtr res = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, probabilityType, scalars->size());
      double* dest = res->getValuePointer(0);
      for (LuapeSampleVector::const_iterator it = scalars->begin(); it != scalars->end(); ++it)
        *dest++ = compute(it.getRawDouble());
      return new LuapeSampleVector(scalars->getIndices(), res);
    }
    else
      return LuapeFunction::compute(context, inputs, outputType);
  }

  double getThreshold() const
    {return threshold;}

  double getGamma() const
    {return gamma;}

protected:
  friend class SoftStumpLuapeFunctionClass;

  double threshold;
  double gamma;
  double a;
  double b;
};

typedef ReferenceCountedObjectPtr<SoftStumpLuapeFunction> SoftStumpLuapeFunctionPtr;

class SoftStumpWeakLearner : public DecoratorLearner
{
public:
  SoftStumpWeakLearner(LuapeLearnerPtr stumpLearner, double gamma = 10.0)
    : DecoratorLearner(stumpLearner), gamma(gamma) {}
  SoftStumpWeakLearner() : gamma(0.0) {}

  LuapeFunctionNodePtr makeSoftStump(ExecutionContext& context, const LuapeFunctionNodePtr& stumpNode, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    LuapeNodePtr argument = stumpNode->getSubNode(0);
    double threshold = stumpNode->getFunction().staticCast<StumpLuapeFunction>()->getThreshold();

    SparseDoubleVectorPtr samples = problem->getTrainingCache()->getSortedDoubleValues(context, argument, examples);
    if (!samples || !samples->getNumValues())
      return LuapeFunctionNodePtr();


    bestObjectiveValue = -DBL_MAX;
    double bestNormalizedObjective = -DBL_MAX;
    double bestLogGamma = 0.0;

    LearningObjectivePtr objective = getObjective();

    double minimumValue = samples->getValue(0).second;
    double maximumValue = samples->getValue(samples->getNumValues() - 1).second;

/*    double gamma = this->gamma / (maximumValue - minimumValue);
    LuapeFunctionPtr softStumpFunction = new SoftStumpLuapeFunction(threshold, gamma);
    LuapeFunctionNodePtr softStump = new LuapeFunctionNode(softStumpFunction, argument);
    LuapeSampleVectorPtr predictions = softStump->compute(context, problem->getTrainingCache(), examples);
    bestObjectiveValue = objective->compute(predictions);
    context.resultCallback(T("softStumpObjective"), bestObjectiveValue);
    return softStump;*/

    LuapeFunctionNodePtr res;
    if (verbose)
    {
      context.enterScope(T("Making soft stump"));
      context.informationCallback(T("Ref objective value = ") + String(bestObjectiveValue));
    }
    
    
    for (double logGamma = 0.0; logGamma <= 5.0; logGamma += 0.25)
    {
      //double amplitude = juce::jmax(threshold - minimumValue, maximumValue - threshold);
      double gamma = pow(10.0, logGamma) / (maximumValue - minimumValue);
/*
      double vMin = 1.0/(1.0 + exp(gamma * amplitude));
      double vMax = 1.0/(1.0 + exp(-gamma * amplitude));

      double b = 1.0 / (vMax - vMin);
      double a = -vMin * b;*/

      if (verbose)
      {
        context.enterScope(T("Gamma ") + String(gamma));
        context.resultCallback("log gamma", logGamma);
        context.resultCallback("gamma", gamma);
        //context.resultCallback("a", a);
        //context.resultCallback("b", b);
      }

      LuapeFunctionPtr softStumpFunction = new SoftStumpLuapeFunction(threshold, gamma);//, a, b);
      LuapeFunctionNodePtr softStump = new LuapeFunctionNode(softStumpFunction, argument);

      LuapeSampleVectorPtr predictions = softStump->compute(context, problem->getTrainingCache(), examples);
      double objectiveValue = objective->compute(predictions);
      context.resultCallback(T("objective"), objectiveValue);

      double sumOfWeights = 0.0;
      for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
        sumOfWeights += fabs(it.getRawDouble() - 0.5);
      sumOfWeights /= (double)predictions->size();
      double normalizedObjective = sumOfWeights ? objectiveValue / sumOfWeights : 0.0;
      if (normalizedObjective >= bestNormalizedObjective)
      {
        bestNormalizedObjective = normalizedObjective;
        bestObjectiveValue = objectiveValue;
        bestLogGamma = logGamma;
        res = softStump;
      }
      if (verbose)
      {
        context.resultCallback(T("sumOfWeights"), sumOfWeights);
        context.resultCallback(T("normalizedObjective"), normalizedObjective);
        context.leaveScope();
      }
    }

    if (verbose)
    {
      context.informationCallback(T("New objective value = ") + String(bestObjectiveValue));
      context.leaveScope(res->toShortString());
    }
    if (bestLogGamma == 5.0)
      return stumpNode;
    else
      return res;
  }

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    LuapeNodePtr res = DecoratorLearner::learn(context, node, problem, examples);
    LuapeFunctionNodePtr functionNode = res.dynamicCast<LuapeFunctionNode>();
    if (functionNode && functionNode->getFunction().isInstanceOf<StumpLuapeFunction>())
    {
      return makeSoftStump(context, functionNode, problem, examples);
    }
    return res;
  }

protected:
  friend class SoftStumpWeakLearnerClass;

  double gamma;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SOFT_STUMP_H_
