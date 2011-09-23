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

# ifdef JUCE_WIN32
#  pragma warning(disable:4996)
# endif // JUCE_WIN32

class FastTextParser : public Stream
{
public:
  FastTextParser(ExecutionContext& context, const File& file, TypePtr elementsType)
    : Stream(context), elementsType(elementsType)
  {
    maxLineLength = 1024;
	  line = (char* )malloc(sizeof (char) * maxLineLength);
    lineNumber = 0;
    f = fopen(file.getFullPathName(), "r");
    if (!f)
      context.errorCallback(T("Could not open file ") + file.getFullPathName());
  }

  FastTextParser() {}
  virtual ~FastTextParser()
  {
    if (f)
      fclose(f);
   free(line);
  }

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool rewind()
  {
    if (f)
    {
      ::rewind(f);
      return true;
    }
    else
      return false;
  }

  virtual bool isExhausted() const
    {return f == NULL;}

  virtual ProgressionStatePtr getCurrentPosition() const
    {return new ProgressionState(lineNumber, 0, "Lines");}

  virtual Variable parseLine(char* line) = 0;

  virtual Variable next()
  {
    if (!f)
      return Variable();
    line = readNextLine();
    if (!line)
    {
      fclose(f);
      f = NULL;
      return Variable();
    }
    ++lineNumber;
    return parseLine(line);
  }

protected:
  TypePtr elementsType;
  FILE* f;

  char* line;
  int maxLineLength;
  size_t lineNumber;

  char* readNextLine()
  {
	  if (fgets(line, maxLineLength, f) == NULL)
		  return NULL;

	  while (strrchr(line, '\n') == NULL)
	  {
		  maxLineLength *= 2;
		  line = (char* )realloc(line, maxLineLength);
		  int len = (int)strlen(line);
		  if (fgets(line + len, maxLineLength - len, f) == NULL)
			  break;
	  }
	  return line;
  }
};

class BinaryClassificationLibSVMFastParser : public FastTextParser
{
public:
  BinaryClassificationLibSVMFastParser(ExecutionContext& context, const File& file)
    : FastTextParser(context, file, pairClass(sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration), booleanType)) {}
  BinaryClassificationLibSVMFastParser() {}

  virtual Variable parseLine(char* line)
  {
    char* label = strtok(line, " \t\n");
    if (!label)
    {
      context.errorCallback(T("Empty line"));
      return Variable();
    }

    char firstLetter = label[0];
    if (firstLetter >= 'A' && firstLetter <= 'Z') firstLetter += 'a' - 'A';
    bool supervision = (firstLetter == 'y' || firstLetter == 't' || firstLetter == '+' || firstLetter == '1');

    SparseDoubleVectorPtr features = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);   
		while (true)
		{
			char* idx = strtok(NULL, ":");
			char* val = strtok(NULL, " \t");

			if (val == NULL)
				break;

      int index = (int)strtol(idx, NULL, 10);
      double value = strtod(val, NULL);
      features->appendValue((size_t)index, value);
		}
    return new Pair(elementsType, features, supervision);
  }
};



class LearningRuleFormulaObjective : public SimpleUnaryFunction
{
public:
  LearningRuleFormulaObjective(const File& trainFile = File(), const File& testFile = File(), double validationSize = 0.1, size_t numIterations = 20)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), trainFile(trainFile), testFile(testFile), validationSize(validationSize), numIterations(numIterations) {}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    trainData = loadData(context, trainFile);
    testData = loadData(context, testFile);
    return doubleType;
  }

  ContainerPtr loadData(ExecutionContext& context, const File& file)
  {
    // features = new DefaultEnumeration(trainFile.getFileName() + T(" features"));
    // return binaryClassificationLibSVMDataParser(context, file, features)->load();
    features = positiveIntegerEnumerationEnumeration;
    context.enterScope(T("Loading data from ") + file.getFileName());
    ContainerPtr res = StreamPtr(new BinaryClassificationLibSVMFastParser(context, file))->load();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& expr) const
  {
    GPExpressionPtr expression = expr.getObjectAndCast<GPExpression>();

    ContainerPtr train, test;
    makeRandomSplit(context, trainData, train, test);
    return computeFormulaScore(context, expression, train, test);    
  }

  double computeFormulaScore(ExecutionContext& context, const GPExpressionPtr& expression, const ContainerPtr& train, const ContainerPtr& test, bool verbose = false) const
  {
    DoubleVectorPtr theta = performTraining(context, expression, train, verbose);
    return theta ? evaluate(context, theta, test) : 0.0;
  }

  double testFormula(ExecutionContext& context, const GPExpressionPtr& expression) const
    {return computeFormulaScore(context, expression, trainData, testData, true);}

protected:
  friend class LearningRuleFormulaObjectiveClass;

  File trainFile;
  File testFile;
  double validationSize;
  size_t numIterations;

  EnumerationPtr features;
  ContainerPtr trainData;
  ContainerPtr testData;

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

  DoubleVectorPtr performTraining(ExecutionContext& context, const GPExpressionPtr& expression, const ContainerPtr& train, bool verbose) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

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
        double acc = evaluate(context, theta, testData);
        context.resultCallback(T("test accuracy"), acc);
        context.resultCallback(T("train accuracy"), numCorrect / (double)n);
        //context.resultCallback(T("parameters"), theta->cloneAndCast<DoubleVector>());
        context.resultCallback(T("parameters l2norm"), theta->l2norm());
        context.resultCallback(T("parameters l1norm"), theta->l1norm());
        context.resultCallback(T("parameters l0norm"), theta->l0norm());
        context.leaveScope(acc);
      }

      double lowerBound = 0.5;

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

}; /* namespace lbcpp */

#endif // !LBCPP_GP_LEARNING_RULE_FORMULA_OBJECTIVE_H_
