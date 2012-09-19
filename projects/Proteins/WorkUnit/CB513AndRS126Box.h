#include "Data/Protein.h"

namespace lbcpp
{

/*
 * Input:
 *  RES:X,X,X,...,
 *  DSSP:X,X,X,...,
 *  XXXXX
 * Ouput:
 *  Protein
 */
class CB513FileParser : public TextParser
{
public:
  CB513FileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), fileName(file.getFileNameWithoutExtension()) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine.trim();

    if (line.startsWith(T("RES:")))
    {
      jassert(!protein);

      std::cout << line << std::endl;
      line = line.substring(4);
      line = line.removeCharacters(T(","));
      std::cout << line << std::endl;

      protein = new Protein(fileName);
      protein->setPrimaryStructure(line);
      return true;
    }

    if (line.startsWith(T("DSSP:")))
    {
      jassert(protein);

      line = line.substring(5);
      line = line.removeCharacters(T(","));

      jassert(protein->getLength() == (size_t)line.length());
      
      VectorPtr dsspSecondaryStructureSequence = Protein::createEmptyDSSPSecondaryStructure(protein->getLength(), true);
      EnumerationPtr dsspEnum = dsspSecondaryStructureElementEnumeration;

      for (size_t i = 0; i < (size_t)line.length(); ++i)
      {
        juce::tchar secondaryStructureCode = line[i];
        if (secondaryStructureCode == T('_') || secondaryStructureCode == T('?'))
          secondaryStructureCode = T('C');

        int secondaryStructureIndex = dsspEnum->findElementByOneLetterCode(secondaryStructureCode);
        if (secondaryStructureIndex < 0)
        {
          context.errorCallback(T("CB513FileParser::parseLine"), fileName + T(" - Unrecognized secondary structure code: '") + secondaryStructureCode + T("'"));
          return false;
        }
        SparseDoubleVectorPtr value = new SparseDoubleVector(dsspEnum, probabilityType);
        value->appendValue(secondaryStructureIndex, 1.0);
        dsspSecondaryStructureSequence->setElement(i, value);
      }
      protein->setDSSPSecondaryStructure(dsspSecondaryStructureSequence);
      return true;
    }

    return true;
  }
  
  virtual bool parseEnd()
  {
    if (!protein)
    {
      context.errorCallback(T("CB513FileParser::parseEnd"), T("No protein parsed"));
      return false;
    }
    
    if (!protein->getDSSPSecondaryStructure())
    {
      context.errorCallback(T("CB513FileParser::parseEnd"), T("No DSSP parsed"));
      return false;
    }

    setResult(protein);
    return true;
  }
  
protected:
  String fileName;
  ProteinPtr protein;
};

class ConvertCB513FileToProtein : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertCB513FileToProtein::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*"));
    size_t correctlyConvected = 0;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      ProteinPtr protein = StreamPtr(new CB513FileParser(context, *files[i]))->next().getObjectAndCast<Protein>();
      if (!protein)
      {
        context.errorCallback(T("ConvertCB513FileToProtein::run"), T("No protein parsed in file: ") + files[i]->getFullPathName());
        continue;
      }
      protein->saveToFile(context, outputDirectory.getChildFile(protein->getName() + T(".xml")));
      ++correctlyConvected;
    }

    context.informationCallback(T("Parsed protein: ") + String((int)correctlyConvected) + T("/") + String(files.size()));
    return true;
  }
  
protected:
  friend class ConvertCB513FileToProteinClass;
  
  File inputDirectory;
  File outputDirectory;
};

/*
 * Input:
 *  seq_NAME:XXX...
 *  dssp:XXX...
 *  XXXXX
 * Ouput:
 *  Protein
 */
class RS126FileParser : public TextParser
{
public:
  RS126FileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), fileName(file.getFileNameWithoutExtension()) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine.trim();

    String prefix = T("seq_") + fileName +T(":");
    if (line.startsWith(prefix))
    {
      jassert(!protein);

      line = line.substring(prefix.length());
      std::cout << line << std::endl;

      protein = new Protein(fileName);
      protein->setPrimaryStructure(line);
      return true;
    }

    if (line.startsWith(T("dssp:")))
    {
      jassert(protein);

      line = line.substring(5);
      line = line.removeCharacters(T(","));

      jassert(protein->getLength() == (size_t)line.length());
      
      VectorPtr dsspSecondaryStructureSequence = Protein::createEmptyDSSPSecondaryStructure(protein->getLength(), true);
      EnumerationPtr dsspEnum = dsspSecondaryStructureElementEnumeration;

      for (size_t i = 0; i < (size_t)line.length(); ++i)
      {
        juce::tchar secondaryStructureCode = line[i];
        if (secondaryStructureCode == T('-'))
          secondaryStructureCode = T('C');

        int secondaryStructureIndex = dsspEnum->findElementByOneLetterCode(secondaryStructureCode);
        if (secondaryStructureIndex < 0)
        {
          context.errorCallback(T("RS126FileParser::parseLine"), fileName + T(" - Unrecognized secondary structure code: '") + secondaryStructureCode + T("'"));
          return false;
        }
        SparseDoubleVectorPtr value = new SparseDoubleVector(dsspEnum, probabilityType);
        value->appendValue(secondaryStructureIndex, 1.0);
        dsspSecondaryStructureSequence->setElement(i, value);
      }
      protein->setDSSPSecondaryStructure(dsspSecondaryStructureSequence);
      return true;
    }

    return true;
  }
  
  virtual bool parseEnd()
  {
    if (!protein)
    {
      context.errorCallback(T("RS126FileParser::parseEnd"), T("No protein parsed"));
      return false;
    }
    
    if (!protein->getDSSPSecondaryStructure())
    {
      context.errorCallback(T("RS126FileParser::parseEnd"), T("No DSSP parsed"));
      return false;
    }

    setResult(protein);
    return true;
  }
  
protected:
  String fileName;
  ProteinPtr protein;
};

class ConvertRS126FileToProtein : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertRS126FileToProtein::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*"));
    size_t correctlyConvected = 0;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      ProteinPtr protein = StreamPtr(new RS126FileParser(context, *files[i]))->next().getObjectAndCast<Protein>();
      if (!protein)
      {
        context.errorCallback(T("ConvertRS126FileToProtein::run"), T("No protein parsed in file: ") + files[i]->getFullPathName());
        continue;
      }
      protein->saveToFile(context, outputDirectory.getChildFile(protein->getName() + T(".xml")));
      ++correctlyConvected;
    }

    context.informationCallback(T("Parsed protein: ") + String((int)correctlyConvected) + T("/") + String(files.size()));
    return true;
  }
  
protected:
  friend class ConvertRS126FileToProteinClass;
  
  File inputDirectory;
  File outputDirectory;
};

/*
 * Input:
 *  XXX
 *  ATOM ...
 *  ...
 *  XXX
 * Ouput:
 *  NULL
 */
class ASTRALFileParser : public TextParser
{
public:
  ASTRALFileParser(ExecutionContext& context, const File& file, const File& outputFile)
    : TextParser(context, file), fileName(file.getFileNameWithoutExtension()),
      outputFile(outputFile) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine;
    if (line.startsWith(T("HEADER")))
      headerLine = line;
    
    if (!line.startsWith(T("ATOM")))
      return true;

    line = line.replaceSection(21, 1, T(" "));
    atomLines.push_back(line);

    String residueNumber = line.substring(23, 26);
    if (residueNumber == previousResidueNumber)
      return true;

    previousResidueNumber = residueNumber;
    String residueName = line.substring(17, 20);
    residueNames.push_back(residueName);

    return true;
  }
  
  virtual bool parseEnd()
  {
    OutputStream* o = outputFile.createOutputStream();
    
    *o << headerLine << '\n';

    /* Primary Structure */
    const size_t n = residueNames.size();
    size_t firstResidueIndex = 0;
    size_t seqResIndex = 1;
    while (firstResidueIndex < n)
      *o << makeSeqResLine(seqResIndex++, String::empty, n, residueNames, firstResidueIndex) << '\n';

    /* PDB file */
    for (size_t i = 0; i < atomLines.size(); ++i)
      *o << atomLines[i] << '\n';
    
    *o << "END" << '\n';
    
    delete o;

    return true;
  }
  
protected:
  String headerLine;
  std::vector<String> atomLines;
  
  String fileName;
  File outputFile;
  std::vector<String> residueNames;
  String previousResidueNumber;

  String makeSeqResLine(size_t serialNumber, const String& chainId, size_t numResidues,
                        const std::vector<String>& residues, size_t& firstResidueIndex)
  {
    String line = T("SEQRES");                                                jassert(line.length() == 6);
    line += T(" ");                                                           jassert(line.length() == 7);
    line += toFixedLengthStringRightJustified(String((int)serialNumber), 3);  jassert(line.length() == 10);
    line += T(" ");                                                           jassert(line.length() == 11);
    line += toFixedLengthString(chainId, 1);                                  jassert(line.length() == 12);
    line += T(" ");                                                           jassert(line.length() == 13);
    line += toFixedLengthStringRightJustified(String((int)numResidues), 4);   jassert(line.length() == 17);
    line += T("  ");                                                          jassert(line.length() == 19);
    
    for (size_t i = 0; i < 13; ++i)
    {
      if (firstResidueIndex < residues.size())
      {
        line += toFixedLengthString(residues[firstResidueIndex], 3);
        ++firstResidueIndex;
      }
      else
        line += T("   ");
      line += T(" ");
    }
    jassert(line.length() == 71);
    return toFixedLengthStringLeftJustified(line, 80);
  }

  String toFixedLengthStringRightJustified(const String& str, int length)
  {
    jassert(str.length() <= length);
    String res = str;
    while (res.length() < length)
      res = T(" ") + res;
    return res;
  }

  String toFixedLengthStringLeftJustified(const String& str, int length)
  {
    jassert(str.length() <= length);
    String res = str;
    while (res.length() < length)
      res += T(" ");
    return res;
  }

  String toFixedLengthString(const String& str, int length)
  {
    jassert(str.length() <= length);
    String res = str;
    int i = 0;
    while (res.length() < length)
    {
      if (i % 2)
        res = T(" ") + res;
      else
        res = res + T(" ");
      ++i;
    }
    return res;
  }
};

class ConvertASTRALFileToPDB : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertASTRALFileToPDB::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*"));
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      StreamPtr parser = new ASTRALFileParser(context, *files[i], outputDirectory.getChildFile(files[i]->getFileNameWithoutExtension() + T(".pdb")));
      parser->next();
    }

    return true;
  }
  
protected:
  friend class ConvertASTRALFileToPDBClass;
  
  File inputDirectory;
  File outputDirectory;
};

};
