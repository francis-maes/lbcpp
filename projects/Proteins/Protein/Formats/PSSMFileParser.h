/*-----------------------------------------.---------------------------------.
| Filename: PSSMFileParser.cpp             | PSSM Parser                     |
| Author  : Julien Becker                  |                                 |
| Started : 22/04/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PSSM_FILE_PARSER_H_
# define LBCPP_PROTEIN_PSSM_FILE_PARSER_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class PSSMFileParser : public TextParser
{
public:
  PSSMFileParser(const File& file, VectorPtr primarySequence)
    : TextParser(file), primarySequence(primarySequence)
  {
    //std::cout << "AA: " << aminoAcidSequence->toString() << std::endl;
  }
  
  virtual TypePtr getElementsType() const
    {return vectorClass(vectorClass(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration())));}

  virtual void parseBegin()
  {
    currentPosition = -3;

    // FIXME: pssm = new Vector(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), n);
    // ne pas créer de Protein pour ca

    ProteinPtr protein; // hmm ... not realy nice
    protein->setPrimaryStructure(primarySequence);
    pssm = protein->createEmptyPositionSpecificScoringMatrix();
  }

  virtual bool parseLine(const String& line)
  {
    if (currentPosition <= -2)
    {
      ++currentPosition;
      return true;
    }

    if (currentPosition == -1)
    {
      tokenize(line, aminoAcidsIndex);
      if (aminoAcidsIndex.size() != 40)
      {
        callback.errorMessage(T("PSSMFileParser::parseLine"), T("Could not recognize PSSM file format"));
        return false;
      }
      aminoAcidsIndex.resize(20);
      ++currentPosition;
      return true;
    }

    if (currentPosition >= (int)primarySequence->size())
      return true; // skip

    if (line.length() < 73)
    {
      callback.errorMessage(T("PSSMFileParser::parseLine"), T("The line is not long enough"));
      return false;
    }

    String serialNumber = line.substring(0, 5).trim();
    if (serialNumber.getIntValue() != currentPosition + 1)
    {
      callback.errorMessage(T("PSSMFileParser::parseLine"), T("Invalid serial number ") + lbcpp::toString(serialNumber));
      return false;
    }   

    String aminoAcid = line.substring(6, 7);
    if (AminoAcid::fromOneLetterCode(aminoAcid[0]) != primarySequence->getVariable(currentPosition))
    {
      callback.errorMessage(T("PSSMFileParser::parseLine"), T("Amino acid does not match at position ") + lbcpp::toString(currentPosition));
      return false;
    }

    size_t numAminoAcids = aminoAcidTypeEnumeration()->getNumElements();
#if 0 // FIXME: use DiscreteProbabilityDistributionPtr scores = new DiscreteProbabilityDistribution(aminoAcidTypeEnumeration());
    double scores[numAminoAcids];
    for (size_t i = 0; i < numAminoAcids; ++i)
    {
      int begin = 10 + i * 3;
      String score = line.substring(begin, begin + 3).trim();
      if (!score.containsOnly(T("0123456789-")))
      {
        callback.errorMessage(T("PSSMFileParser::parseLine"), T("Invalid score: ") + score);
        return false;
      }
      int scoreI = score.getIntValue();
      int index = aminoAcidTypeEnumeration()->getOneLetterCodes().indexOf(aminoAcidsIndex[i]);
      if (index < 0)
      {
        callback.errorMessage(T("PSSMFileParser::parseLine"), T("Unknown amino acid: ") + aminoAcidsIndex[i]);
        return false;
      }
      scores[index] = normalize(scoreI);
      setPssmScore(currentPosition, index, scores[index]);
    }

    String gapScore = line.substring(153, 157).trim();
    setPssmScore(currentPosition, 20, gapScore.getDoubleValue());
    setPssmScore(currentPosition, 21, entropy(scores) / 10.);
#endif // 0

    ++currentPosition;
    return true;
  }

  virtual bool parseEnd()
  {
    setResult(pssm);
    return true;
  }
  
protected:
  VectorPtr primarySequence;
  VectorPtr pssm;
  std::vector<String> aminoAcidsIndex;
  int currentPosition;
  
  double normalize(int score) const
    {return (score <= -5.0) ? 0.0 : (score >= 5.0) ? 1.0 : 0.5 + 0.1 * score;}
  
  double entropy(double values[]) const
  {
    double res = 0;
    for (size_t i = 0; i < aminoAcidTypeEnumeration()->getNumElements(); ++i)
      if (values[i])
        res -= values[i] * log2(values[i]);
    return res;
  }
  
  void setPssmScore(size_t position, size_t index, double value)
  {
    pssm->getVariable(position).getObjectAndCast<DiscreteProbabilityDistribution>()->setVariable(index, value);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PSSM_FILE_PARSER_H_
