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

namespace lbcpp
{

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
    LuapeNodeBuilderPtr nodeBuilder = exhaustiveSequentialNodeBuilder(complexity);
    //LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder(budget, complexity);
    return compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
  }

  LuapeLearnerPtr createWeakLearner(ProteinTarget target) const
  {
    LuapeNodeBuilderPtr nodeBuilder = createNodeBuilder(target);

    LuapeLearnerPtr conditionLearner;
    if (miniBatchRelativeSize == 0.0)
      return laminatingWeakLearner(nodeBuilder, relativeBudget);
    else if (miniBatchRelativeSize < 1.0)
      return banditBasedWeakLearner(nodeBuilder, relativeBudget, miniBatchRelativeSize);
    else
      return exactWeakLearner(nodeBuilder);
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
    LuapeInferencePtr learningMachine = new LuapeBinaryClassifier(createUniverse());
    addFunctions(learningMachine, target);
    learningMachine->setLearner(adaBoostLearner(createWeakLearner(target), numIterations, treeDepth), verbose);
    learningMachine->setBatchLearner(filterUnsupervisedExamplesBatchLearner(learningMachine->getBatchLearner(), true));
    return learningMachine;
  }

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine =  new LuapeClassifier(createUniverse());
    addFunctions(learningMachine, target);
    learningMachine->setLearner(discreteAdaBoostMHLearner(createWeakLearner(target), numIterations, treeDepth), verbose);
    
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
