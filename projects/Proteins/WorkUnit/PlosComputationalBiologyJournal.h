
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
      cysString += T("Cys") + String((int)cysteinIndices[i] + 1) + T(" ");
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

}

