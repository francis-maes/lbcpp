/*-----------------------------------------.---------------------------------.
| Filename: LearningRuleFormulaObjective.h | Evaluate Learning Rule Formula  |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 20:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_
# define LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_

# include "../Bandits/DiscreteBanditExperiment.h"

namespace lbcpp
{

extern EnumerationPtr learningRuleFormulaVariablesEnumeration;

class LearningRuleFormulaObjective : public SimpleUnaryFunction
{
public:
  LearningRuleFormulaObjective(const File& dataFile = File(), double validationSize = 0.1, size_t numIterations = 20)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), dataFile(dataFile), validationSize(validationSize), numIterations(numIterations) {}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    features = new DefaultEnumeration(dataFile.getFileName() + T(" features"));
    TextParserPtr parser = binaryClassificationLibSVMDataParser(context, dataFile, features);
    data = parser->load();
    return doubleType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& expr) const
  {
    GPExpressionPtr expression = expr.getObjectAndCast<GPExpression>();

    ContainerPtr train, test;
    makeRandomSplit(context, data, train, test);
    DoubleVectorPtr theta = performTraining(context, expression, train);
    return evaluate(context, theta, test);
  }

protected:
  friend class LearningRuleFormulaObjectiveClass;

  File dataFile;
  double validationSize;
  size_t numIterations;

  DefaultEnumerationPtr features;
  ContainerPtr data;

  void makeRandomSplit(ExecutionContext& context, const ContainerPtr& data, ContainerPtr& train, ContainerPtr& test) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    size_t n = data->getNumElements();

    ObjectVectorPtr trainVector = objectVector(data->getElementsType(), 0);
    trainVector->reserve((size_t)(1.2 * n * (1.0 - validationSize)));
    ObjectVectorPtr testVector = objectVector(data->getElementsType(), 0);
    testVector->reserve((size_t)(1.2 * n * validationSize));
    for (size_t i = 0; i < n; ++i)
    {
      Variable element = data->getElement(i);
      if (random->sampleBool(validationSize))
        testVector->append(element);
      else
        trainVector->append(element);
    }

    train = trainVector;
    test = testVector;
  }

  DoubleVectorPtr performTraining(ExecutionContext& context, const GPExpressionPtr& expression, const ContainerPtr& train) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    size_t n = train->getNumElements();
    size_t epoch = 1;

    DenseDoubleVectorPtr theta = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
    for (size_t i = 0; i < numIterations; ++i)
    {
      std::vector<size_t> order;
      random->sampleOrder(n, order);
      for (size_t j = 0; j < n; ++j)
      {
        PairPtr example = train->getElement(order[j]).getObjectAndCast<Pair>();
        SparseDoubleVectorPtr input = example->getFirst().getObjectAndCast<DoubleVector>()->toSparseVector();
        bool label = example->getSecond().getBoolean();
        double score = input->dotProduct(theta, 0);
        if (!label) // TODO: check that this is correct
          score = -score;

        updateParameters(expression, theta, input, score, epoch);
        ++epoch;
      }
    }
    return theta;
  }

  void updateParameters(const GPExpressionPtr& expression, DenseDoubleVectorPtr parameters, SparseDoubleVectorPtr x, double score, size_t epoch) const
  {
    size_t n = x->getNumValues();
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<size_t, double>& v = x->getValue(i);
      double& param = parameters->getValueReference(v.first);

      std::vector<double> inputs(4);
      inputs[0] = param;
      inputs[1] = v.second;
      inputs[2] = score;
      inputs[3] = epoch;
      param = expression->compute(&inputs[0]);
    }
    //size_t numParams = (size_t)juce::jmax((int)x->getNumElements(), (int)theta->getNumElements());
  }

  double evaluate(ExecutionContext& context, const DenseDoubleVectorPtr& theta, const ContainerPtr& test) const
  {
    size_t n = test->getNumElements();
    size_t numCorrect = 0;
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = test->getElement(i).getObjectAndCast<Pair>();
      SparseDoubleVectorPtr input = example->getFirst().getObjectAndCast<DoubleVector>()->toSparseVector();
      bool label = example->getSecond().getBoolean();
      double score = input->dotProduct(theta, 0);
      if (label == (score > 0))
        ++numCorrect;
    }
    return numCorrect / (double)n;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_
