
#include <lbcpp/Core/Function.h>
#include "../Data/Formats/PSSMFileParser.h"

namespace lbcpp
{

class ExportDisulfidePatternEvaluator : public SupervisedEvaluator
{
public:
  /* SupervisedEvaluator */
  virtual TypePtr getRequiredPredictionType() const
    {return matrixClass(doubleType);}

  virtual TypePtr getRequiredSupervisionType() const
    {return matrixClass(probabilityType);}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
  {
    MatrixPtr predictedMatrix = prediction.getObjectAndCast<Matrix>();
    printToFile(context, T("unmatched.pattern"), predictedMatrix);

    predictedMatrix = (new KolmogorovPerfectMatchingFunction(0.f))->compute(context, predictedMatrix).getObjectAndCast<Matrix>(context);
    printToFile(context, T("matched.pattern"), predictedMatrix);
  }
  
  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new DummyScoreObject();}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score, const FunctionPtr& function) const {}

private:
  void printToFile(ExecutionContext& context, const String& fileName, const MatrixPtr& matrix) const
  {
    OutputStream* o = context.getFile(fileName).createOutputStream();
    for (size_t i = 0; i < matrix->getNumRows(); ++i)
    {
      for (size_t j = 0; j < matrix->getNumColumns(); ++j)
      {
        *o << matrix->getElement(i, j).toString() + T("\t");
      }
      *o << T("\n");
    }
    delete o;
  }
};

class PlosComputationalBiologyJournal : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    // FASTA Parsing
    if (!fastaFile.exists())
    {
      context.errorCallback(T("FASTA file does not exist !"));
      return false;
    }

    ContainerPtr proteins = (new FASTAFileParser(context, fastaFile))->load(1);
    if (!proteins || proteins->getNumElements() == 0)
    {
      context.errorCallback(T("Invalid FASTA file !"));
      return false;
    }
    // PSSM Parsing
    if (!pssmFile.exists())
    {
      context.errorCallback(T("PSSM file does not exist !"));
      return false;
    }

    ProteinPtr protein = proteins->getElement(0).dynamicCast<Protein>();
    VectorPtr pssm = StreamPtr(new PSSMFileParser(context, pssmFile, protein->getPrimaryStructure()))->next().getObjectAndCast<Vector>();
    if (!pssm)
    {
      context.errorCallback(T("Invalid PSSM file !"));
      return false;
    }
    // Compile protein
    protein->setPositionSpecificScoringMatrix(pssm);
    // Dummy supervision
    protein->setDisulfideBonds(symmetricMatrix(probabilityType, protein->getCysteinIndices().size()));

    proteins = proteins->apply(context, proteinToInputOutputPairFunction(false));

    // Features
    LargeProteinParametersPtr parameter = new LargeProteinParameters();
    parameter->pssmWindowSize = 15;
    parameter->separationProfilSize = 17;
    // Machine learning
    LargeProteinPredictorParametersPtr learningMachine = new LargeProteinPredictorParameters(parameter);
    learningMachine->learner = Function::createFromFile(context, context.getFile("extraTrees.xml"));
    // Protein predictor
    ProteinPredictorPtr predictor = new ProteinPredictor(learningMachine);
    predictor->addTarget(dsbTarget);
    // Evaluators
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->addEvaluator(dsbTarget, new ExportDisulfidePatternEvaluator(), T("Export Patterns"));

    predictor->evaluate(context, proteins, evaluator, T("Evaluating protein ..."));
    return true;
  }

protected:
  friend class PlosComputationalBiologyJournalClass;

  File fastaFile;
  File pssmFile;
};

}

