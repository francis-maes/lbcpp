/*-----------------------------------------.---------------------------------.
| Filename: Main.cpp                       | Main Test File                  |
| Author  : Francis Maes                   |                                 |
| Started : 27/03/2010 12:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "../ProteinInference/Protein.h"
#include "../ProteinInference/AminoAcidDictionary.h"
#include "../ProteinInference/SecondaryStructureDictionary.h"
using namespace lbcpp;

extern void declareProteinClasses();

#if 0
class DaFuckingDataParser : public LearningDataObjectParser
{
public:
  DaFuckingDataParser(const File& pssmFile, const File& sourceDirectory, FeatureDictionaryPtr features)
    : LearningDataObjectParser(pssmFile, features), sourceDirectory(sourceDirectory) {}
    
  virtual void parseBegin()
  {
    currentContent.clear();
    currentLabels.clear();
  }

  virtual bool parseCommentLine(const String& comment)
  {
    this->comment = comment.endsWith(T(".all")) ? comment.dropLastCharacters(4) : comment;
    return true;
  }

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    jassert(columns.size());
    String label = columns[0];
    SparseVectorPtr features;
    if (!parseFeatureList(columns, 1, features))
      return false;
    currentContent.push_back(features);
    currentLabels.push_back(label);
    return true;
  }

  bool parseSourceFile(const File& sourceFile, String& aminoAcidsSequence, String& dsspSequence, String& solventAccesibilitySequence) const
  {
    jassert(sourceFile.existsAsFile());
    InputStream* sourceFileStream = sourceFile.createInputStream();
    jassert(sourceFileStream);
    
    while (!sourceFileStream->isExhausted())
    {
      String line = sourceFileStream->readNextLine();
      StringArray mainTokens;
      mainTokens.addTokens(line, T(":"), NULL);
      jassert(mainTokens.size() == 2);
      String category = mainTokens[0];
      
      StringArray subTokens;
      subTokens.addTokens(mainTokens[1], T(","), NULL);
      jassert(subTokens.size());

      String content = subTokens.joinIntoString(String::empty);

      if (category == T("RES"))
        aminoAcidsSequence = content;
      else if (category == T("DSSP"))
        dsspSequence = content;
      else if (category == T("DSSPACC"))
        solventAccesibilitySequence = content;
    }
    jassert(aminoAcidsSequence.isNotEmpty() && dsspSequence.isNotEmpty() && solventAccesibilitySequence.isNotEmpty());
    delete sourceFileStream;
    return true;
  }

  ProteinPtr makeProtein() const
  {
    size_t length = currentContent.size();
    jassert(length == currentLabels.size() && length);
    ProteinPtr res = new Protein(comment);

    // parse source file
    File sourceFile = sourceDirectory.getChildFile(comment + ".all");
    String aminoAcidsSequence, dsspSequence, solventAccesibilitySequence;
    bool ok = parseSourceFile(sourceFile, aminoAcidsSequence, dsspSequence, solventAccesibilitySequence);
    jassert(ok);
    jassert(aminoAcidsSequence.length() == (int)length);
    jassert(dsspSequence.length() == (int)length);
    jassert(solventAccesibilitySequence.length() == (int)length);

    FeatureDictionaryPtr aminoAcidDictionary = AminoAcidDictionary::getInstance();
    // amino acids
    LabelSequencePtr aminoAcids = new LabelSequence(aminoAcidDictionary, length);
    res->setAminoAcidSequence(aminoAcids);
    
    // pssm
    ScoreVectorSequencePtr pssm = new ScoreVectorSequence(T("PositionSpecificScoringMatrix"), aminoAcidDictionary, length);
    res->setPositionSpecificScoringMatrix(pssm);

    // secondary structure (three states)
    LabelSequencePtr secondaryStructure = new LabelSequence(SecondaryStructureDictionary::getInstance(), length);
    res->setSecondaryStructureSequence(secondaryStructure);

    // DSSP secondary structure (height states)
    LabelSequencePtr dsspSecondaryStructure = new LabelSequence(DSSPSecondaryStructureDictionary::getInstance(), length);
    res->setDSSPSecondaryStructureSequence(dsspSecondaryStructure);

    // solvent accesibility
    LabelSequencePtr solventAccessibility = new LabelSequence(SolventAccesibility2StateDictionary::getInstance(), length);
    res->setSolventAccessibilitySequence(solventAccessibility);

    for (size_t i = 0; i < length; ++i)
    {
      SparseVectorPtr vector = currentContent[i].dynamicCast<SparseVector>();
      jassert(vector);

      // amino acid
      SparseVectorPtr aminoAcid = vector->getSubVector(T("AA"));
      jassert(aminoAcid && aminoAcid->getValues().size() == 1);
      String oneLetterCode = aminoAcid->getDictionary()->getFeatures()->getString(aminoAcid->getValues()[0].first);
      jassert(oneLetterCode.length() == 1);
      jassert(aminoAcidsSequence[i] == oneLetterCode[0]);
      aminoAcids->setString(i, oneLetterCode);

      // pssm
      SparseVectorPtr pssmVector = vector->getSubVector(T("pssm"));
      jassert(pssmVector && pssmVector->hasValues());
      for (size_t j = 0; j < pssmVector->getNumValues(); ++j)
        pssm->setScore(i, pssmVector->getValueNameByPosition(j), pssmVector->getValueByPosition(j));

      // solvent accesibility
      juce::tchar sa = solventAccesibilitySequence[i];
      jassert(sa == 'e' || sa == 'b');
      solventAccessibility->setString(i, sa == 'e' ? T("E") : T("B"));

      // dssp
      String dsspElement; dsspElement += dsspSequence[i];
      dsspSecondaryStructure->setString(i, dsspElement);
      
      SecondaryStructureDictionary::Type threeStatesType = SecondaryStructureDictionary::getIndexFromDSSPElement(dsspElement);
      secondaryStructure->setIndex(i, threeStatesType);

      // secondary structure
      String checkLabel = currentLabels[i];
      jassert(checkLabel == T("C") || checkLabel == T("H") || checkLabel == T("E"));
      jassert(checkLabel == SecondaryStructureDictionary::getInstance()->getFeature(threeStatesType));
    }
    return res;
  }

  virtual bool parseEmptyLine()
  {
    if (currentContent.size())
    {
      setResult(makeProtein());
      currentContent.clear();
      currentLabels.clear();
    }
    return true;
  }
  
private:
  File sourceDirectory;
  String comment;
  std::vector<FeatureGeneratorPtr> currentContent;
  std::vector<String> currentLabels;
};

int main()
{
  declareProteinClasses();

  File sourceDirectory("C:\\Projets\\Proteins\\data\\CB513");
  File pssmDirectory("C:\\Projets\\Proteins\\scripts");
  File outputDirectory("C:\\Projets\\Proteins\\data\\CB513new");
  
  juce::OwnedArray<File> pssmFiles;
  pssmDirectory.findChildFiles(pssmFiles, File::findFiles, false, T("*.data"));
  std::cout << pssmFiles.size() << " pssm files found." << std::endl;

  int proteinNumber = 1;
  FeatureDictionaryPtr features = new FeatureDictionary("pssmFeatures");
  for (int i = 0; i < pssmFiles.size(); ++i)
  {
    File pssmFile = *pssmFiles[i];
    std::cout << "File: " << pssmFile.getFullPathName() << std::endl;
    ObjectStreamPtr parser = new DaFuckingDataParser(pssmFile, sourceDirectory, features);
    while (!parser->isExhausted())
    {
      ProteinPtr protein = parser->nextAndCast<Protein>();
      if (!protein)
        break;
      std::cout << "Protein " << proteinNumber++ << ": " << protein->getName() << std::endl;
      File outputFile = outputDirectory.getChildFile(protein->getName() + T(".protein"));
      protein->saveToFile(outputFile);
      /*std::cout << "PROT: " << protein->toString() << std::endl;
      ProteinPtr protein2 = Object::createFromFileAndCast<Protein>(outputFile);
      std::cout << "PROT2: " << protein2->toString() << std::endl;
      return 0;*/
    }
  }
  return 0;
}
#endif // 0


int main()
{
  declareProteinClasses();

  File inputDirectory(T("C:\\Projets\\LBC++\\projects\\Proteins\\CB513"));
  File outputDirectory(T("C:\\Projets\\LBC++\\projects\\Proteins\\CB513New"));
  if (!outputDirectory.exists())
    outputDirectory.createDirectory();
  ObjectStreamPtr proteinsStream = directoryObjectStream(inputDirectory, T("*.protein"));
  objectSaveToFileConsumer(outputDirectory, T("protein"))->consumeStream(proteinsStream);
  return 0;
}