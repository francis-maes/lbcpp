#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/Data/Stream.h>

#include "../Data/Protein.h"
#include "../Predictor/LargeProteinPredictorParameters.h"
#include "BricoBox.h"

namespace lbcpp
{

class BanditCandidate : public Object
{
public:
  BanditCandidate(const FunctionPtr& function, const StreamPtr& examples, const Variable& parameters)
    : function(function), examples(examples), parameters(parameters) {}

  FunctionPtr getFunction() const
    {return function;}

  StreamPtr getStream() const
    {return examples;}

  Variable getParameters() const
    {return parameters;}

  virtual String toString() const
    {return parameters.toString();}

protected:
  friend class BanditCandidateClass;

  FunctionPtr function;
  StreamPtr examples;
  Variable parameters;

  BanditCandidate() {}
};

extern ClassPtr banditCandidateClass;
typedef ReferenceCountedObjectPtr<BanditCandidate> BanditCandidatePtr;

class BanditFunction : public SimpleUnaryFunction
{
public:
  BanditFunction()
    : SimpleUnaryFunction(banditCandidateClass, probabilityType, T("Bandit")) {}

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    BanditCandidatePtr bandit = input.getObjectAndCast<BanditCandidate>();
    jassert(bandit);
    StreamPtr stream = bandit->getStream();

    if (stream->isExhausted())
      stream->rewind();

    PairPtr obj = stream->next().getObjectAndCast<Pair>();
    Variable result = bandit->getFunction()->compute(context, obj->getFirst(), Variable());

    if (result.inheritsFrom(doubleVectorClass(enumValueType, doubleType)))
    {
       return probability(result.getObjectAndCast<DoubleVector>(context)->getIndexOfMaximumValue()
                              == obj->getSecond().getObjectAndCast<DoubleVector>(context)->getIndexOfMaximumValue() ? 1.f : 0.f);
    }
    if (result.inheritsFrom(probabilityType))
    {
      const bool prediction = result.isBoolean() ? result.getBoolean() : result.getDouble() > 0.5f;
      const bool supervision = obj->getSecond().isBoolean() ? obj->getSecond().getBoolean() : obj->getSecond().getDouble() > 0.5;
      return probability(prediction == supervision ? 1.f : 0.f);
    }
    jassertfalse;
    return Variable();
  }
};

class DoubleVectorContainerSet : public Object
{
public:
  size_t getNumContainers() const
    {return containers.size();}

  size_t getIndex(const String& name) const
  {
    std::map<String, size_t>::const_iterator it = nameToIndex.find(name);
    return (it != nameToIndex.end()) ? it->second : (size_t)-1;
  }

  size_t appendContainer(const String& name, const ContainerPtr& container)
  {
    jassert(containers.size() == 0 || containers.size() != 0 && containers[0]->getNumElements() == container->getNumElements());
    checkInheritance(container->getElementsType(), doubleVectorClass());
    const size_t index = containers.size();
    containers.push_back(container);
    nameToIndex[name] = index;
    return index;
  }

  ContainerPtr getCompositeDoubleVectors(const std::vector<String>& names) const
  {
    const size_t n = names.size();
    std::vector<size_t> indices(n);
    for (size_t i = 0; i < n; ++i)
    {
      indices[i] = getIndex(names[i]);
      jassert(indices[i] != (size_t)-1);
    }
    return getCompositeDoubleVectors(indices);
  }

  ContainerPtr getCompositeDoubleVectors(const std::vector<size_t>& indices) const
  {
    if (containers.size() == 0 || indices.size() == 0)
      return ContainerPtr();

    String enumName;
    for (size_t i = 0; i < indices.size(); ++i)
      enumName += String((int)indices[i]) + T(",");

    ConcatenateEnumerationPtr enumeration = new ConcatenateEnumeration(T("ContainerSet(") + enumName + T(")"));
    for (size_t i = 0; i < indices.size(); ++i)
      enumeration->addSubEnumeration(T("ContainerSet"), containers[indices[i]]->getElementsType()->getTemplateArgument(0));

    const size_t n = containers[0]->getNumElements();
    ObjectVectorPtr res = objectVector(compositeDoubleVectorClass(enumeration, doubleType), n);
    for (size_t i = 0; i < n; ++i)
    {
      CompositeDoubleVectorPtr cdv = new CompositeDoubleVector(compositeDoubleVectorClass(enumeration, doubleType));
      for (size_t j = 0; j < indices.size(); ++j)
        cdv->appendSubVector(containers[indices[j]]->getElement(i).getObjectAndCast<DoubleVector>());
      res->set(i, cdv);
    }
    return res;
  }

protected:
  friend class DoubleVectorContainerSetClass;

  std::map<String, size_t> nameToIndex;
  std::vector<ContainerPtr> containers;
};

typedef ReferenceCountedObjectPtr<DoubleVectorContainerSet> DoubleVectorContainerSetPtr;

class NormalizeDoubleVectorSet : public Object
{
public:
  size_t getNumNormalizers() const
    {return normalizers.size();}

  DoubleVectorPtr normalize(ExecutionContext& context, const DoubleVectorPtr& input, size_t index) const
  {
    jassert(index < normalizers.size());
    return normalizers[index]->compute(context, input).getObjectAndCast<DoubleVector>();
  }

  size_t createNormalizer(ExecutionContext& context, const ContainerPtr& data)
  {
    const TypePtr elementsType = pairClass(data->getClass(), anyType);
    NormalizeDoubleVectorPtr res = new NormalizeDoubleVector(true, false);
    if (!res->train(context, data))
    {
      context.errorCallback(T("NormalizeDoubleVectorSet::createNormalizer"));
      return false;
    }
    normalizers.push_back(res);
    return normalizers.size() - 1;
  }

protected:
  friend class NormalizeDoubleVectorSetClass;

  std::vector<NormalizeDoubleVectorPtr> normalizers;
};

typedef ReferenceCountedObjectPtr<NormalizeDoubleVectorSet> NormalizeDoubleVectorSetPtr;

class ContainerBasedStream : public Stream
{
public:
  ContainerBasedStream(const ContainerPtr& container)
    : container(container), nextIndex(0) {}

  virtual TypePtr getElementsType() const
    {return container->getElementsType();}

  virtual bool rewind()
  {
    nextIndex = 0;
    return true;
  }

  virtual bool isExhausted() const
    {return nextIndex >= container->getNumElements();}

  virtual ProgressionStatePtr getCurrentPosition() const
    {return new ProgressionState(nextIndex, container->getNumElements(), T("elements"));}

  virtual Variable next()
  {
    jassert(nextIndex < container->getNumElements());
    return container->getElement(nextIndex++);
  }

protected:
  friend class ContainerBasedStreamClass;

  ContainerPtr container;
  size_t nextIndex;

  ContainerBasedStream()
    : nextIndex(0) {}
};

class FunctionBasedStream : public Stream
{
public:
  FunctionBasedStream(ExecutionContext& context, const FunctionPtr& function, const std::vector<Variable>& inputs)
    : context(&context), function(function), inputs(inputs), nextIndex(0) {}

  virtual TypePtr getElementsType() const
  {
    if (!function->isInitialized() && inputs.size() != 0)
      function->initialize(*context, inputs[0].getType());
    return function->getOutputType();
  }

  virtual bool rewind()
  {
    nextIndex = 0;
    return true;
  }

  virtual bool isExhausted() const
    {return nextIndex >= inputs.size();}

  virtual ProgressionStatePtr getCurrentPosition() const
    {return new ProgressionState(nextIndex, inputs.size(), T("elements"));}

  virtual Variable next()
  {
    jassert(nextIndex < inputs.size());
    return function->compute(*context, inputs[nextIndex++]);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Stream::clone(context, target);
    ReferenceCountedObjectPtr<FunctionBasedStream> t = target.staticCast<FunctionBasedStream>();
    t->context = this->context;
    t->inputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i)
      t->inputs[i] = inputs[i];
  }

protected:
  friend class FunctionBasedStreamClass;

  ExecutionContext* context;

  FunctionPtr function;
  std::vector<Variable> inputs;

  size_t nextIndex;

  FunctionBasedStream()
    : context(NULL), nextIndex(0) {}
};

class BinaryFunctionBasedStream : public FunctionBasedStream
{
public:
  BinaryFunctionBasedStream(ExecutionContext& context, const FunctionPtr& function, const std::vector<PairPtr>& inputPairs)
    : FunctionBasedStream(context, function, std::vector<Variable>(0))
  {
    const size_t n = inputPairs.size();
    inputs.resize(n);
    for (size_t i = 0; i < n; ++i)
      inputs[i] = Variable(inputPairs[i]);
  }

  virtual TypePtr getElementsType() const
  {
    if (!function->isInitialized() && inputs.size() != 0)
      function->initialize(*context, inputs[0].getType()->getTemplateArgument(0), inputs[0].getType()->getTemplateArgument(1));
    return function->getOutputType();
  }

  virtual Variable next()
  {
    jassert(nextIndex < inputs.size());
    const PairPtr pair = inputs[nextIndex++].getObjectAndCast<Pair>();
    jassert(pair);
    return function->compute(*context, pair->getFirst(), pair->getSecond());
  }

protected:
  friend class BinaryFunctionBasedStreamClass;

  BinaryFunctionBasedStream() {}
};

class PairBinaryFunctionBasedStream : public BinaryFunctionBasedStream
{
public:
  PairBinaryFunctionBasedStream(ExecutionContext& context, const FunctionPtr& function, const std::vector<PairPtr>& inputPairs, const std::vector<Variable>& outputs)
    : BinaryFunctionBasedStream(context, function, inputPairs), outputs(outputs)
  {
    jassert(inputPairs.size() == outputs.size());
    jassert(outputs.size() != 0);
    elementsType = pairClass(BinaryFunctionBasedStream::getElementsType(), outputs[0].getType());
  }

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual Variable next()
  {
    const Variable& output = outputs[nextIndex];
    return new Pair(elementsType, BinaryFunctionBasedStream::next(), output);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    BinaryFunctionBasedStream::clone(context, target);
    ReferenceCountedObjectPtr<PairBinaryFunctionBasedStream> t = target.staticCast<PairBinaryFunctionBasedStream>();
    t->outputs.resize(outputs.size());
    for (size_t i = 0; i < outputs.size(); ++i)
      t->outputs[i] = outputs[i];
  }

protected:
  friend class PairBinaryFunctionBasedStreamClass;

  TypePtr elementsType;
  std::vector<Variable> outputs;

  PairBinaryFunctionBasedStream() {}
};

class StreamBasedStandardDeviationNormalizer : public Function
{
public:
  StreamBasedStandardDeviationNormalizer(const StreamPtr& stream)
  {
    const TypePtr elementsType = stream->getElementsType();
    if (!checkInheritance(elementsType, doubleVectorClass(enumValueType, doubleType)))
      return;
    const size_t n = elementsType->getTemplateArgument(0).dynamicCast<Enumeration>()->getNumElements();
    standardDeviation.resize(n);

    std::vector<ScalarVariableMeanAndVariancePtr> meansAndVariances(n);   
    for (size_t i = 0; i < n; ++i)
      meansAndVariances[i] = new ScalarVariableMeanAndVariance();

    stream->rewind();
    while (!stream->isExhausted())
    {
      DoubleVectorPtr dv = stream->next().dynamicCast<DoubleVector>();
      jassert(dv->getNumElements() == n);
      for (size_t i = 0; i < n; ++i)
      {
        const Variable v = dv->getElement(i);
        meansAndVariances[i]->push(v.exists() ? v.getDouble() : 0.f);
      }
    }

    for (size_t i = 0; i < n; ++i)
    {
      standardDeviation[i] = meansAndVariances[i]->getStandardDeviation();
      if (standardDeviation[i] < 1e-6)
        standardDeviation[i] = 1.f;
    }

    std::cout << "StreamBasedStandardDeviationNormalizer" << std::endl;
    for (size_t i = 0; i < n; ++i)
      std::cout << standardDeviation[i] << " ";
    std::cout << std::endl;
  }

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, doubleType);}

  virtual String getOutputPostFix() const
    {return T("Normalize");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return sparseDoubleVectorClass(inputVariables[0]->getType()->getTemplateArgument(0), inputVariables[0]->getType()->getTemplateArgument(1));}

protected:
  friend class StreamBasedStandardDeviationNormalizerClass;

  std::vector<double> standardDeviation;

  StreamBasedStandardDeviationNormalizer() {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    SparseDoubleVectorPtr sdv = input.getObjectAndCast<DoubleVector>(context)->toSparseVector();
    SparseDoubleVectorPtr res = new SparseDoubleVector(getOutputType());
    const size_t n = sdv->getNumValues();
    std::pair<size_t, double>* value = sdv->getValues();
    for (size_t i = 0; i < n; ++i, ++value)
      res->appendValue(value->first, value->second / standardDeviation[value->first]);
    return res;
  }
};

class ProteinBanditSampler : public Sampler
{
public:
  ProteinBanditSampler(ExecutionContext& context, SamplerPtr decorated, const String& proteinsPath, ProteinTarget target, const LargeProteinPredictorParametersPtr& largePredictor)
    : decorated(decorated)
    , target(target), largePredictor(largePredictor)
  {
    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("train/")), 0, T("Loading training proteins"));
//    ContainerPtr testingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("test/")), 0, T("Loading testing proteins"));

    jassert(typeOfProteinPerception(target) == residueType);    

    FunctionPtr proteinPerceptionFunction = largePredictor->createProteinPerception();

    std::vector<ProteinPrimaryPerceptionPtr> trainingProteinPerceptions;
//    std::vector<ProteinPrimaryPerceptionPtr> testingProteinPerceptions;
    createProteinPerception(context, proteinPerceptionFunction, trainingProteins, trainingProteinPerceptions);
//    createProteinPerception(context, proteinPerceptionFunction, testingProteins, testingProteinPerceptions);

    buildInputPairsAndOutputs(context, trainingProteins, trainingProteinPerceptions, trainingInputPairs, trainingOutputs);
//    buildInputPairsAndOutputs(context, testingProteins, testingProteinPerceptions, testingInputPairs, testingOutputs);

    randomizeExamples(context, trainingInputPairs, trainingOutputs);
//    randomizeExamples(context, testingInputPairs, testingOutputs);
  }

  virtual Variable computeExpectation(const Variable* inputs = NULL) const
  {
    jassert(inputs == NULL);
    return decorated->computeExpectation();
  }

  LargeProteinParametersPtr sampleUniqueCandidate(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs) const
  {
    ScopedLock _(lock);
    enum {maxAttempsToSample = 50};
    for (size_t i = 0; i < maxAttempsToSample; ++i)
    {
      LargeProteinParametersPtr res = decorated->sample(context, random, inputs).getObjectAndCast<LargeProteinParameters>();
      const String key = res->toString();
      if (wasAlreadySampled.count(key) != 1)
      {
        const_cast<ProteinBanditSampler*>(this)->wasAlreadySampled[key] = true;
        return res;
      }
    }
    return LargeProteinParametersPtr();
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    LargeProteinParametersPtr parameters = sampleUniqueCandidate(context, random, inputs);

    if (!parameters)
      return Variable();

    LargeProteinPredictorParametersPtr predictor = largePredictor->cloneAndCast<LargeProteinPredictorParameters>();
    predictor->setParameters(parameters);

    FunctionPtr residuePerceptionFunction = predictor->createResiduePerception();

    StreamPtr trainingStream = new PairBinaryFunctionBasedStream(context, residuePerceptionFunction, trainingInputPairs, trainingOutputs);

    FunctionPtr learner = classificationStreamBasedNearestNeighbor(trainingStream, 5, false);
    learner->initialize(context, trainingStream->getElementsType()->getTemplateArgument(0), trainingStream->getElementsType()->getTemplateArgument(1));

//    StreamPtr testingStream = new PairBinaryFunctionBasedStream(context, residuePerceptionFunction, testingInputPairs, testingOutputs);
    
    return new BanditCandidate(learner, trainingStream, parameters);
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                     const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& validationWeights = DenseDoubleVectorPtr())
  {
    const size_t n = trainingSamples->getNumElements();
    if (n == 0)
      return;

    ObjectVectorPtr parameters = objectVector(largeProteinParametersClass, n);
    for (size_t i = 0; i < n; ++i)
      parameters->setElement(i, trainingSamples->getElement(i).getObjectAndCast<BanditCandidate>(context)->getParameters());
    jassert(!validationSamples); // TODO: if any
    decorated->learn(context, trainingInputs, parameters, trainingWeights, validationInputs, validationSamples, validationWeights);
  }

  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    const size_t n = samples->getNumElements();    
    ObjectVectorPtr parameters = objectVector(largeProteinParametersClass, n);
    for (size_t i = 0; i < n; ++i)
      parameters->setElement(i, samples->getElement(i).getObjectAndCast<BanditCandidate>()->getParameters());

    return decorated->computeProbabilities(inputs, parameters);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    Sampler::clone(context, target);
    ReferenceCountedObjectPtr<ProteinBanditSampler> t = target.staticCast<ProteinBanditSampler>();
    t->decorated = decorated->cloneAndCast<Sampler>(context);

    t->trainingInputPairs = trainingInputPairs;
    t->trainingOutputs = trainingOutputs;
    t->testingInputPairs = testingInputPairs;    
    t->testingOutputs = testingOutputs;
  }

protected:
  friend class ProteinBanditSamplerClass;
  CriticalSection lock;
  SamplerPtr decorated;

  ProteinTarget target;
  LargeProteinPredictorParametersPtr largePredictor;

  ProteinBanditSampler() {}

  void createProteinPerception(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& proteins, std::vector<ProteinPrimaryPerceptionPtr>& proteinPerceptions) const
  {
    const size_t n = proteins->getNumElements();
    proteinPerceptions.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      proteinPerceptions[i] = function->compute(context, proteins->getElement(i).getObject()->getVariable(0)).getObjectAndCast<ProteinPrimaryPerception>();
      jassert(proteinPerceptions[i]);
    }
  }

  void buildInputPairsAndOutputs(ExecutionContext& context, const ContainerPtr& proteins, const std::vector<ProteinPrimaryPerceptionPtr>& proteinPerceptions, std::vector<PairPtr>& inputs, std::vector<Variable>& outputs) const
  {
    const size_t n = proteinPerceptions.size();
    if (n == 0)
      return;
    
    const TypePtr elementsType = pairClass(positiveIntegerType, proteinPerceptions[0]->getClass());
    for (size_t i = 0; i < n; ++i)
    {
      const ProteinPtr protein = proteins->getElement(i).getObject()->getVariable(1).getObjectAndCast<Protein>();
      const ContainerPtr supervisions = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<Container>();

      const size_t length = protein->getLength();
      jassert(length == supervisions->getNumElements());
      for (size_t j = 0; j < length; ++j)
      {
        const Variable output = supervisions->getElement(j);
        if (!output.exists())
          continue;
        inputs.push_back(new Pair(elementsType, j, proteinPerceptions[i]));
        outputs.push_back(output);
      }
    }
  }

  void randomizeExamples(ExecutionContext& context, std::vector<PairPtr>& v1, std::vector<Variable>& v2) const
  {
    const RandomGeneratorPtr rand = context.getRandomGenerator();
    jassert(v1.size() == v2.size());
    for (size_t i = 1; i < v1.size(); ++i)
    {
      const size_t newIndex = rand->sampleSize(i + 1);

      PairPtr tmp1 = v1[newIndex];
      v1[newIndex] = v1[i];
      v1[i] = tmp1;

      Variable tmp2 = v2[newIndex];
      v2[newIndex] = v2[i];
      v2[i] = tmp2;
    }
  }

  inline void swap(size_t& a, size_t& b)
    {size_t tmp = a; a = b; b = tmp;}

private:
  std::vector<PairPtr> trainingInputPairs;
  std::vector<Variable> trainingOutputs;

  std::vector<PairPtr> testingInputPairs;    
  std::vector<Variable> testingOutputs;

  std::map<String, bool> wasAlreadySampled;
};

class ProteinBanditTestFeatures : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsDirectory), 1, T("Loading proteins"));
    if (!proteins || proteins->getNumElements() == 0)
    {
      context.errorCallback(T("No proteins"));
      return false;
    }

    ProteinPtr protein = proteins->getElement(0).dynamicCast<Pair>()->getSecond().dynamicCast<Protein>();
    const size_t length = protein->getLength();
    const size_t increment = length / 10;
    //LargeProteinParametersPtr parameter = new LargeProteinParameters();
    ProteinPrimaryPerceptionPtr perception = (new LargeProteinPredictorParameters())->createProteinPerception()->compute(context, protein).dynamicCast<ProteinPrimaryPerception>();
    jassert(perception);
    for (size_t i = 0; i < largeProteinParametersClass->getNumMemberVariables(); ++i)
    {
      std::cout << largeProteinParametersClass->getMemberVariableName(i) << std::endl;

      TypePtr type = largeProteinParametersClass->getMemberVariableType(i);
      LargeProteinParametersPtr parameter = new LargeProteinParameters();
      parameter->setVariable(i, type == booleanType ? true : Variable(5, positiveIntegerType));
      LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);

      FunctionPtr residueFunction = predictor->createResiduePerception();
      residueFunction->initialize(context, positiveIntegerType, perception->getClass());

      for (size_t j = 0; j < 3; ++j)
      {
        Variable v = residueFunction->compute(context, Variable(j * increment, positiveIntegerType), perception);
        std::cout << v.toString() << std::endl;
      }
    }

    return true;
  }

protected:
  friend class ProteinBanditTestFeaturesClass;

  String proteinsDirectory;

  void printContainer(const ContainerPtr& data) const
  {
    if (!data)
    {
      std::cout << "Nil" << std::endl;
      return;
    }
    const size_t n = data->getNumElements();
    if (n == 0)
    {
      std::cout << "Empty" << std::endl;
      return;
    }
    for (size_t i = 0; i < 2; ++i)
      std::cout << data->getElement(i).toString() << std::endl;
  }
};

class ProteinBanditTestSampler : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters();
    predictor->learningMachineName = T("kNN");
    predictor->knnNeighbors = 5;
    SamplerPtr sampler = new ProteinBanditSampler(context,
                                                 objectCompositeSampler(largeProteinParametersClass, LargeProteinParameters::createSingleTaskSingleStageSamplers()),
                                                 proteinsPath, ss3Target, predictor);
    BanditCandidatePtr candidate = sampler->sample(context, context.getRandomGenerator()).getObjectAndCast<BanditCandidate>();

    ObjectPtr obj = candidate->getParameters().getObject();
    for (size_t i = 0; i < largeProteinParametersClass->getNumMemberVariables(); ++i)
    {
      std::cout << largeProteinParametersClass->getMemberVariableName(i)
                << ": " << obj->getVariable(i).toString() << std::endl;
    }

    FunctionPtr objectiveFunction = new BanditFunction();
    
    for (size_t i = 0; i < 50; ++i)
    {
      Variable v = objectiveFunction->compute(context, candidate);
      std::cout << "Result " << i << ": " << v.toString() << std::endl;
    }
    return true;
  }

protected:
  friend class ProteinBanditTestSamplerClass;

  String proteinsPath;
};

class ProteinBanditWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    LargeProteinPredictorParametersPtr predictor(new LargeProteinPredictorParameters());
    predictor->learningMachineName = T("kNN");
    predictor->knnNeighbors = 5;

    SamplerPtr sampler = new ProteinBanditSampler(context, objectCompositeSampler(largeProteinParametersClass, LargeProteinParameters::createSingleTaskSingleStageSamplers())
                                              , proteinsPath, ss3Target, predictor);
    FunctionPtr f(new BanditFunction());
    OptimizationProblemPtr problem(new OptimizationProblem(f, Variable(), sampler));
    OptimizerPtr optimizer(banditEDAOptimizer(5, 10, 5, 0.5f, 300, 2));
    return optimizer->compute(context, problem);
  }

protected:
  friend class ProteinBanditWorkUnitClass;

  String proteinsPath;
};

class SubSetSampler : public Sampler
{

};

};
