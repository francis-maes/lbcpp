#include <lbcpp/lbcpp.h>
#include <lbcpp/ObjectStream.h>
#include "InferenceData/ScoreVectorSequence.h"

class PSSMParser : public TextObjectParser
{
public:
  PSSMParser(const File& file, size_t sequenceLength) : TextObjectParser(file)
    , pssm(new ScoreVectorSequence(T("PositionSpecificScoringMatrix"), AminoAcidDictionary::getInstance(), sequenceLength))
    , numLineToSkip(0)
    , currentPosition(0)
    {}
  
  virtual void parseBegin()
  {
    // Skip header
    numLineToSkip = 3;
  }
  
  virtual bool parseLine(const String& line)
  {
    if (numLineToSkip) {
      --numLineToSkip;
      return true;
    }

    if (line.isEmpty()) {
      // Skip footer
      numLineToSkip = 5;
      setResult(pssm);
      return true;
    }
    
    std::vector< String > columns;
    tokenize(line, columns);
    jassert(columns.size() == 44);
    jassert(currentPosition < pssm->size());
    for (size_t i = 2; i < 22; ++i)
      pssm->setScore(currentPosition, i - 2, normalize(columns[i].getIntValue()));
    ++currentPosition;
    
    return true;
  }
  
  virtual bool parseEnd()
  {
    if (numLineToSkip || !pssm->size()) {
      jassert(false);
      return false;
    }

    return true;
  }
  
protected:
  ScoreVectorSequencePtr pssm;
  size_t numLineToSkip;
  size_t currentPosition;
  
private:
  inline double normalize(int score) const
    {return (score <= -5) ? 0. : (score >= 5) ? 1. : 0.5 + 0.1 * score;}
};