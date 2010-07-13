/*-----------------------------------------.---------------------------------.
| Filename: PSSMFileParser.cpp             | PSSM Parser                     |
| Author  : Julien Becker                  |                                 |
| Started : 22/04/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PSSM_FILE_PARSER_H_
# define LBCPP_PROTEIN_PSSM_FILE_PARSER_H_

# include "../Protein.h"

namespace lbcpp
{

class PSSMFileParser : public TextParser
{
public:
  PSSMFileParser(const File& file, VectorPtr primaryStructure)
    : TextParser(file), primaryStructure(primaryStructure)
    {}
  
  virtual TypePtr getElementsType() const
    {return vectorClass(vectorClass(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration())));}

  virtual void parseBegin()
  {
    currentPosition = -3;

    pssm = new Vector(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), primaryStructure->getNumVariables());
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

    if (currentPosition >= (int)primaryStructure->size())
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
    if (AminoAcid::fromOneLetterCode(aminoAcid[0]) != primaryStructure->getVariable(currentPosition))
    {
      callback.errorMessage(T("PSSMFileParser::parseLine"), T("Amino acid does not match at position ") + lbcpp::toString(currentPosition));
      return false;
    }

    size_t numAminoAcids = aminoAcidTypeEnumeration()->getNumElements();

    DiscreteProbabilityDistributionPtr scores = new DiscreteProbabilityDistribution(aminoAcidTypeEnumeration());
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
      scores->setVariable(index, normalize(scoreI));
    }

    String gapScore = line.substring(153, 157).trim();
    scores->setVariable(20, gapScore.getDoubleValue());
    scores->setVariable(21, scores->computeEntropy());

    pssm->setVariable(currentPosition, scores);

    ++currentPosition;
    return true;
  }

  virtual bool parseEnd()
  {
    setResult(pssm);
    return true;
  }
  
protected:
  VectorPtr primaryStructure;
  VectorPtr pssm;
  std::vector<String> aminoAcidsIndex;
  int currentPosition;
  
  double normalize(int score) const
    {return (score <= -5.0) ? 0.0 : (score >= 5.0) ? 1.0 : 0.5 + 0.1 * score;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PSSM_FILE_PARSER_H_
