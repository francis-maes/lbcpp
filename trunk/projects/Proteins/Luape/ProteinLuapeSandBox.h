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
    : protein(proteinPerception), position(index)
  {
    aminoAcidType = (AminoAcidType)protein->getPrimaryStructure()->getElement(index).getInteger();
    pssmRow = getDenseDoubleVector(protein->getPositionSpecificScoringMatrix(), index);
    secondaryStructure = getDenseDoubleVector(protein->getSecondaryStructure(), index);
    dsspSecondaryStructure = getDenseDoubleVector(protein->getDSSPSecondaryStructure(), index);
    structuralAlphabet = getDenseDoubleVector(protein->getStructuralAlphabetSequence(), index);
    solventAccessibilityAt20p = getProbability(protein->getSolventAccessibilityAt20p(), index);
    disordered = getProbability(protein->getDisorderRegions(), index);
  }
  ProteinResiduePerception() : position((size_t)-1), aminoAcidType(totalNumAminoAcids), solventAccessibilityAt20p(doubleMissingValue), disordered(doubleMissingValue) {}

  void setPrevious(const ProteinResiduePerceptionPtr& previous)
  {
    this->previous = previous;
    previous->next = this;
  }

protected:
  friend class ProteinResiduePerceptionClass;

  ProteinPerceptionPtr protein;
  size_t position;

  ProteinResiduePerceptionPtr previous;
  ProteinResiduePerceptionPtr next;

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
    for (size_t i = 0; i < n; ++i)
    {
      ProteinResiduePerceptionPtr residue = new ProteinResiduePerception(this, protein, i);
      residues->set(i, residue);
      if (i > 0)
        residue->setPrevious(residues->getAndCast<ProteinResiduePerception>(i - 1));
    }

    // cysteins
    cysteinIndices = protein->getCysteinIndices();
  }
  ProteinPerception() {}

  size_t getNumResidues() const
    {return residues->getNumElements();}

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

//////////////////////////////////////////////
////////// Protein Predictor Parameters //////
//////////////////////////////////////////////

class LuapeProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  Variable createProteinPerceptionFunction(ExecutionContext& context, const Variable& input) const
    {return new ProteinPerception(input.getObjectAndCast<Protein>());}

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
    {jassert(false); return FunctionPtr();}

  // ProteinPerception -> SymmetricMatrix[Cystein Pair Perception]
  virtual FunctionPtr createDisulfideSymmetricResiduePairVectorPerception() const
    {return lbcppMemberUnaryFunction(LuapeProteinPredictorParameters, createCysteinPairPerceptionFunction,
                                      proteinPerceptionClass, objectSymmetricMatrixClass(proteinResiduePairPerceptionClass));}

  /*
  ** Learning machines
  */
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    jassert(false);
    return FunctionPtr();
  }

  virtual FunctionPtr disulfideBondPredictor(ProteinTarget target) const
  {
    LuapeInferencePtr classifier = binaryClassifier(target).staticCast<LuapeInference>();
    classifier->addInput(proteinResiduePairPerceptionClass, "residuePair");
    return mapNSymmetricMatrixFunction(classifier, 1);
  }

  virtual FunctionPtr binaryClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine = new LuapeBinaryClassifier();

    
    learningMachine->setLearner(adaBoostLearner(binaryTreeWeakLearner(singleStumpWeakLearner(), singleStumpWeakLearner())), 100);
    return learningMachine;
  }

  virtual FunctionPtr multiClassClassifier(ProteinTarget target) const
  {
    LuapeInferencePtr learningMachine =  new LuapeClassifier();
    learningMachine->setLearner(adaBoostMHLearner(binaryTreeWeakLearner(singleStumpWeakLearner(), singleStumpWeakLearner()), true), 100);
    return learningMachine;
  }

  virtual FunctionPtr regressor(ProteinTarget target) const
  {
    jassert(false);
    return FunctionPtr();
  }
};

//////////////////////////////////////////////
///////////////  SAND BOX      ///////////////
//////////////////////////////////////////////

class ProteinLuapeSandBox : public WorkUnit
{
public:
  ProteinLuapeSandBox() : maxProteinCount(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr trainingProteins = loadProteinPairs(context, trainingInputDirectory, trainingSupervisionDirectory, "training");
    ContainerPtr testingProteins = loadProteinPairs(context, testingInputDirectory, testingSupervisionDirectory, "testing");
    if (!trainingProteins || !testingProteins)
      return false;

    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(20);

#if 0
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    /*predictor->learningMachineName = T("ExtraTrees");
    predictor->x3Trees = 100;
    predictor->x3Attributes = 0;
    predictor->x3Splits = 1;*/
    predictor->learningMachineName = "kNN";
    predictor->knnNeighbors = 5;
#endif
     
    ProteinPredictorParametersPtr predictor = new LuapeProteinPredictorParameters();

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    iteration->addTarget(dsbTarget);

    if (!iteration->train(context, trainingProteins, ContainerPtr(), T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
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

  ContainerPtr loadProteinPairs(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, const String& description)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteinCount, T("Loading ") + description + T(" proteins"));
    context.informationCallback(String(proteins ? (int)proteins->getNumElements() : 0) + T(" ") + description + T(" proteins"));
    return proteins;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
