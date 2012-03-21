
#include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

extern BatchLearnerPtr simplePerceptronBatchLearner();

class SimplePerceptron : public Function
{
public:
  SimplePerceptron(double bias = 0.f)
    : bias(bias)
    {setBatchLearner(simplePerceptronBatchLearner());}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? booleanType : (TypePtr)doubleVectorClass(enumValueType, doubleType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return booleanType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DoubleVectorPtr ddv = inputs[0].getObjectAndCast<DoubleVector>();
    return ddv->dotProduct(weights, 0) + bias > 0;
  }

protected:
  friend class SimplePerceptronClass;
  friend class SimplePerceptronBatchLearner;

  DenseDoubleVectorPtr weights;
  double bias;
};

typedef ReferenceCountedObjectPtr<SimplePerceptron> SimplePerceptronPtr;
extern ClassPtr simplePerceptronClass;

class SimplePerceptronBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return simplePerceptronClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    SimplePerceptronPtr perceptron = function.staticCast<SimplePerceptron>();
    jassert(perceptron);

    if (!checkHasAtLeastOneExemples(trainingData))
    {
      context.errorCallback(T("No training examples"));
      return false;
    }

    EnumerationPtr elementsEnumeration = trainingData[0]->getVariable(0).getObjectAndCast<DoubleVector>()->getElementsEnumeration();
    perceptron->weights = new DenseDoubleVector(elementsEnumeration, doubleType);
    perceptron->bias = 0.f;

    const double baseline = perceptron->evaluate(context, trainingData, binaryClassificationEvaluator())->getScoreToMinimize();

    const size_t numIterations = 10000;
    const double learningRate = 0.001f;
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.progressCallback(new ProgressionState(i + 1, numIterations, T("Iteration")));
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback(T("Iteration"), i);
      double sumLoss = 0.0;
      for (size_t j = 0; j < trainingData.size(); ++j)
      {
        const double trueOutput = trainingData[j]->getVariable(1).getBoolean() ? 1.f : -1.f;
        const double predOutput = trainingData[j]->getVariable(0).getObjectAndCast<DoubleVector>()->dotProduct(perceptron->weights, 0) + perceptron->bias;        
        if (predOutput * trueOutput <= 1)
        {
          trainingData[j]->getVariable(0).getObjectAndCast<DoubleVector>()->addWeightedTo(perceptron->weights, 0, learningRate * trueOutput);
          perceptron->bias += learningRate * trueOutput;
          sumLoss += fabs(predOutput - trueOutput);
        }
        //std::cout << "[" << trueOutput << ", " << predOutput << "] " << perceptron->weights->toString() << std::endl;
      }

      const double res = perceptron->evaluate(context, trainingData, binaryClassificationEvaluator())->getScoreToMinimize();
      context.resultCallback(T("Q2 Error"), res);
      context.resultCallback(T("Loss"), sumLoss);
      context.resultCallback(T("Baseline"), baseline);
      context.resultCallback(T("Weights"), perceptron->weights);
      context.resultCallback(T("Bias"), perceptron->bias);
      context.leaveScope(res);
    }
    return true;
  }
};

class SimplePerceptronWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    DefaultEnumerationPtr features = new DefaultEnumeration(T("a1aEnumeration"));
    ContainerPtr trainingData = generateData(context);//*/binaryClassificationLibSVMDataParser(context, File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/BinaryClassification/a1a.train")), features)->load()->randomize();
    //trainingData = addCartesianProduct(context, trainingData);

    ContainerPtr testingData = generateData(context);//*/binaryClassificationLibSVMDataParser(context, File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/BinaryClassification/a1a.test")), features)->load()->randomize();
    //testingData = addCartesianProduct(context, testingData);

    FunctionPtr perceptron = new SimplePerceptron();
    perceptron = binaryClassificationExtraTree(10000, 100, 1);
    if (!perceptron->train(context, trainingData, ContainerPtr(), T("Training")))
      return false;

    perceptron->evaluate(context, trainingData, binaryClassificationEvaluator(), T("Evaluation - Train Set"));
    return perceptron->evaluate(context, testingData, binaryClassificationEvaluator(), T("Evaluation - Test Set"))->getScoreToMinimize();
  }

protected:
  friend class SimplePerceptronWorkUnitClass;

  ContainerPtr generateData(ExecutionContext& context) const
  {
    context.enterScope(T("Generating data"));
    const size_t numVariables = 2;
    const size_t numData = 1000;
    RandomGeneratorPtr rand = context.getRandomGenerator();

    DefaultEnumerationPtr features = new DefaultEnumeration(T("GeneratedDataEnumeration"));
    for (size_t j = 0; j < numVariables; ++j)
      features->addElement(context, T("e") + String((int)j));

    TypePtr elementsType = pairClass(denseDoubleVectorClass(features, doubleType), booleanType);
    VectorPtr res = objectVector(elementsType, numData);
    for (size_t i = 0; i < numData; ++i)
    {
      DenseDoubleVectorPtr ddv = new DenseDoubleVector(features, doubleType);
      ddv->setValue(0, 1.f);
      for (size_t j = 0; j < numVariables; ++j)
        ddv->setValue(j, rand->sampleBool() ? 1.f : 0.f);
      const bool output = ddv->getValue(0) == ddv->getValue(1);
      res->setElement(i, new Pair(elementsType, ddv, output));
    }
    context.leaveScope(res);
    return res;
  }
/*
  DoubleVectorPtr createCartesianProductFeatures(ExecutionContext& context, const DoubleVectorPtr& baseFeatures) const
  {
    EnumerationPtr baseEnum = baseFeatures->getElementsEnumeration();
    const size_t n = baseEnum->getNumElements();

    DefaultEnumerationPtr cartesianEnum = new DefaultEnumeration(T("CartesianEnumeration"));
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
        cartesianEnum->addElement(context, baseEnum->getElementName(i) + T("^") + baseEnum->getElementName(j));
    ConcatenateEnumerationPtr elementsType = new ConcatenateEnumeration();
    elementsType->addSubEnumeration(T("Base"), baseEnum);
    elementsType->addSubEnumeration(T("Product"), cartesianEnum);

    DenseDoubleVectorPtr productFeatures = new DenseDoubleVector(cartesianEnum, doubleType);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
        productFeatures->setValue(i * n + j, baseFeatures->getElement(i).getDouble() * baseFeatures->getElement(j).getDouble());

    CompositeDoubleVectorPtr res = new CompositeDoubleVector(compositeDoubleVectorClass(elementsType, doubleType));
    res->appendSubVector(baseFeatures);
    res->appendSubVector(productFeatures);

    return res;
  }
*/
  ContainerPtr addCartesianProduct(ExecutionContext& context, const ContainerPtr& data) const
  {
    if (!data || data->getNumElements() == 0)
      return ContainerPtr();

    TypePtr baseType = data->getElement(0).getObject()->getVariable(0).getType();
    TypePtr outputType = data->getElement(0).getObject()->getVariable(1).getType();
    FunctionPtr f = cartesianProductFeatureGenerator(false);
    if (!f->initialize(context, baseType, baseType))
      return ContainerPtr();

    ConcatenateEnumerationPtr concatenateType = new ConcatenateEnumeration(T("addCartesianEnumeration"));
    concatenateType->addSubEnumeration(T("Base"), baseType->getTemplateArgument(0));
    concatenateType->addSubEnumeration(T("Product"), f->getOutputType()->getTemplateArgument(0));

    TypePtr compositeType = compositeDoubleVectorClass(concatenateType, doubleType);
    TypePtr pairType = pairClass(compositeType, outputType);

    const size_t n = data->getNumElements();
    ObjectVectorPtr res = objectVector(pairType, n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable input = data->getElement(i).getObject()->getVariable(0);
      Variable output = data->getElement(i).getObject()->getVariable(1);

      CompositeDoubleVectorPtr composite = new CompositeDoubleVector(compositeType);
      composite->appendSubVector(input.getObjectAndCast<DoubleVector>());
      composite->appendSubVector(f->compute(context, input, input).getObjectAndCast<DoubleVector>());

      res->setElement(i, new Pair(pairType, composite, output));
    }
    return res;
  }
};

};

