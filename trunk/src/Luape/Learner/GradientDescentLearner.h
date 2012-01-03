/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearner.h       | Gradient Descent Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 02/01/2011 22:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_GRADIENT_DESCENT_H_
# define LBCPP_LUAPE_LEARNER_GRADIENT_DESCENT_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Function/IterationFunction.h>
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class GradientDescentLearner : public IterativeLearner
{
public:
  GradientDescentLearner(IterationFunctionPtr learningRate, size_t maxIterations)
    : IterativeLearner(maxIterations), learningRate(learningRate) {}
  GradientDescentLearner() {}

  virtual bool initialize(ExecutionContext& context)
  {
    LuapeSequenceNodePtr rootNode = function->getRootNode().staticCast<LuapeSequenceNode>();
    featureFunction = new LuapeCreateSparseVectorNode(rootNode->getNodes());
    parameters = vector(rootNode->getType(), 0);
    parameters->reserve(featureFunction->getNumSubNodes() * 3);
    transformIntoFeatureFunction(featureFunction, parameters, rootNode->getType());
    return true;
  }

  virtual bool finalize(ExecutionContext& context)
  {
    context.enterScope(T("Finalizing"));
    transformIntoOriginalForm(featureFunction, parameters, parameters->getElementsType());
    LuapeSequenceNodePtr rootNode = function->getRootNode().staticCast<LuapeSequenceNode>();
    rootNode->setNodes(featureFunction->getNodes());
    parameters = VectorPtr();
    featureFunction = LuapeCreateSparseVectorNodePtr();
    context.leaveScope();
    return true;
  }

protected:
  friend class GradientDescentLearnerClass;

  IterationFunctionPtr learningRate;

  LuapeCreateSparseVectorNodePtr featureFunction;
  VectorPtr parameters;

  DenseDoubleVectorPtr computeMultiClassActivation(const SparseDoubleVectorPtr& features) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(function.staticCast<LuapeClassifier>()->getDoubleVectorClass());
    for (size_t i = 0; i < features->getNumValues(); ++i)
    {
      const std::pair<size_t, double>& feature = features->getValue(i);
      const DenseDoubleVectorPtr& param = parameters.staticCast<ObjectVector>()->get(feature.first).staticCast<DenseDoubleVector>();
      param->addWeightedTo(res, 0, feature.second);
    }
    return res;
  }

  ObjectVectorPtr computeMultiClassActivations(const ObjectVectorPtr& featuresVector) const
  {
    size_t n = featuresVector->getNumElements();
    ObjectVectorPtr res = new ObjectVector(function.staticCast<LuapeClassifier>()->getDoubleVectorClass(), n);
    for (size_t i = 0; i < n; ++i)
      res->set(i, computeMultiClassActivation(featuresVector->get(i).staticCast<SparseDoubleVector>()));
    return res;
  }

  void transformIntoFeatureFunction(LuapeNodePtr node, VectorPtr parameters, TypePtr parametersType)
  {
    LuapeConstantNodePtr constant = node.dynamicCast<LuapeConstantNode>();
    if (constant && constant->getType() == parametersType)
    {
      const Variable& value = constant->getValue();
      if (value.exists())
      {
        size_t index = parameters->getNumElements();
        parameters->append(constant->getValue());
        constant->setValue(Variable(index, positiveIntegerType));
      }
      else
        constant->setValue(Variable::missingValue(positiveIntegerType));
    }

    LuapeTestNodePtr test = node.dynamicCast<LuapeTestNode>();
    if (test)
      test->setType(positiveIntegerType);

    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      transformIntoFeatureFunction(node->getSubNode(i), parameters, parametersType);
  }

  void transformIntoOriginalForm(LuapeNodePtr node, const VectorPtr& parameters, TypePtr parametersType)
  {
    LuapeConstantNodePtr constant = node.dynamicCast<LuapeConstantNode>();
    if (constant && constant->getType() == positiveIntegerType)
    {
      const Variable& value = constant->getValue();
      if (value.exists())
        constant->setValue(parameters->getElement((size_t)value.getInteger()));
      else
        constant->setValue(Variable::missingValue(parametersType));
    }

    LuapeTestNodePtr test = node.dynamicCast<LuapeTestNode>();
    if (test)
      test->setType(parametersType);

    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      transformIntoOriginalForm(node->getSubNode(i), parameters, parametersType);
  }
};

class ClassifierSGDLearner : public GradientDescentLearner
{
public:
  ClassifierSGDLearner(MultiClassLossFunctionPtr lossFunction, IterationFunctionPtr learningRate, size_t maxIterations)
    : GradientDescentLearner(learningRate, maxIterations), lossFunction(lossFunction) {}
  ClassifierSGDLearner() {}

  virtual bool initialize(ExecutionContext& context)
  {
    if (!GradientDescentLearner::initialize(context))
      return false;
    
    context.enterScope(T("Computing training features"));
    LuapeSampleVectorPtr featureSamples = trainingCache->getSamples(context, featureFunction, trainingCache->getAllIndices());
    trainingFeatures = featureSamples->getVector().staticCast<ObjectVector>();
    jassert(trainingFeatures);
    context.leaveScope();

    if (validationCache)
    {
      context.enterScope(T("Computing validation features"));
      LuapeSampleVectorPtr featureSamples = validationCache->getSamples(context, featureFunction, validationCache->getAllIndices());
      validationFeatures = featureSamples->getVector().staticCast<ObjectVector>();
      jassert(validationFeatures);
      context.leaveScope();
    }
    return true;
  }

  virtual bool doLearningIteration(ExecutionContext& context, double& trainingScore, double& validationScore)
  {
    static const double learningRate = 0.1;

    const LuapeClassifierPtr& classifier = function.staticCast<LuapeClassifier>();
    const LuapeVectorSumNodePtr& sumNode = classifier->getRootNode().staticCast<LuapeVectorSumNode>();
    EnumerationPtr labels = classifier->getLabels();
    size_t numLabels = labels->getNumElements();

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(trainingCache->getNumSamples(), order);

    ScalarVariableStatistics loss;
    context.enterScope(T("Perform ") + String((int)order.size()) + T(" stochastic gradient steps"));
    for (size_t i = 0; i < order.size(); ++i)
    {
      // get example
      const ObjectPtr& example = trainingData[order[i]];
      Variable supervision = example->getVariable(1);
      size_t correctClass;
      if (supervision.isInteger())
        correctClass = (size_t)supervision.getInteger();
      else
        correctClass = (size_t)supervision.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();

      // compute terms and sum
      SparseDoubleVectorPtr exampleFeatures = trainingFeatures->get(order[i]).staticCast<SparseDoubleVector>();
      DenseDoubleVectorPtr activations = computeMultiClassActivation(exampleFeatures);

      // compute loss value and gradient
      double lossValue = 0.0;
      DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(classifier->getDoubleVectorClass());
      lossFunction->computeMultiClassLoss(activations, correctClass, numLabels, &lossValue, &lossGradient, 1.0);
      loss.push(lossValue);

      // update
      for (size_t j = 0; j < exampleFeatures->getNumValues(); ++j)
      {
        size_t featureIndex = exampleFeatures->getValue(j).first;
        double featureValue = exampleFeatures->getValue(j).second;
        const DenseDoubleVectorPtr& param = parameters.staticCast<ObjectVector>()->get(featureIndex).staticCast<DenseDoubleVector>();
        lossGradient->addWeightedTo(param, 0, -learningRate * featureValue );
      }
    }
    context.leaveScope();

    context.enterScope(T("Recache training node"));
    ObjectVectorPtr trainingPredictions = computeMultiClassActivations(trainingFeatures);
    trainingCache->recacheNode(context, sumNode, trainingPredictions);
    context.leaveScope();

    if (validationCache)
    {
      context.enterScope(T("Recache validation node"));
      ObjectVectorPtr validationPredictions = computeMultiClassActivations(validationFeatures);
      validationCache->recacheNode(context, sumNode, validationPredictions);
      context.leaveScope();
    }
    evaluatePredictions(context, trainingScore, validationScore);
    context.resultCallback(T("meanLoss"), loss.getMean());

    //context.informationCallback(sumNode->toShortString());
    return true;
  }

protected:
  friend class ClassifierSGDLearnerClass;

  MultiClassLossFunctionPtr lossFunction;

  ObjectVectorPtr trainingFeatures;
  ObjectVectorPtr validationFeatures;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_GRADIENT_DESCENT_H_
