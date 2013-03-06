
#include <lbcpp/Core/Function.h>
#include "../Data/Formats/PSSMFileParser.h"

namespace lbcpp
{

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

    // Add PSSM to the protein
    protein->setPositionSpecificScoringMatrix(pssm);
    protein->setDisulfideBonds(symmetricMatrix(probabilityType, protein->getCysteinIndices().size()));
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

    Variable res = predictor->compute(context, protein, protein);

    exportResult(context, res.getObjectAndCast<Protein>());
    return true;
  }

protected:
  friend class PlosComputationalBiologyJournalClass;

  File fastaFile;
  File pssmFile;
  File outputFile;

private:
  void exportResult(ExecutionContext& context, const ProteinPtr& protein) const
  {
    const SymmetricMatrixPtr& pattern = protein->getDisulfideBonds(context);
    const std::vector<size_t> cysteinIndices = protein->getCysteinIndices();
    jassert(pattern->getDimension() == cysteinIndices.size());
    const size_t n = cysteinIndices.size();

    if (outputFile.exists())
      outputFile.deleteFile();
    
    OutputStream* o = outputFile.createOutputStream();
    *o << "#------------------------------------------#\n";
    *o << "|               x3CysBridges               |-------------------------#\n";
    *o << "#------------------------------------------#          Julien Becker  |\n";
    *o << "                                      |                Francis Maes  |\n";
    *o << "    Disulfide Bonding Probability     |              Louis Wehenkel  |\n";
    *o << "                  &                   #------------------------------#\n";
    *o << "   Disulfide Connectivity Pattern\n";
    *o << "\n";
    *o << "\n";
    *o << "\n";

    String ps = protein->getPrimaryStructure()->toString();
    String formatedPs;
    for (size_t i = 0; i < (size_t)ps.length(); ++i)
    {
      if (i % 50 == 0)
        formatedPs += T("\n     ");
      else if (i % 5 == 0)
        formatedPs += T(" ");
      formatedPs += ps[i];
    }
    
    String cysString;
    for (size_t i = 0; i < n; ++i)
    {
      if (i % 5 == 0)
        cysString += T("\n     ");
      cysString += toFixedLengthStringLeftJustified(T("Cys") + String((int)cysteinIndices[i] + 1), 6) + T(" ");
    }
    
    *o << "#- Query ------------------------------------------------------------#\n";
           
    *o << "Sequence : " << formatedPs << "\n";
    *o << "Cysteines: " << cysString << "\n";
    *o << "\n";
    *o << "\n";

    *o << "#- Disulfide Bonding Probabilities - (Rank, Probability, Cys, Cys) --#\n";

    typedef std::multimap<double, std::pair<size_t, size_t> > ScoresMap;
    ScoresMap bondingProbabilities;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j)
        bondingProbabilities.insert(std::make_pair(pattern->getElement(i, j).getDouble(), std::make_pair(cysteinIndices[i] + 1, cysteinIndices[j] + 1)));
    
    ScoresMap::const_reverse_iterator it = bondingProbabilities.rbegin();
    for (size_t i = 1; it != bondingProbabilities.rend(); ++i, it++)
    {
      if (it->first < 1e-6)
        break;

      *o << toFixedLengthStringLeftJustified(String((int)i), 3) << " ";
      *o << toFixedLengthStringLeftJustified(String(it->first, 5), 7) << " ";
      *o << toFixedLengthStringLeftJustified(T("Cys") + String((int)it->second.first), 6) << " ";
      *o << toFixedLengthStringLeftJustified(T("Cys") + String((int)it->second.second), 6);
      *o << "\n";
    }

    *o << "\n";
    *o << "\n";

    *o << "#- Disulfide Connectivity Pattern - (Cys, Cys) ----------------------#\n";

    SymmetricMatrixPtr kolmogorov = (new KolmogorovPerfectMatchingFunction(0.f))->compute(context, pattern).getObjectAndCast<SymmetricMatrix>(context);
    jassert(kolmogorov && kolmogorov->getDimension() == n);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j)
      {
        if (kolmogorov->getElement(i,j).getDouble() != 0.f)
        {
          *o << toFixedLengthStringLeftJustified(T("Cys") + String((int)cysteinIndices[i] + 1), 6) << " ";
          *o << toFixedLengthStringLeftJustified(T("Cys") + String((int)cysteinIndices[j] + 1), 6) << "\n";
        }
      }

    delete o;
  }
  
  String toFixedLengthStringLeftJustified(const String& str, int length) const
  {
    jassert(str.length() <= length);
    String res = str;
    while (res.length() < length)
      res += T(" ");
    return res;
  }
};

class ExtraTreesCysBridgesLearner : public WorkUnit
{
public:
  Variable run(ExecutionContext& context)
  {
    enum {numFolds = 10};
    size_t numProteinsToLoad = 0;
#if JUCE_MAC && JUCE_DEBUG
    numProteinsToLoad = 10;
#endif
    ContainerPtr cbsProteins = Protein::loadProteinsFromDirectoryPair(context, File(), cbsProteinsDirectory, numProteinsToLoad, T("Loading CBS proteins"));
    ContainerPtr dsbProteins = Protein::loadProteinsFromDirectoryPair(context, File(), dsbProteinsDirectory, numProteinsToLoad, T("Loading DSB proteins"));
    if (!cbsProteins || cbsProteins->getNumElements() == 0
        || !dsbProteins || dsbProteins->getNumElements() == 0)
    {
      context.errorCallback(T("No proteins found !"));
      return false;
    }

    File cbsDirectory = context.getFile(T(".x3CysBridges.cbs"));
    cbsDirectory.createDirectory();

    /* Cystein Bonding State */
    LargeProteinParametersPtr cbsParameters = new LargeProteinParameters();
    cbsParameters->pssmWindowSize = 11;
    cbsParameters->usePSSMGlobalHistogram = true;
    cbsParameters->useNumCysteins = true;

    // Make CBS predictions
    {
      LargeProteinPredictorParametersPtr model = new LargeProteinPredictorParameters(cbsParameters);
      model->learner = binaryClassificationExtraTree(1000, 0, 1, true);

      ProteinPredictorPtr predictor = new ProteinPredictor(model);
      predictor->addTarget(cbsTarget);

      if (!predictor->train(context, cbsProteins, dsbProteins, T("Learning Cystein Bonding State predictor")))
        return false;
      predictor->evaluate(context, dsbProteins, saveToDirectoryEvaluator(cbsDirectory, T(".xml")), T("Saving predictions to directory"));
      
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      evaluator->addEvaluator(cbsTarget, elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore)), T("CBS"), true);
      CompositeScoreObjectPtr scores = predictor->evaluate(context, dsbProteins, evaluator, T("Evaluate CBS on DSB-Proteins"));
      context.informationCallback(T("CBS Error on DSB-Proteins: ") + String(evaluator->getScoreToMinimize(scores)));
    }

    // Save CBS Model
    {
      LargeProteinPredictorParametersPtr model = new LargeProteinPredictorParameters(cbsParameters);
      model->learner = binaryClassificationExtraTree(1000, 0, 1, true, context.getFile(T("x3CysBridges.cbs")));
      
      ProteinPredictorPtr predictor = new ProteinPredictor(model);
      predictor->addTarget(cbsTarget);

      if (!predictor->train(context, cbsProteins, dsbProteins, T("Learning Cystein Bonding State predictor")))
        return false;
      predictor->saveToFile(context, context.getFile(T("x3CysBridges.cbs.xml")));
    }

    dsbProteins = Protein::loadProteinsFromDirectoryPair(context, cbsDirectory, dsbProteinsDirectory, numProteinsToLoad, T("Loading intermediate predictions"));
    // In order to predict bridges for cysteines predicted as bonded,
    // we have to copy cysteine state predictions into the supervised
    // proteins.
    copyCysteineBondingStatePredictions(context, dsbProteins);

    /* Disulfide Connectivity Parttern */
    {
      LargeProteinParametersPtr dsbParameters = new LargeProteinParameters();
      dsbParameters->pssmWindowSize = 15;
      dsbParameters->separationProfilSize = 17;

      LargeProteinPredictorParametersPtr model = new LargeProteinPredictorParameters(dsbParameters);
      model->learner = binaryClassificationExtraTree(1000, 0, 1, true, context.getFile(T("x3CysBridges.dsb")));

      ProteinPredictorPtr predictor = new ProteinPredictor(model);
      predictor->addTarget(odsbTarget);

      if (!predictor->train(context, dsbProteins, ContainerPtr(), T("Learning Disulfide Bond predictor")))
        return false;
      predictor->saveToFile(context, context.getFile(T("x3CysBridges.dsb.xml")));
    }

    cbsDirectory.deleteRecursively();

    return true;
  }

protected:
  friend class ExtraTreesCysBridgesLearnerClass;

  File cbsProteinsDirectory;
  File dsbProteinsDirectory;

  void copyCysteineBondingStatePredictions(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    for (size_t i = 0; i < proteins->getNumElements(); ++i)
      proteins->getElement(i).dynamicCast<Pair>()->getSecond().getObjectAndCast<Protein>()->setCysteinBondingStates(context, proteins->getElement(i).dynamicCast<Pair>()->getFirst().getObjectAndCast<Protein>()->getCysteinBondingStates(context));
  }  
};

}

