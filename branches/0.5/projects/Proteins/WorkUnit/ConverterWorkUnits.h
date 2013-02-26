#include "Data/Protein.h"
#include "Data/Formats/TDBFileParser.h"

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
      outputFile(outputFile), terLine(false) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine;
    if (line.startsWith(T("HEADER")))
      headerLine = line;

    if (line.startsWith(T("TER ")))
      terLine = true;

    if (terLine || !line.startsWith(T("ATOM")))
      return true;

    line = line.replaceSection(21, 1, T(" "));
    atomLines.push_back(line);

    String residueNumber = line.substring(23, 27);
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
  String fileName;
  File outputFile;

  bool terLine;
  String headerLine;
  std::vector<String> atomLines;
  
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

class ConvertTDBFileToProtein : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertTDBFileToProtein::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*"));
    size_t correctlyConvected = 0;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      ProteinPtr protein = StreamPtr(new TDBFileParser(context, *files[i]))->next().getObjectAndCast<Protein>();
      if (!protein)
      {
        context.errorCallback(T("ConvertTDBFileToProtein::run"), T("No protein parsed in file: ") + files[i]->getFullPathName());
        continue;
      }
      protein->saveToFile(context, outputDirectory.getChildFile(protein->getName() + T(".xml")));
      ++correctlyConvected;
    }
    
    context.informationCallback(T("Parsed protein: ") + String((int)correctlyConvected) + T("/") + String(files.size()));
    return true;
  }
  
protected:
  friend class ConvertTDBFileToProteinClass;
  
  File inputDirectory;
  File outputDirectory;
};

class ConvertPDBToProteinWorkUnit : public WorkUnit
{
public:
  ConvertPDBToProteinWorkUnit() : outputFile(File::getCurrentWorkingDirectory().getChildFile(T("output"))) {}
  
  virtual String toString() const
    {return T("Convert a PDB file to a Protein file and vice-versa.");}
  
  virtual Variable run(ExecutionContext& context)
  {
    if (!inputFile.exists())
    {
      context.errorCallback(T("ConvertPDBToProteinWorkUnit::run"), T("Input file ") + inputFile.getFileName().quoted() +(" does not exists"));
      return false;
    }
    
    if (inputFile.getFileExtension() != T(".pdb") && inputFile.getFileExtension() != T(".xml"))
    {
      context.errorCallback(T("ConvertPDBToProteinWorkUnit::run"), T("Input file is not valid"));
      return false;
    }
    
    if (inputFile.getFileExtension() == T(".pdb") && !convertPDBToProtein(context, inputFile, outputFile))
    {
      context.errorCallback(T("ConvertPDBToProteinWorkUnit::run"), T("Input file is not a valid PDB"));
      return false;
    }
    
    if (inputFile.getFileExtension() == T(".xml") && !convertProteinToPDB(context, inputFile, outputFile))
    {
      context.errorCallback(T("ConvertPDBToProteinWorkUnit::run"), T("Input file is not a valid XML"));
      return false;
    }
    
    return true;
  }
  
protected:
  friend class ConvertPDBToProteinWorkUnitClass;
  
  File inputFile;
  File outputFile;

  bool convertPDBToProtein(ExecutionContext& context, const File& inputFile, const File& outputFile)
  {
    ProteinPtr protein = Protein::createFromPDB(context, inputFile, false);
    if (!protein)
      return false;
    
    Variable(protein).saveToFile(context, outputFile);
    return true;
  }
  
  bool convertProteinToPDB(ExecutionContext& context, const File& inputFile, const File& outputFile)
  {
    ProteinPtr protein = Protein::createFromXml(context, inputFile);
    if (!protein)
      return false;
    
    protein->saveToPDBFile(context, outputFile);
    return true;
  }  
};

class ConvertSPXFileToProteins : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (!spxFile.exists())
      return false;

    ConsumerPtr consumer = saveToFileConsumer(outputDirectory);
    consumer->consumeStream(context, new SPXFileParser(context, spxFile));

    return true;
  }

protected:
  friend class ConvertSPXFileToProteinsClass;

  File spxFile;
  File outputDirectory;
};

/*
 * Input:
 *  (blank line)
 *  Name
 *  Num. residue
 *  Sequence AA
 *  Sequence predicted SS3 (H, E, C)
 *  Sequence predictied solvent accessibility (e, b at 25%)
 *  Sequence disordered residue (T, N)
 * Ouput:
 *  Protein
 */
class DisproFileParser : public TextParser
{
public:
  DisproFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), proteinLength((size_t)-1) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine.trim();
    if (line.startsWith(T("#")) || line == String::empty)
      return true;

    if (!protein)
    {
      protein = new Protein(line);
      return true;
    }

    if (proteinLength == (size_t)-1)
    {
      if (!line.containsOnly(T("0123456789")))
      {
        context.errorCallback(T("DisproFileParser::parseLine"),
                              T("Invalid length - ") + line +
                              T("is not an integer"));
        return false;
      }
      proteinLength = (size_t)line.getIntValue();
      return true;
    }

    if (!protein->getPrimaryStructure())
    {
      if ((size_t)line.length() != proteinLength)
      {
        context.errorCallback(T("DisproFileParser::parseLine"),
                              T("Invalid sequence length: ") +
                              String(line.length()) + T("instead of ") +
                              String((int)proteinLength));
        return false;        
      }
      protein->setPrimaryStructure(line);
      return true;
    }
    
    if (!protein->getSecondaryStructure())
    {
      if ((size_t)line.length() != proteinLength)
      {
        context.errorCallback(T("DisproFileParser::parseLine"),
                              T("Invalid SS3 sequence length: ") +
                              String(line.length()) + T("instead of ") +
                              String((int)proteinLength));
        return false;
      }
      DoubleVectorPtr helix = new SparseDoubleVector(secondaryStructureElementEnumeration, probabilityType);
      helix->setElement(0, 1.f);
      DoubleVectorPtr sheet = new SparseDoubleVector(secondaryStructureElementEnumeration, probabilityType);
      sheet->setElement(1, 1.f);
      DoubleVectorPtr coil = new SparseDoubleVector(secondaryStructureElementEnumeration, probabilityType);
      coil->setElement(2, 1.f);
      
      VectorPtr ss3 = objectVector(sparseDoubleVectorClass(secondaryStructureElementEnumeration, probabilityType), proteinLength);
      for (size_t i = 0; i < proteinLength; ++i)
        if (line[i] == T('H'))
          ss3->setElement(i, helix);
        else if (line[i] == T('E'))
          ss3->setElement(i, sheet);
        else
          ss3->setElement(i, coil);
      protein->setSecondaryStructure(ss3);
      return true;
    }
    
    if (!protein->getSolventAccessibilityAt20p())
    {
      if ((size_t)line.length() != proteinLength)
      {
        context.errorCallback(T("DisproFileParser::parseLine"),
                              T("Invalid SA sequence length: ") +
                              String(line.length()) + T("instead of ") +
                              String((int)proteinLength));
        return false;
      }
      DenseDoubleVectorPtr sa20 = Protein::createEmptyProbabilitySequence(proteinLength);
      for (size_t i = 0; i < proteinLength; ++i)
        sa20->setElement(i, probability(line[i] == T('e') ? 1.f : 0.f));
      protein->setSolventAccessibilityAt20p(sa20);
      return true;
    }

    if (!protein->getSolventAccessibilityAt20p())
    {
      if ((size_t)line.length() != proteinLength)
      {
        context.errorCallback(T("DisproFileParser::parseLine"),
                              T("Invalid SA sequence length: ") +
                              String(line.length()) + T("instead of ") +
                              String((int)proteinLength));
        return false;
      }
      DenseDoubleVectorPtr sa20 = Protein::createEmptyProbabilitySequence(proteinLength);
      for (size_t i = 0; i < proteinLength; ++i)
        sa20->setElement(i, probability(line[i] == T('e') ? 1.f : 0.f));
      protein->setSolventAccessibilityAt20p(sa20);
      return true;
    }

    if ((size_t)line.length() != proteinLength)
    {
      context.errorCallback(T("DisproFileParser::parseLine"),
                            T("Invalid DR sequence length: ") +
                            String(line.length()) + T("instead of ") +
                            String((int)proteinLength));
      return false;
    }
    DenseDoubleVectorPtr dr = Protein::createEmptyProbabilitySequence(proteinLength);
    for (size_t i = 0; i < proteinLength; ++i)
      dr->setElement(i, probability(line[i] == T('T') ? 1.f : 0.f));
    protein->setDisorderRegions(dr);

    setResult(protein);
    protein = ProteinPtr();
    proteinLength = (size_t)-1;

    return true;
  }
  
  virtual bool parseEnd()
  {
    return true;
  }
  
protected:
  ProteinPtr protein;
  size_t proteinLength;
};

class ConvertDisproFileToProteins : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputFile == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertDisproFileToProteins::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    outputDirectory.getChildFile(T("sup/")).createDirectory();
    outputDirectory.getChildFile(T("in/")).createDirectory();
    
    StreamPtr stream = StreamPtr(new DisproFileParser(context, inputFile));
    while (!stream->isExhausted())
    {
      ProteinPtr protein = stream->next().getObjectAndCast<Protein>();
      if (!protein)
        continue;
      ProteinPtr supervision = new Protein(protein->getName());
      supervision->setPrimaryStructure(protein->getPrimaryStructure());
      supervision->setDisorderRegions(protein->getDisorderRegions());

      ProteinPtr input = new Protein(protein->getName());
      input->setPrimaryStructure(protein->getPrimaryStructure());
      input->setSecondaryStructure(protein->getSecondaryStructure());
      input->setSolventAccessibilityAt20p(protein->getSolventAccessibilityAt20p());

      supervision->saveToFile(context, outputDirectory.getChildFile(T("sup/") + supervision->getName() + T(".xml")));
      input->saveToFile(context, outputDirectory.getChildFile(T("in/") + input->getName() + T(".xml")));
    }
    
    return true;
  }
  
protected:
  friend class ConvertDisproFileToProteinsClass;
  
  File inputFile;
  File outputDirectory;
};

/*
 * Input:
 *  >Name
 *  $ sn: prot_size: total_dr: num_dr: size_dr: #...
 *  Sequence AA
 *  Sequence disordered residue (+, -)
 * Ouput:
 *  Protein
 */
class SpineFileParser : public TextParser
{
public:
  SpineFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& srcLine)
  {
    String line = srcLine.trim();
    if (line == String::empty)
      return true;

    if (!protein)
    {
      if (!line.startsWithChar(T('>')))
      {
        context.errorCallback(T("SpineFileParser::parseLine"),
                              T("Invalid line. The line must start with '>' : ") +
                              line);
        return false;
      }

      int endIndex = line.indexOfChar(0, T(' '));
      String name = line.substring(1, endIndex == -1 ? line.length() : endIndex);
      if (name.startsWith(T("DisProt")))
        name = name.substring(8, 15);
      else
        name = name.removeCharacters(T(":"));
      std::cout << name << std::endl;
      protein = new Protein(name);
      return true;
    }

    if (line.startsWithChar(T('$')))
      return true;

    if (!protein->getPrimaryStructure())
    {
      protein->setPrimaryStructure(line);
      return true;
    }

    if ((size_t)line.length() != protein->getLength())
    {
      context.errorCallback(T("SpineFileParser::parseLine"),
                            T("Invalid DR sequence length: ") +
                            String(line.length()) + T("instead of ") +
                            String((int)protein->getLength()));
      return false;
    }

    const size_t n = protein->getLength();
    DenseDoubleVectorPtr dr = Protein::createEmptyProbabilitySequence(n);
    for (size_t i = 0; i < n; ++i)
      dr->setElement(i, probability(line[i] == T('+') ? 1.f : 0.f));
    protein->setDisorderRegions(dr);

    setResult(protein);
    protein = ProteinPtr();

    return true;
  }
  
  virtual bool parseEnd()
    {return true;}
  
protected:
  ProteinPtr protein;
};

class ConvertSpineFileToProteins : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputFile == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertSpineFileToProteins::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    outputDirectory.getChildFile(T("sup/")).createDirectory();
    
    StreamPtr stream = StreamPtr(new SpineFileParser(context, inputFile));
    while (!stream->isExhausted())
    {
      ProteinPtr protein = stream->next().getObjectAndCast<Protein>();
      if (!protein)
        continue;
      protein->saveToFile(context, outputDirectory.getChildFile(T("sup/") + protein->getName() + T(".xml")));
    }

    return true;
  }
  
protected:
  friend class ConvertSpineFileToProteinsClass;
  
  File inputFile;
  File outputDirectory;
};

};
