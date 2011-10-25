/*-----------------------------------------.---------------------------------.
| Filename: LearningRuleFormulaObjective.h | Evaluate Learning Rule Formula  |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 20:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_
# define LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_

# include "FormulaSearchProblem.h"
# include "../Bandits/DiscreteBanditExperiment.h"

namespace lbcpp
{

extern EnumerationPtr learningRuleFormulaVariablesEnumeration;

class LearningRuleFormulaObjective : public SimpleUnaryFunction
{
public:
  LearningRuleFormulaObjective(size_t numIterations = 20)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), numIterations(numIterations) {}
  
  virtual void sampleTrainAndValidationData(ExecutionContext& context, ContainerPtr& train, ContainerPtr& validation) const = 0;

  virtual ContainerPtr getTrainingData(ExecutionContext& context) const = 0;
  virtual ContainerPtr getTestingData(ExecutionContext& context) const = 0;

  double testFormula(ExecutionContext& context, const GPExpressionPtr& expression) const
    {return computeFormulaScore(context, expression, getTrainingData(context), getTestingData(context), true);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& expr) const
  {
    GPExpressionPtr expression = expr.getObjectAndCast<GPExpression>();

    ContainerPtr train, validation;
    sampleTrainAndValidationData(context, train, validation);
    return computeFormulaScore(context, expression, train, validation);
  }

protected:
  friend class LearningRuleFormulaObjectiveClass;

  size_t numIterations;

  double computeFormulaScore(ExecutionContext& context, const GPExpressionPtr& expression, const ContainerPtr& train, const ContainerPtr& test, bool verbose = false) const
  {
    DoubleVectorPtr theta = performTraining(context, expression, train, verbose);
    return theta ? evaluate(context, theta, test) : 0.0;
  }

  DoubleVectorPtr performTraining(ExecutionContext& context, const GPExpressionPtr& expression, const ContainerPtr& train, bool verbose) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    ContainerPtr testData = ContainerPtr(); // this feature is disabled for the moment, getTestingData(context) ;

    size_t n = train->getNumElements();
    size_t epoch = 1;
    double trainAccuracy = 0.0;

    DenseDoubleVectorPtr theta = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
    for (size_t i = 0; i < numIterations; ++i)
    {
      if (verbose)
      {
        context.enterScope(T("Iteration ") + String((int)i));
        context.resultCallback(T("iteration"), i);
      }

      std::vector<size_t> order;
      random->sampleOrder(n, order);
      size_t numCorrect = 0;
      size_t numPositives = 0;
      size_t numNegatives = 0;
      for (size_t j = 0; j < n; ++j)
      {
        PairPtr example = train->getElement(order[j]).getObjectAndCast<Pair>();
        SparseDoubleVectorPtr input = example->getFirst().getObjectAndCast<DoubleVector>()->toSparseVector();
        bool label = example->getSecond().getBoolean();
        double score = input->dotProduct(theta, 0);
        double sign = label ? 1 : -1;
        if (score * sign > 0) ++numCorrect;
        if (sign > 0) ++numPositives; else ++numNegatives;
        updateParameters(expression, theta, input, score, sign, epoch);
        ++epoch;
      }

      trainAccuracy = numCorrect / (double)n;

      if (verbose)
      {
        double acc = numCorrect / (double)n;
        if (testData)
        {
          acc = evaluate(context, theta, testData);
          context.resultCallback(T("test accuracy"), acc);
        }
        context.resultCallback(T("train accuracy"), numCorrect / (double)n);
        //context.resultCallback(T("parameters"), theta->cloneAndCast<DoubleVector>());
        context.resultCallback(T("parameters l2norm"), theta->l2norm());
        context.resultCallback(T("parameters l1norm"), theta->l1norm());
        context.resultCallback(T("parameters l0norm"), theta->l0norm());
        context.leaveScope(acc);
      }

      double lowerBound = 0.01;// juce::jmin((int)numPositives, (int)numNegatives) / (double)n;
      if (trainAccuracy < lowerBound)
      {
        if (verbose)
          context.informationCallback(T("Breaking, too low accuracy: ") + String(trainAccuracy) + T(" < ") + String(lowerBound));
        return DenseDoubleVectorPtr();
      }
    }
    if (verbose)
      context.informationCallback(T("Finished with train acc ") + String(trainAccuracy));
    return theta;
  }

  void updateParameters(const GPExpressionPtr& expression, DenseDoubleVectorPtr parameters, SparseDoubleVectorPtr x, double score, double sign, size_t epoch) const
  {
    size_t n = juce::jmax((int)parameters->getNumValues(), (int)x->getNumValues());
    if (!n)
      return;

    parameters->ensureSize(n);
    double* params = parameters->getValuePointer(0);

    size_t index = 0;
    size_t ns = x->getNumValues();
    for (size_t i = 0; i < n; ++i)
    {
      double& param = params[i];
      double feature = 0.0;
      if (index < ns && x->getValue(index).first == i)
      {
        feature = x->getValue(index).second;
        ++index;
      }
      updateParameter(expression, param, feature, score, sign, epoch);
    }

    /*
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<size_t, double>& v = x->getValue(i);
      double& param = parameters->getValueReference(v.first);
      updateParameter(param, v.second, score, sign, epoch);      
    }*/
  }

  void updateParameter(const GPExpressionPtr& expression, double& param, double feature, double score, double sign, size_t epoch) const
  {
    std::vector<double> inputs(4);
    inputs[0] = param;
    inputs[1] = feature;
    inputs[2] = score * sign;
    inputs[3] = epoch;
    param += sign * expression->compute(&inputs[0]);
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

typedef ReferenceCountedObjectPtr<LearningRuleFormulaObjective> LearningRuleFormulaObjectivePtr;

class FileLearningRuleFormulaObjective : public LearningRuleFormulaObjective
{
public:
  FileLearningRuleFormulaObjective(const File& trainFile = File(), const File& testFile = File(), double validationSize = 0.1, size_t numIterations = 20)
    : LearningRuleFormulaObjective(numIterations), trainFile(trainFile), testFile(testFile), validationSize(validationSize)
  {
    features = new DefaultEnumeration("features");
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    trainData = loadData(context, trainFile);
    testData = loadData(context, testFile);
    return doubleType;
  }
  
  virtual ContainerPtr getTrainingData(ExecutionContext& context) const
    {return trainData;}

  virtual ContainerPtr getTestingData(ExecutionContext& context) const
    {return testData;}

  ContainerPtr loadData(ExecutionContext& context, const File& file)
  {
    context.enterScope(T("Loading data from ") + file.getFileName());
    ContainerPtr res = binaryClassificationLibSVMFastParser(context, file, features)->load();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  virtual void sampleTrainAndValidationData(ExecutionContext& context, ContainerPtr& train, ContainerPtr& validation) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    size_t n = trainData->getNumElements();

    ObjectVectorPtr trainVector = objectVector(trainData->getElementsType(), 0);
    trainVector->reserve((size_t)(1.2 * n * (1.0 - validationSize)));
    ObjectVectorPtr validationVector = objectVector(trainData->getElementsType(), 0);
    validationVector->reserve((size_t)(1.2 * n * validationSize));
    for (size_t i = 0; i < n; ++i)
    {
      Variable element = trainData->getElement(i);
      if (random->sampleBool(validationSize))
        validationVector->append(element);
      else
        trainVector->append(element);
    }

    train = trainVector;
    validation = validationVector;
  }

protected:
  friend class FileLearningRuleFormulaObjectiveClass;

  File trainFile;
  File testFile;
  double validationSize;

  EnumerationPtr features;
  ContainerPtr trainData;
  ContainerPtr testData;
};

class SparseLearningRuleFormulaObjective : public LearningRuleFormulaObjective
{
public:
  SparseLearningRuleFormulaObjective()
    : numDimensions(1000), numTrainExamples(1000), numValidationExamples(100), separatorSparsity(0.0), examplesSparsity(0.99)
  {
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    testTheta = sampleParameters(context);
    return doubleType;
  }

  virtual ContainerPtr getTrainingData(ExecutionContext& context) const
    {return sampleDataset(context, testTheta, numTrainExamples);}

  virtual ContainerPtr getTestingData(ExecutionContext& context) const
    {return sampleDataset(context, testTheta, numValidationExamples);}

  virtual void sampleTrainAndValidationData(ExecutionContext& context, ContainerPtr& train, ContainerPtr& validation) const
  {
    DenseDoubleVectorPtr theta = sampleParameters(context);
    train = sampleDataset(context, theta, numTrainExamples);
    validation = sampleDataset(context, theta, numValidationExamples);
  }

  DenseDoubleVectorPtr sampleParameters(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    DenseDoubleVectorPtr res = new DenseDoubleVector(numDimensions, 0.0);
    for (size_t i = 0; i < numDimensions; ++i)
      if (random->sampleBool(separatorSparsity) == false)
        res->setValue(i, random->sampleDoubleFromGaussian());
    return res;
  }
   
  ContainerPtr sampleDataset(ExecutionContext& context, const DenseDoubleVectorPtr& theta, size_t numExamples) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ClassPtr featuresClass = sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    TypePtr pairType = pairClass(featuresClass, booleanType);

    std::vector<size_t> featureIndices(numDimensions);
    for (size_t i = 0; i < numDimensions; ++i)
      featureIndices[i] = i;

    ObjectVectorPtr res = objectVector(pairType, numExamples);
    size_t numPositives = 0;
    ScalarVariableStatisticsPtr featureSparsityStats = new ScalarVariableStatistics(T("featureSparsity"));
    for (size_t i = 0; i < numExamples; ++i)
    {
      SparseDoubleVectorPtr features = new SparseDoubleVector(featuresClass);
      features->reserveValues((size_t)(numDimensions * juce::jmin(1.0, (1.0 - examplesSparsity * 0.9))));
      for (size_t j = 0; j < numDimensions; ++j)
        if (random->sampleBool(examplesSparsity) == false)
          features->appendValue(j, 1.0);
      featureSparsityStats->push(features->getNumValues());

      bool supervision = features->dotProduct(theta, 0) > 0;
      if (supervision)
        ++numPositives;

      res->set(i, new Pair(pairType, features, supervision));
    }

    //context.informationCallback(T("Num params: ") + String(theta->l0norm()) + T(" / ") + String((int)numDimensions));
    //context.informationCallback(T("Num positive: ") + String((int)numPositives) + T(" / ") + String((int)numExamples));
    //context.informationCallback(T("Examples Sparsity: ") + featureSparsityStats->toShortString());
    return res;
  }

protected:
  friend class SparseLearningRuleFormulaObjectiveClass;

  size_t numDimensions;
  size_t numTrainExamples;
  size_t numValidationExamples;
  double separatorSparsity; // percentage of null values
  double examplesSparsity; // percentage of null values

  DenseDoubleVectorPtr testTheta;
};


class LearningRuleFormulaSearchProblem : public FormulaSearchProblem
{
public:
  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return learningRuleFormulaVariablesEnumeration;}

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpLog; i <= gpAbs; ++i)
      unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpLessThan; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);
    for (size_t i = 0; i < count - 1; ++i)
    {
      std::vector<double> input(4);
      input[0] = random->sampleDoubleFromGaussian(0.0, 10.0); // param
      input[1] = random->sampleDoubleFromGaussian(0.0, 1.0);  // feature
      input[2] = random->sampleDoubleFromGaussian(0.0, 10.0); // score
      input[3] = (size_t)pow(10, random->sampleDouble(0.0, 5.0));
      res[i] = input;
    }

    std::vector<double> zero(4);
    zero[0] = 1.0; zero[1] = 0.0; zero[2] = 0.0; zero[3] = 1;
    res[count - 1] = zero;
  }

  virtual FormulaKeyPtr makeFormulaKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    std::map<size_t, size_t> variableUseCounts;
    expression->getVariableUseCounts(variableUseCounts);
    if (variableUseCounts[1] == 0 || variableUseCounts[2] == 0) // at least feature or score must be used
      return FormulaKeyPtr();
    if (variableUseCounts[3] > 0) // forbid variable "epoch" for the moment
      return FormulaKeyPtr(); 

    FormulaKeyPtr res = new FormulaKey(inputSamples.size() * sizeof (juce::int64));
    for (size_t i = 0; i < inputSamples.size(); ++i)
    {
      double value = expression->compute(&inputSamples[i][0]);
      if (!isNumberValid(value))
        return FormulaKeyPtr();
      res->pushInteger((juce::int64)(value * 100000));
    }
    return res;
  }

protected:
  friend class LearningRuleFormulaSearchProblemClass;

  LearningRuleFormulaObjectivePtr objective;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_
