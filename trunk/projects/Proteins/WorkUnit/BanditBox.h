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

class ProteinBanditSampler : public Sampler
{
public:
  ProteinBanditSampler(ExecutionContext& context, SamplerPtr decorated, const String& proteinsPath, ProteinTarget target, const LargeProteinPredictorParametersPtr& largePredictor)
    : decorated(decorated)
    , target(target), largePredictor(largePredictor)
    , trainingFeaturesSet(new DoubleVectorContainerSet()), testingFeaturesSet(new DoubleVectorContainerSet()), normalizer(new NormalizeDoubleVectorSet())
  {
    trainingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("train/")), 0, T("Loading training proteins"));
    testingProteins = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("test/")), 0, T("Loading testing proteins"));
    trainingSupervisions = computeSupervisions(context, trainingProteins);
    testingSupervisions = computeSupervisions(context, testingProteins);
  }

  virtual Variable computeExpectation(const Variable* inputs = NULL) const
  {
    jassert(inputs == NULL);
    return decorated->computeExpectation();
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    LargeProteinParametersPtr parameters = decorated->sample(context, random, inputs).getObjectAndCast<LargeProteinParameters>();
    jassert(parameters);
    jassert(typeOfProteinPerception(target) == residueType);

    ContainerPtr trainingFeatures = computeFeatures(context, trainingFeaturesSet, parameters, trainingProteins);
    FunctionPtr learner = largePredictor->learningMachine(target);

    if (!learner->train(context, makeContainerOfPair(trainingFeatures, trainingSupervisions), ContainerPtr(), T("Training")))
    {
      context.errorCallback(T("ProteinBanditSampler::sample"), T("Error during training function !"));
      return Variable();
    }

    ContainerPtr testingFeatures = computeFeatures(context, testingFeaturesSet, parameters, testingProteins);

    return new BanditCandidate(learner, new ContainerBasedStream(makeContainerOfPair(testingFeatures, testingSupervisions)), parameters);
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
    t->trainingProteins = trainingProteins;
    t->testingProteins = testingProteins;
    t->trainingSupervisions = trainingSupervisions;
    t->testingSupervisions = testingSupervisions;
    t->trainingFeaturesSet = trainingFeaturesSet;
    t->testingFeaturesSet = testingFeaturesSet;
    t->normalizer = normalizer;
  }

protected:
  friend class ProteinBanditSamplerClass;

  SamplerPtr decorated;

  ProteinTarget target;
  LargeProteinPredictorParametersPtr largePredictor;

  ProteinBanditSampler() {}

  ContainerPtr computeFeatures(ExecutionContext& context, const DoubleVectorContainerSetPtr& featuresSet, const LargeProteinParametersPtr& parameters, const ContainerPtr& proteins) const
  {
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<size_t> indices;
    for (size_t i = 0; i < n; ++i)
    {
      const String featureName = largeProteinParametersClass->getMemberVariableName(i) + T("[") + parameters->getVariable(i).toString() + T("]");
      size_t index = featuresSet->getIndex(featureName);
      if (index != (size_t)-1)
      {
        indices.push_back(index);
        continue;
      }
      ContainerPtr features = largePredictor->computeFeatures(context, i, parameters->getVariable(i), proteins);
      if (!features)
        continue;

      if (featuresSet == trainingFeaturesSet)
      {
        jassert(normalizer->getNumNormalizers() == featuresSet->getNumContainers())
        normalizer->createNormalizer(context, features);
      }
      const size_t numData = features->getNumElements();
      jassert(normalizer->getNumNormalizers() != 0);
      const size_t normalizerIndex = normalizer->getNumNormalizers() - 1;
      ContainerPtr normalizedFeatures = vector(features->getElementsType(), numData);
      for (size_t j = 0; j < numData; ++j)
        normalizedFeatures->setElement(j, normalizer->normalize(context, features->getElement(j).getObjectAndCast<DoubleVector>(), normalizerIndex));

      index = featuresSet->appendContainer(featureName, features);

      indices.push_back(index);
    }

    return featuresSet->getCompositeDoubleVectors(indices);
  }

  ContainerPtr computeSupervisions(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    const size_t n = proteins->getNumElements();
    VectorPtr res = vector(proteinClass->getMemberVariableType(target)->getTemplateArgument(0));
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr supervision = proteins->getElement(i).getObject()->getVariable(1).getObjectAndCast<Protein>()->getTargetOrComputeIfMissing(context, target).getObjectAndCast<Container>();
      const size_t numSupervisions = supervision->getNumElements();
      for (size_t j = 0; j < numSupervisions; ++j)
        res->append(supervision->getElement(j));
    }
    return res;
  }

  ContainerPtr makeContainerOfPair(const ContainerPtr& features, const ContainerPtr& supervisions) const
  {
    jassert(features->getNumElements() == supervisions->getNumElements());
    const size_t n = features->getNumElements();
    const ClassPtr elementsType = pairClass(features->getElementsType(), supervisions->getElementsType());
    VectorPtr res = vector(elementsType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, new Pair(elementsType, features->getElement(i), supervisions->getElement(i)));
    return res;
  }

private:
  ContainerPtr trainingProteins;
  ContainerPtr testingProteins;
  
  ContainerPtr trainingSupervisions;
  ContainerPtr testingSupervisions;

  DoubleVectorContainerSetPtr trainingFeaturesSet;
  DoubleVectorContainerSetPtr testingFeaturesSet;
  NormalizeDoubleVectorSetPtr normalizer;  
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

    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters();
    DoubleVectorContainerSetPtr featuresSet = new DoubleVectorContainerSet();
    NormalizeDoubleVectorSetPtr normalizer = new NormalizeDoubleVectorSet();
    std::vector<size_t> indices;
    for (size_t i = 0; i < largeProteinParametersClass->getNumMemberVariables(); ++i)
    {
      std::cout << largeProteinParametersClass->getMemberVariableName(i) << std::endl;
      jassert(featuresSet->getIndex(largeProteinParametersClass->getMemberVariableName(i)) == (size_t)-1);

      TypePtr type = largeProteinParametersClass->getMemberVariableType(i);
      ContainerPtr features = predictor->computeFeatures(context, i, type == booleanType ? true : Variable(5, positiveIntegerType), proteins);
      jassert(features->getNumElements() == proteins->getElement(0).getObject()->getVariable(0).getObjectAndCast<Protein>()->getLength());
      
      normalizer->createNormalizer(context, features);
      const size_t numData = features->getNumElements();
      const size_t normalizerIndex = normalizer->getNumNormalizers() - 1;
      ContainerPtr normalizedFeatures = vector(features->getElementsType(), numData);
      for (size_t j = 0; j < numData; ++j)
        normalizedFeatures->setElement(j, normalizer->normalize(context, features->getElement(j).getObjectAndCast<DoubleVector>(), normalizerIndex));

      size_t index = featuresSet->appendContainer(largeProteinParametersClass->getMemberVariableName(i), normalizedFeatures);
      indices.push_back(index);
      //printContainer(features);
    }

    for (size_t i = 0; i < indices.size(); ++i)
      jassert(indices[i] == i);

    ContainerPtr cdvContainer = featuresSet->getCompositeDoubleVectors(indices);
    printContainer(cdvContainer);

    std::vector<size_t> otherIndices;
    for (size_t i = 0; i < indices.size(); i += 2)
      otherIndices.push_back(i);

    cdvContainer = featuresSet->getCompositeDoubleVectors(otherIndices);
    printContainer(cdvContainer);

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
    OptimizerPtr optimizer(banditEDAOptimizer(5, 10, 5, 0.5f, 1000, 2));
    return optimizer->compute(context, problem);
  }

protected:
  friend class ProteinBanditWorkUnitClass;

  String proteinsPath;
};

};