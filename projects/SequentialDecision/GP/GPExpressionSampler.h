/*-----------------------------------------.---------------------------------.
| Filename: GPExpressionSampler.h          | Genetic Programming Expression  |
| Author  : Francis Maes                   |  Sampler                        |
| Started : 23/05/2011 16:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GENETIC_PROGRAMMING_EXPRESSION_SAMPLER_H_
# define LBCPP_GENETIC_PROGRAMMING_EXPRESSION_SAMPLER_H_

# include <lbcpp/Sampler/Sampler.h>
# include "GPExpression.h"

namespace lbcpp
{

extern EnumerationPtr gpExprLabelsEnumeration;
extern ClassPtr gpExpressionSamplingContextClass;

class GPExpressionSamplingContext;
typedef ReferenceCountedObjectPtr<GPExpressionSamplingContext> GPExpressionSamplingContextPtr;

class GPExpressionSamplingContext : public Object
{
public:
  GPExpressionSamplingContext(GPExpressionSamplingContextPtr parent, size_t parentLabel)
    : parent(parent), depth(parent->depth + 1), parentLabel(parentLabel), grandParentLabel(parent->parentLabel){}
  GPExpressionSamplingContext() : parentLabel(gpExprLabelsEnumeration->getNumElements()),
    grandParentLabel(gpExprLabelsEnumeration->getNumElements()), depth(0) {}

private:
  friend class GPExpressionSamplingContextClass;

  GPExpressionSamplingContextPtr parent;
  size_t parentLabel;
  size_t grandParentLabel;
  size_t depth;
};

// GPExpressionSamplingContext -> DoubleVector
class GPExpressionSamplingContextFeatureGenerator : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t context = builder.addInput(gpExpressionSamplingContextClass, T("context"));
    size_t depth = builder.addFunction(getVariableFunction(T("depth")), context);
    size_t parentLabel = builder.addFunction(getVariableFunction(T("parentLabel")), context);
    size_t grandParentLabel = builder.addFunction(getVariableFunction(T("grandParentLabel")), context);

    builder.startSelection();
      builder.addFunction(defaultPositiveDoubleFeatureGenerator(10, 0, 1), depth);
      builder.addFunction(enumerationFeatureGenerator(), parentLabel);
      builder.addFunction(enumerationFeatureGenerator(), grandParentLabel);
    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
  }
};

class GPExpressionSampler : public CompositeSampler
{
public:
  GPExpressionSampler(DiscreteSamplerPtr classSampler, EnumerationPtr variablesEnumeration, size_t depth)
    : classSampler(classSampler), variablesEnumeration(variablesEnumeration), depth(depth)
  {
    featureGenerator = new GPExpressionSamplingContextFeatureGenerator();
    featureGenerator->initialize(defaultExecutionContext(), gpExpressionSamplingContextClass); 
  }

  GPExpressionSampler() {}

  virtual String toShortString() const
    {return classSampler->toShortString();}

  SamplerPtr createSubExprSampler() const
  {
    if (depth < 5)
      return new GPExpressionSampler(classSampler, variablesEnumeration, depth + 1);
    else
      return constantSampler(new VariableGPExpression());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    if (samplers.empty())
    {
      std::vector<SamplerPtr>& samplers = const_cast<GPExpressionSampler* >(this)->samplers;
      // binary
      for (size_t i = 0; i < gpOperatorEnumeration->getNumElements(); ++i)
        samplers.push_back(objectCompositeSampler(binaryGPExpressionClass,
           createSubExprSampler(), constantSampler(Variable(i, gpOperatorEnumeration)), createSubExprSampler()));

      // unary
      for (size_t i = 0; i < gpPreEnumeration->getNumElements(); ++i)
        samplers.push_back(objectCompositeSampler(unaryGPExpressionClass,
           constantSampler(Variable(i, gpPreEnumeration)), createSubExprSampler()));

      // constant
      samplers.push_back(constantSampler(new ConstantGPExpression(0.0)));

      // variables
      for (size_t i = 0; i < 4; ++i)
        samplers.push_back(objectCompositeSampler(variableGPExpressionClass, constantSampler(Variable(i, variablesEnumeration))));

      samplers.push_back(classSampler);
    }

    GPExpressionSamplingContextPtr samplingContext;
    if (inputs)
      samplingContext = inputs[0].getObjectAndCast<GPExpressionSamplingContext>();
    else
      samplingContext = new GPExpressionSamplingContext();

    Variable features = featureGenerator->compute(context, samplingContext);

    size_t index = (size_t)classSampler->sample(context, random, &features).getInteger();
    jassert(index < samplers.size() - 1);

    Variable subSamplingContext = new GPExpressionSamplingContext(samplingContext, index);
    return samplers[index]->sample(context, random, &subSamplingContext);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
    {jassert(false);}
 
  size_t matchExpression(const GPExpressionPtr& expression) const
  {
    TypePtr type = expression->getClass();
    if (type == binaryGPExpressionClass)
    {
      const BinaryGPExpressionPtr& expr = expression.staticCast<BinaryGPExpression>();
      GPOperator op = expr->getOperator();
      return op - gpAddition;
    }
    else if (type == unaryGPExpressionClass)
    {
      const UnaryGPExpressionPtr& expr = expression.staticCast<UnaryGPExpression>();
      GPPre pre = expr->getOperator();
      return 4 + (pre - gpLog);
    }
    else if (type == constantGPExpressionClass)
    {
      //const ConstantGPExpressionPtr& expr = expression.staticCast<ConstantGPExpression>();
      return 7;// + expr->getConstantIndex();
    }
    else if (type == variableGPExpressionClass)
    {
      const VariableGPExpressionPtr& expr = expression.staticCast<VariableGPExpression>();
      return 8 + expr->getIndex();
    }
    else
    {
      jassert(false);
      return false;
    }
  }

  void createClassificationExamplesRecursively(ExecutionContext& context, const GPExpressionSamplingContextPtr& samplingContext, const GPExpressionPtr& expression, const ObjectVectorPtr& features, const VectorPtr& labels)
  {
    size_t index = matchExpression(expression);
    features->append(featureGenerator->compute(context, samplingContext));
    labels->append(Variable(index, gpExprLabelsEnumeration));

    if (expression.dynamicCast<UnaryGPExpression>())
    {
      GPExpressionSamplingContextPtr subSamplingContext = new GPExpressionSamplingContext(samplingContext, index);
      createClassificationExamplesRecursively(context, subSamplingContext, expression.staticCast<UnaryGPExpression>()->getExpression(), features, labels);
      return;
    }
    if (expression.dynamicCast<BinaryGPExpression>())
    {
      GPExpressionSamplingContextPtr subSamplingContext = new GPExpressionSamplingContext(samplingContext, index);
      const BinaryGPExpressionPtr& binaryExpression = expression.staticCast<BinaryGPExpression>();
      createClassificationExamplesRecursively(context, subSamplingContext, binaryExpression->getLeft(), features, labels);
      createClassificationExamplesRecursively(context, subSamplingContext, binaryExpression->getRight(), features, labels);
      return;
    }
  }
  
  void makeClassificationData(ExecutionContext& context, const ContainerPtr& samples, ContainerPtr& inputs, ContainerPtr& outputs)
  {
    size_t n = samples->getNumElements();

    ObjectVectorPtr featureVectors = new ObjectVector(doubleVectorClass(), 0);
    VectorPtr labels = vector(gpExprLabelsEnumeration, 0);
    
    for (size_t i = 0; i < n; ++i)
    {
      GPExpressionPtr expression = samples->getElement(i).getObjectAndCast<GPExpression>();
      createClassificationExamplesRecursively(context, new GPExpressionSamplingContext(), expression, featureVectors, labels);
    }

    inputs = featureVectors;
    outputs = labels;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    ContainerPtr subInputs, subOutputs;
    makeClassificationData(context, trainingSamples, subInputs, subOutputs);
    classSampler->learn(context, subInputs, subOutputs);
  }

protected:
  friend class GPExpressionSamplerClass;

  DiscreteSamplerPtr classSampler;
  FunctionPtr featureGenerator;
  EnumerationPtr variablesEnumeration;
  size_t depth;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_EXPRESSION_SAMPLER_H_
