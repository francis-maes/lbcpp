#include <lbcpp/lbcpp.h>

namespace lbcpp
{

/*
 * Input:
 *  line 1 - AA sequence
 *  line 2 - DR sequence (0, 1, ?)
 * Ouput:
 *  Protein
 */
class FabienFileParser : public TextParser
{
public:
  FabienFileParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), fileName(file.getFileNameWithoutExtension()) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}
  
  virtual void parseBegin() {}
  
  virtual bool parseLine(const String& line)
  {
    if (line.startsWithChar(T('>')))
      return true;

    String trimmedLine = line.trim();
    if (!protein)
    {
      protein = new Protein(fileName);
      protein->setPrimaryStructure(trimmedLine);
      return true;
    }
    
    if (trimmedLine.length() != (int)protein->getLength())
    {
      context.errorCallback(T("FabienFileParser::parseLine"), T("Lengths of AA and DR are different !"));
      return false;
    }
    
    DoubleVectorPtr disorderedRegions = Protein::createEmptyProbabilitySequence(protein->getLength());
    for (size_t i = 0; i < (size_t)trimmedLine.length(); ++i)
    {
      if (trimmedLine[(int)i] == T('0'))
        disorderedRegions->setElement(i, 0.0);
      else if (trimmedLine[(int)i] == T('1'))
        disorderedRegions->setElement(i, 1.0);
      else if (trimmedLine[(int)i] == T('?'))
        disorderedRegions->setElement(i, disorderedRegions->missingElement());
      else
      {
        context.errorCallback(T("FabienFileParser::parseLine"), T("Unknown DR symbol: ") + trimmedLine[(int)i]);
        return false;
      }
    }
    protein->setDisorderRegions(disorderedRegions);

    return true;
  }
  
  virtual bool parseEnd()
  {
    if (!protein)
    {
      context.errorCallback(T("FabienFileParser::parseEnd"), T("No protein parsed"));
      return false;
    }
    setResult(protein);
    return true;
  }
  
protected:
  String fileName;
  ProteinPtr protein;
};

class ConvertFabienFileToProtein : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent || outputDirectory == File::nonexistent)
    {
      context.errorCallback(T("ConvertFabienFileToProtein::run"), T("At least one of argument is wrong !"));
      return false;
    }
    
    juce::OwnedArray<File> files;
    inputDirectory.findChildFiles(files, File::findFiles, false, T("*.fastadr"));
    size_t correctlyConvected = 0;
    for (size_t i = 0; i < (size_t)files.size(); ++i)
    {
      ProteinPtr protein = StreamPtr(new FabienFileParser(context, *files[i]))->next().getObjectAndCast<Protein>();
      if (!protein)
      {
        context.errorCallback(T("ConvertFabienFileToProtein::run"), T("No protein parsed in file: ") + files[i]->getFullPathName());
        continue;
      }
      protein->saveToFile(context, outputDirectory.getChildFile(protein->getName() + T(".xml")));
      ++correctlyConvected;
    }

    context.informationCallback(T("Parsed protein: ") + String((int)correctlyConvected) + T("/") + String(files.size()));
    return true;
  }
  
protected:
  friend class ConvertFabienFileToProteinClass;
  
  File inputDirectory;
  File outputDirectory;
};

};
