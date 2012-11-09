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
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class GradientDescentLearner : public IterativeLearner
{
public:
  GradientDescentLearner(SplitObjectivePtr objective, IterationFunctionPtr learningRate, size_t maxIterations)
    : IterativeLearner(objective, maxIterations), learningRate(learningRate) {}
  GradientDescentLearner() {}

  virtual bool initialize(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    SequenceExpressionPtr rootNode = node.staticCast<SequenceExpression>();
    featureFunction = new CreateSparseVectorExpression(rootNode->getNodes());
    parameters = vector(rootNode->getType(), 0);
    parameters->reserve(featureFunction->getNumSubNodes() * 3);
    transformIntoFeatureFunction(featureFunction, parameters, rootNode->getType());
    return true;
  }

  virtual bool finalize(ExecutionContext& context, const ExpressionPtr& node, const ExpressionDomainPtr& problem, const IndexSetPtr& examples)
  {
    context.enterScope(T("Finalizing"));
    transformIntoOriginalForm(featureFunction, parameters, parameters->getElementsType());
    SequenceExpressionPtr rootNode = node.staticCast<SequenceExpression>();
    rootNode->setNodes(featureFunction->getNodes());
    parameters = VectorPtr();
    featureFunction = CreateSparseVectorExpressionPtr();
    context.leaveScope();
    return true;
  }

protected:
  friend class GradientDescentLearnerClass;

  IterationFunctionPtr learningRate;

  CreateSparseVectorExpressionPtr featureFunction;
  VectorPtr parameters;

  DenseDoubleVectorPtr computeMultiClassActivation(const SparseDoubleVectorPtr& features) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector((ClassPtr)parameters->getElementsType());
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
    ObjectVectorPtr res = new ObjectVector((ClassPtr)parameters->getElementsType(), n);
    for (size_t i = 0; i < n; ++i)
      res->set(i, computeMultiClassActivation(featuresVector->get(i).staticCast<SparseDoubleVector>()));
    return res;
  }

  void transformIntoFeatureFunction(ExpressionPtr node, VectorPtr parameters, TypePtr parametersType)
  {
    ConstantExpressionPtr constant = node.dynamicCast<ConstantExpression>();
    if (constant && constant->getType() == parametersType)
    {
      const Variable& value = constant->getValue();
      if (value.exists())
      {
        size_t index = parameters->getNumElements();
        parameters->append(constant->getValue());
        constant->setValue(new NewInteger(index)); // positiveIntegerType
      }
      else
        constant->setValue(ObjectPtr()); // positiveIntegerType
    }

    TestExpressionPtr test = node.dynamicCast<TestExpression>();
    if (test)
      test->setType(positiveIntegerType);

    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      transformIntoFeatureFunction(node->getSubNode(i), parameters, parametersType);
  }

  void transformIntoOriginalForm(ExpressionPtr node, const VectorPtr& parameters, TypePtr parametersType)
  {
    ConstantExpressionPtr constant = node.dynamicCast<ConstantExpression>();
    if (constant && constant->getType() == positiveIntegerType)
    {
      const Variable& value = constant->getValue();
      if (value.exists())
        constant->setValue(parameters->getElement((size_t)value.getInteger()).toObject());
      else
        constant->setValue(ObjectPtr()); // parametersType
    }

    TestExpressionPtr test = node.dynamicCast<TestExpression>();
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
    : GradientDescentLearner(discreteAdaBoostMHSplitObjective(), learningRate, maxIterations), lossFunction(lossFunction) {}
  ClassifierSGDLearner() {}

  virtual bool initialize(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    if (!GradientDescentLearner::initialize(context, node, problem, examples))
      return false;
    
    context.enterScope(T("Computing training features"));
    DataVectorPtr featureSamples = problem->getTrainingCache()->getSamples(context, featureFunction, examples);
    trainingFeatures = featureSamples->getVector().staticCast<ObjectVector>();
    jassert(trainingFeatures);
    context.leaveScope();

    if (problem->getValidationCache())
    {
      context.enterScope(T("Computing validation features"));
      DataVectorPtr featureSamples = problem->getValidationCache()->getSamples(context, featureFunction, examples);
      validationFeatures = featureSamples->getVector().staticCast<ObjectVector>();
      jassert(validationFeatures);
      context.leaveScope();
    }
    return true;
  }
  
  virtual bool doLearningIteration(ExecutionContext& context, ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    static const double learningRate = 0.1;

    const VectorSumExpressionPtr& sumNode = node.staticCast<VectorSumExpression>();
    ClassPtr doubleVectorClass = sumNode->getType();
    EnumerationPtr labels = DoubleVector::getElementsEnumeration(doubleVectorClass);
    size_t numLabels = labels->getNumElements();
    VectorPtr supervisions = problem->getTrainingSupervisions();

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleSubset(examples->getIndices(), examples->size(), order); // randomize

    ScalarVariableStatistics loss;
    context.enterScope(T("Perform ") + String((int)order.size()) + T(" stochastic gradient steps"));
    for (size_t i = 0; i < order.size(); ++i)
    {
      // get correct class
      Variable supervision = supervisions->getElement(order[i]);
      size_t correctClass;
      if (!lbcpp::convertSupervisionVariableToEnumValue(supervision, correctClass))
      {
        context.errorCallback(T("Missing supervision"));
        return false;
      }

      // compute terms and sum
      SparseDoubleVectorPtr exampleFeatures = trainingFeatures->get(order[i]).staticCast<SparseDoubleVector>();
      DenseDoubleVectorPtr activations = computeMultiClassActivation(exampleFeatures);

      // compute loss value and gradient
      double lossValue = 0.0;
      DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(doubleVectorClass);
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
    problem->getTrainingCache()->recacheNode(context, sumNode, trainingPredictions);
    context.leaveScope();

    if (problem->getValidationCache())
    {
      context.enterScope(T("Recache validation node"));
      ObjectVectorPtr validationPredictions = computeMultiClassActivations(validationFeatures);
      problem->getValidationCache()->recacheNode(context, sumNode, validationPredictions);
      context.leaveScope();
    }
    evaluatePredictions(context, problem, trainingScore, validationScore);
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
