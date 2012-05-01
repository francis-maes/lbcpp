/*-----------------------------------------.---------------------------------.
| Filename: LuapeProteinPredictorParameters.h | Luape Predictor Params       |
| Author  : Francis Maes                   |                                 |
| Started : 23/12/2011 12:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_PREDICTOR_PARAMETERS_H_
# define LBCPP_PROTEINS_LUAPE_PREDICTOR_PARAMETERS_H_

# include "ProteinPerception.h"
# include "../Predictor/ProteinPredictorParameters.h"
# include <lbcpp/Luape/LuapeLearner.h>
# include "../../../src/Luape/Function/ObjectLuapeFunctions.h"
# include "../Evaluator/ProteinEvaluator.h"
# include "../Evaluator/KolmogorovPerfectMatchingFunction.h"

# define USE_EXTRA_TREES

namespace lbcpp
{

class DisulfideBondClassifier : public LuapeBinaryClassifier
{
public:
  void setProteinPairs(ExecutionContext& context, const ContainerPtr& proteinPairs, bool isTrainingData)
  {
    std::vector<SymmetricMatrixPtr>& supervisions = (isTrainingData ? trainPatterns : testPatterns);
    size_t n = proteinPairs->getNumElements();
    supervisions.resize(n);
    
    std::vector<ObjectPtr> data;
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr pair = proteinPairs->getElement(i).getObject();
      ProteinPtr inputProtein = pair->getVariable(0).getObjectAndCast<Protein>();
      ProteinPerceptionPtr inputPerception = new ProteinPerception(inputProtein);
      const std::vector<size_t>& cysteinIndices = inputPerception->getCysteinIndices();
      
      ProteinPtr supervisionProtein = pair->getVariable(1).getObjectAndCast<Protein>();
      supervisions[i] = supervisionProtein->getDisulfideBonds(context);
      size_t numCysteins = supervisions[i]->getDimension();
      for (size_t j = 0; j < numCysteins; ++j)
        for (size_t k = j + 1; k < numCysteins; ++k)
        {
          ObjectPtr bondPerception = new ProteinResiduePairPerception(inputPerception,
            inputPerception->getResidue(cysteinIndices[j]), inputPerception->getResidue(cysteinIndices[k]));
          data.push_back(new Pair(bondPerception, supervisions[i]->getElement(j, k).getDouble() > 0.5));
        }
    }
 
    if (isTrainingData)
    {
      supervision = new LuapeInputNode(probabilityType, T("supervision"), inputs.size());
      trainingCache = createSamplesCache(context, data);
    }
    else
      validationCache = createSamplesCache(context, data);
  }

  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& sup) const
  {
    SupervisedEvaluatorPtr evaluator = new DisulfidePatternEvaluator(new KolmogorovPerfectMatchingFunction(0.f), 0.f);
    SupervisedEvaluatorPtr evaluator2 = symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore));
    
    ScoreObjectPtr score = evaluator->createEmptyScoreObject(context, FunctionPtr());
    ScoreObjectPtr score2 = evaluator2->createEmptyScoreObject(context, FunctionPtr());

    const std::vector<SymmetricMatrixPtr>& supervisions = (sup == getTrainingSupervisions() ? trainPatterns : testPatterns);
    double* predictionsPtr = predictions.staticCast<DenseDoubleVector>()->getValuePointer(0);
    size_t index = 0;
    for (size_t i = 0; i < supervisions.size(); ++i)
    {
      SymmetricMatrixPtr supervision = supervisions[i];
      size_t n = supervision->getNumRows();
      DoubleSymmetricMatrixPtr prediction = new DoubleSymmetricMatrix(probabilityType, n, 0.0);
      for (size_t j = 0; j < n; ++j)
        for (size_t k = j + 1; k < n; ++k)
        {
          jassert(index < predictions->getNumElements());
          double activation = predictionsPtr[index++];
          double probability = 1 / (1 + exp(-activation));
          prediction->setValue(j, k, probability);
        }
      evaluator->addPrediction(context, prediction, supervision, score);
      evaluator2->addPrediction(context, prediction, supervision, score2);
    }
    jassert(index == predictions->getNumElements());
    evaluator->finalizeScoreObject(score, FunctionPtr());
    evaluator2->finalizeScoreObject(score2, FunctionPtr());
    return score->getScoreToMinimize() + score2->getScoreToMinimize() / (double)supervisions.size();
  }

private:
  std::vector<SymmetricMatrixPtr> trainPatterns;
  std::vector<SymmetricMatrixPtr> testPatterns;
};

typedef ReferenceCountedObjectPtr<DisulfideBondClassifier> DisulfideBondClassifierPtr;

class LuapeProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  LuapeProteinPredictorParameters(size_t treeDepth, size_t complexity, double relativeBudget, double miniBatchRelativeSize, size_t numIterations, bool verbose = true)
    : treeDepth(treeDepth), complexity(complexity), relativeBudget(relativeBudget), miniBatchRelativeSize(miniBatchRelativeSize), numIterations(numIterations), verbose(verbose) {}
  LuapeProteinPredictorParameters() : treeDepth(1), complexity(5), relativeBudget(5.0), miniBatchRelativeSize(0.0), numIterations(1000), verbose(false) {}

  Variable createProteinPerceptionFunction(ExecutionContext& context, const Variable& input) const
    {return new ProteinPerception(input.getObjectAndCast<Protein>());}
  
  Variable createResidueVectorPerceptionFunction(ExecutionContext& context, const Variable& input) const
    {return input.getObjectAndCast<ProteinPerception>()->getResidues();}

  Variable createCysteinPairPerceptionFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPerceptionPtr& protein = input.getObjectAndCast<ProteinPerception>();
    const std::vector<size_t>& cysteinIndices = protein->getCysteinIndices();
    size_t n = cysteinIndices.size();
    ObjectSymmetricMatrixPtr res = new ObjectSymmetricMatrix(proteinResiduePairPerceptionClass, n, ObjectPtr());
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i; j < n; ++j)
        res->setElement(i, j, new ProteinResiduePairPerception
          (protein, protein->getResidue(cysteinIndices[i]), protein->getResidue(cysteinIndices[j])));
    return res;
  }

  /*
  ** Perceptions
  */
  virtual FunctionPtr createProteinPerception() const
    {return lbcppMemberUnaryFunction(LuapeProteinPredictorParameters, createProteinPerceptionFunction,
                                      proteinClass, proteinPerceptionClass);}

  // ProteinPerception -> DoubleVector
  virtual FunctionPtr createGlobalPerception() const
    {jassert(false); return FunctionPtr();}

  // ProteinPerception -> Vector[Residue Perception]
  virtual FunctionPtr createResidueVectorPerception() const
    {return lbcppMemberUnaryFunction(LuapeProteinPredictorParameters, createResidueVectorPerceptionFunction,
                                      proteinPerceptionClass, objectVectorClass(proteinResiduePerceptionClass));}

  // ProteinPerception -> SymmetricMatrix[Cystein Pair Perception]
  virtual FunctionPtr createDisulfideSymmetricResiduePairVectorPerception() const
    {return lbcppMemberUnaryFunction(LuapeProteinPredictorParameters, createCysteinPairPerceptionFunction,
                                      proteinPerceptionClass, objectSymmetricMatrixClass(proteinResiduePairPerceptionClass));}

  /*
  ** Learning machines
  */
  LuapeNodeBuilderPtr createNodeBuilder(ProteinTarget target) const
  {
    //LuapeNodeBuilderPtr nodeBuilder = policyBasedNodeBuilder(new RandomPolicy(), budget, complexity);
    //LuapeNodeBuilderPtr nodeBuilder = adaptativeSamplingNodeBuilder(budget, complexity);
    //LuapeNodeBuilderPtr nodeBuilder = exhaustiveSequentialNodeBuilder(complexity);
    LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder((size_t)relativeBudget, complexity);
    //return compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
    return nodeBuilder;
  }

  LuapeLearnerPtr createWeakLearner(ProteinTarget target) const
  {
    LuapeNodeBuilderPtr nodeBuilder = createNodeBuilder(target);

# ifndef USE_EXTRA_TREES
/*    if (miniBatchRelativeSize == 0.0)
      return laminatingWeakLearner(nodeBuilder, relativeBudget);
    else if (miniBatchRelativeSize < 1.0)
      return banditBasedWeakLearner(nodeBuilder, relativeBudget, miniBatchRelativeSize);
    else*/
      return exactWeakLearner(nodeBuilder);
# else
    return randomSplitWeakLearner(nodeBuilder);
# endif // !USE_EXTRA_TREES
  }

  // task level
  virtual FunctionPtr disulfideBondPredictor(ProteinTarget target) const
  {
    LuapeInferencePtr classifier = binaryClassifier(target).staticCast<LuapeInference>();
    classifier->addInput(proteinResiduePairPerceptionClass, "bond");
    return mapNSymmetricMatrixFunction(classifier, 1);
  }

  virtual FunctionPtr labelVectorPredictor(ProteinTarget target) const
  {
    LuapeInferencePtr classifier = multiClassClassifier(target).staticCast<LuapeInference>();
    classifier->addInput(proteinResiduePerceptionClass, "r");
    return mapNContainerFunction(classifier);
  }
  
  virtual FunctionPtr probabilityVectorPredictor(ProteinTarget target) const
  {
    LuapeInferencePtr classifier = binaryClassifier(target).staticCast<LuapeInference>();
    classifier->addInput(proteinResiduePerceptionClass, "r");
    return mapNContainerFunction(classifier);
  }

  // atomic level
  virtual FunctionPtr binaryClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine;
    /*if (target == dsbTarget)
      learningMachine = new DisulfideBondClassifier();
    else*/
      learningMachine = new LuapeBinaryClassifier();
    addFunctions(learningMachine, target);
    
# ifndef USE_EXTRA_TREES
    LuapeLearnerPtr weakLearner = createWeakLearner(target);
    weakLearner->setVerbose(verbose);
    learningMachine->setLearner(adaBoostLearner(weakLearner, numIterations, treeDepth), verbose);
# else
    LuapeLearnerPtr weakLearner = createWeakLearner(target);
    weakLearner->setVerbose(verbose);
    LuapeLearnerPtr baseLearner = treeLearner(new InformationGainBinaryLearningObjective(), weakLearner, 1, 0);
    baseLearner->setVerbose(verbose);
    learningMachine->setLearner(ensembleLearner(baseLearner, numIterations), verbose);
# endif // !USE_EXTRA_TREES
    learningMachine->setBatchLearner(filterUnsupervisedExamplesBatchLearner(learningMachine->getBatchLearner(), false));
    return learningMachine;
  }

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine =  new LuapeClassifier(createUniverse());
    addFunctions(learningMachine, target);
# ifndef USE_EXTRA_TREES
    learningMachine->setLearner(discreteAdaBoostMHLearner(createWeakLearner(target), numIterations, treeDepth), verbose);
#else
    LuapeLearnerPtr weakLearner = createWeakLearner(target);
    weakLearner->setVerbose(verbose);
    LuapeLearnerPtr baseLearner = treeLearner(new InformationGainLearningObjective(), weakLearner, 1, 0);
    baseLearner->setVerbose(verbose);
    learningMachine->setLearner(ensembleLearner(baseLearner, numIterations), verbose);  
#endif
    //MultiClassLossFunctionPtr lossFunction = oneAgainstAllMultiClassLossFunction(hingeDiscriminativeLossFunction());
    //LuapeLearnerPtr strongLearner = compositeLearner(generateTestNodesLearner(createNodeBuilder(target)), classifierSGDLearner(lossFunction, constantIterationFunction(0.1), numIterations));
    //learningMachine->setLearner(strongLearner);

    learningMachine->setBatchLearner(filterUnsupervisedExamplesBatchLearner(learningMachine->getBatchLearner(), true));
    return learningMachine;
  }

  virtual FunctionPtr regressor(ProteinTarget target) const
  {
    jassert(false);
    return FunctionPtr();
  }

  // not used
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    jassert(false);
    return FunctionPtr();
  }

protected:
  friend class LuapeProteinPredictorParametersClass;

  size_t treeDepth;
  size_t complexity;
  double relativeBudget;
  double miniBatchRelativeSize;
  size_t numIterations;
  bool verbose;

  struct Universe : public LuapeUniverse
  {
    bool isGetAccessor(const LuapeNodePtr& node, LuapeNodePtr& argument, String& variableName)
    {
      LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
      if (!functionNode)
        return false;
      GetVariableLuapeFunctionPtr function = functionNode->getFunction().dynamicCast<GetVariableLuapeFunction>();
      if (!function)
        return false;
      argument = functionNode->getArgument(0);
      size_t variableIndex = function->getVariableIndex();
      variableName = argument->getType()->getMemberVariableShortName(variableIndex);
      if (variableName.isEmpty())
        variableName = argument->getType()->getMemberVariableName(variableIndex);
      return true;
    }

    bool isGetRelativeAccessor(const LuapeNodePtr& node, LuapeNodePtr& argument, int& delta)
    {
      LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
      if (!functionNode)
        return false;
      ProteinGetRelativeResidueLuapeFunctionPtr function = functionNode->getFunction().dynamicCast<ProteinGetRelativeResidueLuapeFunction>();
      if (!function)
        return false;
      argument = functionNode->getArgument(0);
      delta = function->getDelta();
      return true;
    }

    virtual LuapeNodePtr canonizeNode(const LuapeNodePtr& node)
    {
      // [x].getRelative(a).getRelative(b) => [x].getRelative(a+b)
      if (node->getType() == proteinResiduePerceptionClass)
      {
        LuapeNodePtr parent;
        LuapeNodePtr grandParent;
        int deltaA, deltaB;
        if (isGetRelativeAccessor(node, parent, deltaA))
        {
          if (isGetRelativeAccessor(parent, grandParent, deltaB))
          {
            int delta = deltaA + deltaB;
            return delta ? makeFunctionNode(new ProteinGetRelativeResidueLuapeFunction(delta), grandParent) : grandParent;
          }
        }
      }

      if (node->getType() == proteinResiduePerceptionClass || node->getType() == proteinPerceptionClass)
      {
        LuapeNodePtr parent;
        String parentVariable;
        if (isGetAccessor(node, parent, parentVariable))
        {
          LuapeNodePtr grandParent;
          String grandParentVariable;
          if (isGetAccessor(parent, grandParent, grandParentVariable))
          {
            if (grandParent->getType() == proteinResiduePerceptionClass)
            {
              // [x].prev.next => [x]
              // [x].next.prev => [x]
              if ((parentVariable == T("next") && grandParentVariable == T("prev")) ||
                  (parentVariable == T("prev") && grandParentVariable == T("next")) ||
                  (parentVariable == T("nextT") && grandParentVariable == T("prevT")) ||
                  (parentVariable == T("prevT") && grandParentVariable == T("nextT")))
                return grandParent;

              // [x]....protein => [x].protein
              if (parentVariable == T("protein"))
                return makeFunctionNode(getVariableLuapeFunction(proteinResiduePerceptionClass, T("protein")), grandParent); // [x].*.protein => [x].protein
            }
          }
        }
      }
      return node;
    }
  };

  LuapeUniversePtr createUniverse() const
    {return new Universe();}

  void addFunctions(const LuapeInferencePtr& machine, ProteinTarget target) const
  {
    // boolean operations
    machine->addFunction(andBooleanLuapeFunction());
    machine->addFunction(equalBooleanLuapeFunction());

    // integer operations
    machine->addFunction(addIntegerLuapeFunction());
    machine->addFunction(subIntegerLuapeFunction());
    machine->addFunction(mulIntegerLuapeFunction());
    machine->addFunction(divIntegerLuapeFunction());

    // double operations
    machine->addFunction(addDoubleLuapeFunction());
    machine->addFunction(subDoubleLuapeFunction());
    machine->addFunction(mulDoubleLuapeFunction());
    machine->addFunction(divDoubleLuapeFunction());

    // enumeration operations
    machine->addFunction(equalsConstantEnumLuapeFunction());

    // object operations
    machine->addFunction(getVariableLuapeFunction());
    machine->addFunction(getContainerLengthLuapeFunction());

    // double vectors
    machine->addFunction(getDoubleVectorElementLuapeFunction());
    machine->addFunction(computeDoubleVectorStatisticsLuapeFunction());
    machine->addFunction(getDoubleVectorExtremumsLuapeFunction());

    // protein-specific operations
    machine->addFunction(new ProteinGetRelativeResidueLuapeFunction());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_PREDICTOR_PARAMETERS_H_
