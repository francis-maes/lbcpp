/*-----------------------------------------.---------------------------------.
| Filename: SPXFileParser.h                | SPX File parser                 |
| Author  : Julien Becker                  |                                 |
| Started : 28/03/2011 21:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_SPX_FILE_PARSER_H_
# define LBCPP_PROTEINS_FORMATS_SPX_FILE_PARSER_H_

# include "../Protein.h"
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class SPXFileParser : public TextParser
{
public:
  SPXFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), proteinLength(0), numBondedCysteins(0), numCysteins(0) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin()
    {}

  virtual bool parseLine(const String& line)
  {
    String trimmedLine = line.trim();

    if (trimmedLine.startsWithChar(T('#')))
      return true;

    if (trimmedLine == String::empty)
    {
      if (currentProtein)
      {
        currentProtein->setDisulfideBonds(disulfideBonds);
        setResult(currentProtein);
        currentProtein = ProteinPtr();
        proteinLength = 0;
        numBondedCysteins = 0;
        numCysteins = 0;
        disulfideBonds = SymmetricMatrixPtr();
        expectedLineType = noLine;
      }
      return true;
    }

    if (!currentProtein)
    {
      currentProtein = new Protein(trimmedLine);
      return true;
    }

    if (proteinLength == 0)
    {
      StringArray tokens;
      tokens.addTokens(trimmedLine, false);
      jassert(tokens.size() == 3);

      Variable v = Variable::createFromString(context, positiveIntegerType, tokens[0]);
      jassert(v.exists() && v.getInteger() > 0);
      proteinLength = v.getInteger();

      v = Variable::createFromString(context, positiveIntegerType, tokens[1]);
      jassert(v.exists() && v.getInteger() > 0);
      numBondedCysteins = v.getInteger();

      v = Variable::createFromString(context, positiveIntegerType, tokens[2]);
      jassert(v.exists() && v.getInteger() > 0);
      numCysteins = v.getInteger();

      expectedLineType = primaryStructure;
      return true;
    }

    if (expectedLineType == primaryStructure)
    {
      jassert((size_t)trimmedLine.length() == proteinLength);
      currentProtein->setPrimaryStructure(trimmedLine);
      jassert(currentProtein->getCysteinIndices().size() == numCysteins);
      expectedLineType = secondaryStructure;
      return true;
    }
    
    if (expectedLineType == secondaryStructure)
    {
      jassert((size_t)trimmedLine.length() == proteinLength);
      ContainerPtr v = Protein::createEmptyDSSPSecondaryStructure(proteinLength, true);
      for (size_t i = 0; i < proteinLength; ++i)
      {
        int dsspIndex = dsspSecondaryStructureElementEnumeration->findElementByOneLetterCode(trimmedLine[i]);
        jassert(dsspIndex >= 0);
        SparseDoubleVectorPtr value = new SparseDoubleVector(dsspSecondaryStructureElementEnumeration, probabilityType);
        value->appendValue(dsspIndex, 1.0);
        v->setElement(i, value);
      }
      currentProtein->setDSSPSecondaryStructure(v);
      expectedLineType = solventAccessibility;
      return true;
    }

    if (expectedLineType == solventAccessibility)
    {
      jassert((size_t)trimmedLine.length() == proteinLength);
      DoubleVectorPtr v = Protein::createEmptyProbabilitySequence(proteinLength);
      for (size_t i = 0; i < proteinLength; ++i)
      {
        if (trimmedLine[i] == T('e'))
          v->setElement(i, probability(1.0));
        else if (trimmedLine[i] == T('-'))
          v->setElement(i, probability(0.0));
        else
          jassertfalse;
      }
      expectedLineType = ssBond;
      currentProtein->setSolventAccessibilityAt20p(v);
      return true;
    }

    if (expectedLineType == ssBond)
    {
      if (!disulfideBonds)
        disulfideBonds = symmetricMatrix(probabilityType, currentProtein->getCysteinIndices().size());

      StringArray tokens;
      tokens.addTokens(trimmedLine, false);
      jassert(tokens.size() == 6);

      const std::vector<int>& cysteinInvIndices = currentProtein->getCysteinInvIndices();

      Variable v = Variable::createFromString(context, positiveIntegerType, tokens[1]);
      jassert(v.exists() && v.getInteger() > 0);
      size_t firstCysteinIndex = cysteinInvIndices[v.getInteger() - 1];
      jassert(firstCysteinIndex >= 0);

      v = Variable::createFromString(context, positiveIntegerType, tokens[4]);
      jassert(v.exists() && v.getInteger() > 0);
      size_t secondCysteinIndex = cysteinInvIndices[v.getInteger() - 1];
      jassert(secondCysteinIndex >= 0);

      disulfideBonds->setElement(firstCysteinIndex, secondCysteinIndex, probability(1.0));
      return true;
    }
    return false;
  }
  
  virtual bool parseEnd()
  {
    if (currentProtein)
    {
      currentProtein->setDisulfideBonds(disulfideBonds);
      setResult(currentProtein);
    }
    return true;
  }

private:
  ProteinPtr currentProtein;
  size_t proteinLength;
  size_t numBondedCysteins;
  size_t numCysteins;
  SymmetricMatrixPtr disulfideBonds;
  
  enum {noLine = 0, primaryStructure, secondaryStructure, solventAccessibility, ssBond} expectedLineType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_SPX_FILE_PARSER_H_
