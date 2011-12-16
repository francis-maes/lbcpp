/*-----------------------------------------.---------------------------------.
| Filename: ProteinLuapeSandBox.h          | Protein Luape SandBox           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
# define LBCPP_PROTEINS_LUAPE_SAND_BOX_H_

#include <lbcpp/Core/Function.h>
#include <lbcpp/Luape/LuapeInference.h>
#include "../Predictor/DecoratorProteinPredictorParameters.h"
#include "../Predictor/LargeProteinPredictorParameters.h"
#include "../Evaluator/ProteinEvaluator.h"

namespace lbcpp
{

//////////////////////////////////////////////
////////////// Protein Perception ////////////
//////////////////////////////////////////////

class ProteinPerception;
typedef ReferenceCountedObjectPtr<ProteinPerception> ProteinPerceptionPtr;
class ProteinResiduePerception;
typedef ReferenceCountedObjectPtr<ProteinResiduePerception> ProteinResiduePerceptionPtr;
class ProteinResiduePairPerception;
typedef ReferenceCountedObjectPtr<ProteinResiduePairPerception> ProteinResiduePairPerceptionPtr;

class ProteinResiduePerception : public Object
{
public:
  ProteinResiduePerception(const ProteinPerceptionPtr& proteinPerception, const ProteinPtr& protein, size_t index)
    : protein(proteinPerception), position(index), positionOfType((size_t)-1)
  {
    aminoAcidType = (AminoAcidType)protein->getPrimaryStructure()->getElement(index).getInteger();
    pssmRow = getDenseDoubleVector(protein->getPositionSpecificScoringMatrix(), index);
    secondaryStructure = getDenseDoubleVector(protein->getSecondaryStructure(), index);
    dsspSecondaryStructure = getDenseDoubleVector(protein->getDSSPSecondaryStructure(), index);
    structuralAlphabet = getDenseDoubleVector(protein->getStructuralAlphabetSequence(), index);
    solventAccessibilityAt20p = getProbability(protein->getSolventAccessibilityAt20p(), index);
    disordered = getProbability(protein->getDisorderRegions(), index);
  }
  ProteinResiduePerception() : position((size_t)-1), positionOfType((size_t)-1), aminoAcidType(totalNumAminoAcids), solventAccessibilityAt20p(doubleMissingValue), disordered(doubleMissingValue) {}

  void setPrevious(const ProteinResiduePerceptionPtr& previous)
  {
    this->previous = previous;
    previous->next = this;
  }

  void setPreviousOfType(const ProteinResiduePerceptionPtr& previous)
  {
    this->previousOfType = previous;
    previous->nextOfType = this;
  }
  
  void setPositionOfType(size_t position)
    {positionOfType = position;}

  const ProteinPerceptionPtr& getProtein() const
    {return protein;}

  size_t getPosition() const
    {return position;}

  AminoAcidType getAminoAcidType() const
    {return aminoAcidType;}

protected:
  friend class ProteinResiduePerceptionClass;

  ProteinPerceptionPtr protein;
  size_t position;

  ProteinResiduePerceptionPtr previous;
  ProteinResiduePerceptionPtr next;

  ProteinResiduePerceptionPtr previousOfType;
  ProteinResiduePerceptionPtr nextOfType;
  size_t positionOfType;

  AminoAcidType aminoAcidType;
  DenseDoubleVectorPtr pssmRow;
  DenseDoubleVectorPtr secondaryStructure;
  DenseDoubleVectorPtr dsspSecondaryStructure;
  DenseDoubleVectorPtr structuralAlphabet;
  double solventAccessibilityAt20p;
  double disordered;

  static DenseDoubleVectorPtr getDenseDoubleVector(const ContainerPtr& container, size_t index)
  {
    if (!container)
      return DenseDoubleVectorPtr();
    jassert(index < container->getNumElements());
    DoubleVectorPtr res = container->getElement(index).getObjectAndCast<DoubleVector>();
    if (!res)
      return DenseDoubleVectorPtr();
    return res->toDenseDoubleVector();
  }

  static double getProbability(const ContainerPtr& container, size_t index)
  {
    if (!container)
      return doubleMissingValue;
    jassert(index < container->getNumElements());
    return container.staticCast<DenseDoubleVector>()->getValue(index);
  }
};

extern ClassPtr proteinResiduePerceptionClass;

class ProteinResiduePairPerception : public Object
{
public:
  ProteinResiduePairPerception(const ProteinPerceptionPtr& protein, const ProteinResiduePerceptionPtr& firstResidue, const ProteinResiduePerceptionPtr& secondResidue)
    : protein(protein), firstResidue(firstResidue), secondResidue(secondResidue) {}
  ProteinResiduePairPerception() {}

protected:
  friend class ProteinResiduePairPerceptionClass;

  ProteinPerceptionPtr protein;
  ProteinResiduePerceptionPtr firstResidue;
  ProteinResiduePerceptionPtr secondResidue;
};

extern ClassPtr proteinResiduePairPerceptionClass;

class ProteinPerception : public Object
{
public:
  ProteinPerception(const ProteinPtr& protein)
  {
    // create residues
    size_t n = protein->getLength();
    residues = new ObjectVector(proteinResiduePerceptionClass, n);

    std::vector<ProteinResiduePerceptionPtr> lastResiduesByType(totalNumAminoAcids);
    std::vector<size_t> countByType(totalNumAminoAcids, 0);
    for (size_t i = 0; i < n; ++i)
    {
      ProteinResiduePerceptionPtr residue = new ProteinResiduePerception(this, protein, i);
      residues->set(i, residue);
      if (i > 0)
        residue->setPrevious(residues->getAndCast<ProteinResiduePerception>(i - 1));
      
      ProteinResiduePerceptionPtr& last = lastResiduesByType[residue->getAminoAcidType()];
      if (last)
        residue->setPreviousOfType(last);
      last = residue;

      size_t& positionByType = countByType[residue->getAminoAcidType()];
      positionByType++;
      residue->setPositionOfType(positionByType);
    }

    // cysteins
    cysteinIndices = protein->getCysteinIndices();
  }
  ProteinPerception() {}

  size_t getNumResidues() const
    {return residues->getNumElements();}

  const ObjectVectorPtr& getResidues() const
    {return residues;}

  const ProteinResiduePerceptionPtr& getResidue(size_t index) const
    {return residues->getAndCast<ProteinResiduePerception>(index);}

  const std::vector<size_t>& getCysteinIndices() const
    {return cysteinIndices;}

protected:
  friend class ProteinPerceptionClass;

  ObjectVectorPtr residues;
  std::vector<size_t> cysteinIndices;
};

extern ClassPtr proteinPerceptionClass;

class ProteinGetRelativeResidueLuapeFunction : public LuapeFunction
{
public:
  ProteinGetRelativeResidueLuapeFunction(int delta = 0)
    : delta(delta) {}

  virtual String toShortString() const
    {return "[" + String(delta) + "]";}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type == proteinResiduePerceptionClass;}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& ) const
    {return proteinResiduePerceptionClass;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(".neighbor[") + String(delta) + T("]");}
  
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinResiduePerceptionPtr residue = inputs[0].getObjectAndCast<ProteinResiduePerception>();
    if (!residue)
      return Variable::missingValue(proteinResiduePerceptionClass);
    ProteinPerceptionPtr protein = residue->getProtein();
    int position = (int)residue->getPosition() + delta;
    if (position < 0 || position >= (int)protein->getNumResidues())
      return Variable::missingValue(proteinResiduePerceptionClass);
    return protein->getResidue((size_t)position);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    enum {windowHalfSize = 4};

    VectorPtr res = vector(integerType, windowHalfSize * 2 + 1);
    for (int i = -windowHalfSize; i <= windowHalfSize; ++i)
      res->setElement((size_t)(i + windowHalfSize), i);
    return res;
  }

protected:
  friend class ProteinGetRelativeResidueLuapeFunctionClass;

  int delta; 
};

//////////////////////////////////////////////
////////// Protein Predictor Parameters //////
//////////////////////////////////////////////

class LuapeProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  LuapeProteinPredictorParameters(size_t treeDepth, size_t numSteps, size_t budget, size_t numIterations)
    : treeDepth(treeDepth), numSteps(numSteps), budget(budget), numIterations(numIterations) {}

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
  BoostingWeakLearnerPtr createWeakLearner(ProteinTarget target) const
  {
    BoostingWeakLearnerPtr conditionLearner = policyBasedWeakLearner(treeBasedRandomPolicy(), budget, numSteps);
    conditionLearner = compositeWeakLearner(constantWeakLearner(), conditionLearner);

    BoostingWeakLearnerPtr res = conditionLearner;
    for (size_t i = 1; i < treeDepth; ++i)
      res = binaryTreeWeakLearner(conditionLearner, res);
    return res;
  }

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
    machine->addFunction(getDoubleVectorElementLuapeFunction());

    // protein-specific operations
    machine->addFunction(new ProteinGetRelativeResidueLuapeFunction());
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

  // atomic level
  virtual FunctionPtr binaryClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine = new LuapeBinaryClassifier();
    addFunctions(learningMachine, target);
    learningMachine->setLearner(adaBoostLearner(createWeakLearner(target)), numIterations);
    return learningMachine;
  }

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine =  new LuapeClassifier();
    addFunctions(learningMachine, target);
    learningMachine->setLearner(adaBoostMHLearner(createWeakLearner(target), true), numIterations);
    learningMachine->setBatchLearner(filterUnsupervisedExamplesBatchLearner(learningMachine->getBatchLearner()));
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
  size_t treeDepth;
  size_t numSteps;
  size_t budget;
  size_t numIterations;
};

//////////////////////////////////////////////
///////////////  SAND BOX      ///////////////
//////////////////////////////////////////////

class ProteinLuapeSandBox : public WorkUnit
{
public:
  ProteinLuapeSandBox() : maxProteinCount(0), treeDepth(2), numSteps(6), budget(100), numIterations(100) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr trainingProteins = loadProteinPairs(context, trainingInputDirectory, trainingSupervisionDirectory, "training");
    ContainerPtr testingProteins = loadProteinPairs(context, testingInputDirectory, testingSupervisionDirectory, "testing");
    if (!trainingProteins || !testingProteins)
      return false;

#if 0
    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(20);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    /*predictor->learningMachineName = T("ExtraTrees");
    predictor->x3Trees = 100;
    predictor->x3Attributes = 0;
    predictor->x3Splits = 1;*/
    predictor->learningMachineName = "kNN";
    predictor->knnNeighbors = 5;
#endif
     
    ProteinPredictorParametersPtr predictor = new LuapeProteinPredictorParameters(treeDepth, numSteps, budget, numIterations);

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    //iteration->addTarget(dsbTarget);
    iteration->addTarget(ss3Target);

    if (!iteration->train(context, trainingProteins, testingProteins, T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = createEvaluator(true);    
    iteration->evaluate(context, trainingProteins, evaluator, T("Evaluate on training proteins"));

    evaluator = createEvaluator(true);
    CompositeScoreObjectPtr scores = iteration->evaluate(context, testingProteins, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class ProteinLuapeSandBoxClass;

  File trainingInputDirectory;
  File trainingSupervisionDirectory;
  File testingInputDirectory;
  File testingSupervisionDirectory;
  size_t maxProteinCount;

  size_t treeDepth;
  size_t numSteps;
  size_t budget;
  size_t numIterations;

  ContainerPtr loadProteinPairs(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, const String& description)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteinCount, T("Loading ") + description + T(" proteins"));
    context.informationCallback(String(proteins ? (int)proteins->getNumElements() : 0) + T(" ") + description + T(" proteins"));
    return proteins;
  }

  ProteinEvaluatorPtr createEvaluator(bool isFinalEvaluation) const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->addEvaluator(dsbTarget,  symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation), 1), T("Disulfide Bonds (Sens. and Spec)"));
    evaluator->addEvaluator(dsbTarget,  symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore, isFinalEvaluation), 1), T("Disulfide Bonds (MCC)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore), 1), T("Disulfide Bonds (Raw)"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0), T("Disulfide Bonds (Greedy L=6)"));    
    return evaluator;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
