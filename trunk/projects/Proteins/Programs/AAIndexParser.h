#include <lbcpp/lbcpp.h>
#include <lbcpp/ObjectStream.h>
#include "InferenceData/ScoreVectorSequence.h"

class AAIndexParser : public TextObjectParser
{
public:
  AAIndexParser(const File& file) : TextObjectParser(file)
  , currentAAIndex(ObjectPtr())
  , parseNextLine(0)
    {}
  
  virtual void parseBegin()
    {}
  
  virtual bool parseLine(const String& line)
  {
    if (parseNextLine) {
      if(!parse(line)) {
        parseNextLine = 0;
        return true;
      }
      if (--parseNextLine == 0)
      {
        normalizeResult();
        setResult(currentAAIndex);
      }
      return true;
    }
    
    if (line == T("I    A/L     R/K     N/M     D/F     C/P     Q/S     E/T     G/W     H/Y     I/V")) {
      parseNextLine = 2;
      currentAAIndex = new ScoreVectorSequence(T("AAIndex"), AminoAcidDictionary::getInstance(), 1);
      return true;
    }
    
    return true;
  }

  virtual bool parseEnd()
    {return true;}

private:
  ScoreVectorSequencePtr currentAAIndex;
  size_t parseNextLine;
  
  bool parse(const String& line)
  {
    std::vector< String > columns;
    tokenize(line, columns);

    jassert(columns.size() == 10);

    static const String letters = T("ARNDCQEGHILKMFPSTWYV");
    
    
    for (size_t i = 0; i < columns.size(); ++i)
    {
      if (columns[i] == T("NA"))
        return false;
      
      size_t index = (parseNextLine - 1) * 10 + i;
      double value = columns[i].getDoubleValue();
      AminoAcidDictionary::Type type = AminoAcidDictionary::getTypeFromOneLetterCode(letters[index]);
      currentAAIndex->setScore(0, type, value);
    }

    return true;
  }
  
  void normalizeResult()
  {
    double minValue = DBL_MAX;
    double maxValue = -DBL_MAX;
    
    for (size_t i = 0; i < 20; ++i)
    {
      double value = currentAAIndex->getScore(0, i);
      
      if (value > maxValue)
        maxValue = value;
      
      if (value < minValue)
        minValue = value;
    }
    
    double sum = 0.;
    for (size_t i = 0; i < 20; ++i)
    {
      currentAAIndex->setScore(0, i, (currentAAIndex->getScore(0, i) - minValue) / (maxValue - minValue) );
      sum += currentAAIndex->getScore(0, i);
    }
    
    // special case
    double aOrAA = currentAAIndex->getScore(0, AminoAcidDictionary::asparagine) + currentAAIndex->getScore(0, AminoAcidDictionary::asparticAcid);
    currentAAIndex->setScore(0, AminoAcidDictionary::asparagineOrAsparticAcid, aOrAA / 2);
    double gOrGA = currentAAIndex->getScore(0, AminoAcidDictionary::glutamine) + currentAAIndex->getScore(0, AminoAcidDictionary::glutamicAcid);
    currentAAIndex->setScore(0, AminoAcidDictionary::glutamineOrGlutamicAcid, gOrGA / 2);
    double lOrIL = currentAAIndex->getScore(0, AminoAcidDictionary::leucine) + currentAAIndex->getScore(0, AminoAcidDictionary::isoleucine);
    currentAAIndex->setScore(0, AminoAcidDictionary::lecineOrIsoleucine, lOrIL / 2);
    
    currentAAIndex->setScore(0, AminoAcidDictionary::unknown, sum / 20);
  }
};