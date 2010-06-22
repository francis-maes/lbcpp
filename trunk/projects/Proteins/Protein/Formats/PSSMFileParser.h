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

class PSSMFileParser : public TextObjectParser
{
public:
  PSSMFileParser(const File& file, LabelSequencePtr aminoAcidSequence)
    : TextObjectParser(file), aminoAcidSequence(aminoAcidSequence)
  {
    //std::cout << "AA: " << aminoAcidSequence->toString() << std::endl;
  }

  virtual void parseBegin()
  {
    currentPosition = -3;
    pssm = new ScoreVectorSequence(name, AminoAcidDictionary::getInstance(), aminoAcidSequence->size(), AminoAcidDictionary::numAminoAcids + 2);
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
        Object::error(T("PSSMFileParser::parseLine"), T("Could not recognize PSSM file format"));
        return false;
      }
      aminoAcidsIndex.resize(20);
      ++currentPosition;
      return true;
    }

    if (currentPosition >= (int)aminoAcidSequence->size())
      return true; // skip

    if (line.length() < 73)
    {
      Object::error(T("PSSMFileParser::parseLine"), T("The line is not long enough"));
      return false;
    }

    String serialNumber = line.substring(0, 5).trim();
    if (serialNumber.getIntValue() != currentPosition + 1)
    {
      Object::error(T("PSSMFileParser::parseLine"), T("Invalid serial number ") + lbcpp::toString(serialNumber));
      return false;
    }   

    String aminoAcid = line.substring(6, 7);
    if (aminoAcid != aminoAcidSequence->getString(currentPosition))
    {
      Object::error(T("PSSMFileParser::parseLine"), T("Amino acid does not match at position ") + lbcpp::toString(currentPosition));
      return false;
    }

    double scores[AminoAcidDictionary::numAminoAcids];
    for (size_t i = 0; i < (size_t)AminoAcidDictionary::numAminoAcids; ++i)
    {
      int begin = 10 + i * 3;
      String score = line.substring(begin, begin + 3).trim();
      if (!score.containsOnly(T("0123456789-")))
      {
        Object::error(T("PSSMFileParser::parseLine"), T("Invalid score: ") + score);
        return false;
      }
      int scoreI = score.getIntValue();
      int index = AminoAcidDictionary::getInstance()->getFeatures()->getIndex(aminoAcidsIndex[i]);
      if (index < 0)
      {
        Object::error(T("PSSMFileParser::parseLine"), T("Unknown amino acid: ") + aminoAcidsIndex[i]);
        return false;
      }
      scores[index] = normalize(scoreI);
      pssm->setScore(currentPosition, index, scores[index]);
    }

    String gapScore = line.substring(153, 157).trim();
    pssm->setScore(currentPosition, 20, gapScore.getDoubleValue());
    
    pssm->setScore(currentPosition, 21, entropy(scores));

    ++currentPosition;
    return true;
  }

  virtual bool parseEnd()
  {
    setResult(pssm);
    return true;
  }
  
protected:
  LabelSequencePtr aminoAcidSequence;
  ScoreVectorSequencePtr pssm;
  std::vector<String> aminoAcidsIndex;
  int currentPosition;
  
  double normalize(int score) const
    {return (score <= -5.0) ? 0.0 : (score >= 5.0) ? 1.0 : 0.5 + 0.1 * score;}
  
  double entropy(double values[AminoAcidDictionary::numAminoAcids]) const
  {
    double res = 0;
    for (size_t i = 0; i < AminoAcidDictionary::numAminoAcids; ++i)
      if (values[i])
        res -= values[i] * log2(values[i]);
    return res;
  } 
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PSSM_FILE_PARSER_H_
