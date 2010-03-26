#include <lbcpp/lbcpp.h>
#include "GeneratedCode/Data/Bio/AminoAcidSequence.lh"
#include "GeneratedCode/Data/Bio/PositionSpecificScoringMatrix.lh"
#include "GeneratedCode/Data/Bio/SecondaryStructureSequence.lh"
#include "GeneratedCode/Data/Bio/SolventAccesibilitySequence.lh"
using namespace lbcpp;

class StringToObjectMap : public Object
{
public:
  void setObject(const String& key, ObjectPtr object)
    {objects[key] = object;}
  
  virtual String toString() const
  {
    if (objects.empty())
      return T("No objects");
    else if (objects.size() == 1)
      return T("Object ") + objects.begin()->first + T(":\n\t") + objects.begin()->second->toString();
    else
    {
      String res = String((int)objects.size()) + T(" objects:\n");
      for (ObjectsMap::const_iterator it = objects.begin(); it != objects.end(); ++it)
        res += T("Object ") + it->first + T(":\n\t") + lbcpp::toString(it->second) + T("\n");
      return res;
    }
  }

protected:
  virtual bool load(InputStream& istr)
    {return lbcpp::read(istr, objects);}

  virtual void save(OutputStream& ostr) const
    {lbcpp::write(ostr, objects);}

private:
  typedef std::map<String, ObjectPtr> ObjectsMap;
  ObjectsMap objects;
};

class Protein : public StringToObjectMap
{
public:
  Protein(const String& name)
    : name(name) {}
  Protein() {}

  virtual String toString() const
    {return T("Protein ") + name + T(":\n") + StringToObjectMap::toString();}

  virtual String getName() const
    {return name;}

  void setAminoAcidSequence(AminoAcidSequencePtr sequence)
    {setObject(T("AminoAcidSequence"), sequence);}

  void setPositionSpecificScoringMatrix(PositionSpecificScoringMatrixPtr pssm)
    {setObject(T("PositionSpecificScoringMatrix"), pssm);}

  void setSolventAccessibilitySequence(SolventAccessibilitySequencePtr solventAccessibility)
    {setObject(T("SolventAccessibilitySequence"), solventAccessibility);}

protected:
  virtual bool load(InputStream& istr)
    {return lbcpp::read(istr, name) && StringToObjectMap::load(istr);}

  virtual void save(OutputStream& ostr) const
    {lbcpp::write(ostr, name); StringToObjectMap::save(ostr);}

private:
  String name;
};

typedef ReferenceCountedObjectPtr<Protein> ProteinPtr;

// Data: 

// AminoAcid,
// LabelSequence: AminoAcidSequence
// VectorSequence: PositionSpecificScoringMatrix
// LabelSequence: 3 state and 8 state SecondaryStructureSequence
// LabelSequence: 2 state SolventAccesibilitySequence
// ScoreSequence: regression SolventAccesibilitySequence
// ScalarMatrix: 2 state ResidueContactMap
// ScalarMatrix: regression ResidueDistanceMap
// VectorSequence: BackboneSequence
// Object: ThirdaryStructure

void declareProteinClasses()
{
  LBCPP_DECLARE_CLASS(Protein);

  LBCPP_DECLARE_CLASS(AminoAcidSequence);
  LBCPP_DECLARE_CLASS(PositionSpecificScoringMatrix);
  LBCPP_DECLARE_CLASS(SecondaryStructureSequence);
  LBCPP_DECLARE_CLASS(SolventAccessibilitySequence);
}

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
    jassert(currentContent.size() == currentLabels.size() && currentContent.size());
    ProteinPtr res = new Protein(comment);

    // parse source file
    File sourceFile = sourceDirectory.getChildFile(comment + ".all");
    String aminoAcidsSequence, dsspSequence, solventAccesibilitySequence;
    bool ok = parseSourceFile(sourceFile, aminoAcidsSequence, dsspSequence, solventAccesibilitySequence);
    jassert(ok);
    jassert(aminoAcidsSequence.length() == currentContent.size());
    jassert(dsspSequence.length() == currentContent.size());
    jassert(solventAccesibilitySequence.length() == currentContent.size());

    // amino acids
    AminoAcidSequencePtr aminoAcids = new AminoAcidSequence();
    aminoAcids->setLength(currentContent.size());
    res->setAminoAcidSequence(aminoAcids);
    
    // pssm
    PositionSpecificScoringMatrixPtr pssm = new PositionSpecificScoringMatrix();
    pssm->setLength(currentContent.size());
    res->setPositionSpecificScoringMatrix(pssm);

    // three state secondary structure
    SecondaryStructureSequencePtr threeStateSecondary = new SecondaryStructureSequence(false);
    threeStateSecondary->setLength(currentContent.size());
    res->setObject("Three State Secondary Structure", threeStateSecondary);

    // eight state secondary structure
    SecondaryStructureSequencePtr eightStateSecondary = new SecondaryStructureSequence(true);
    eightStateSecondary->setLength(currentContent.size());
    res->setObject("Eight State Secondary Structure", eightStateSecondary);

    // solvent accesibility
    SolventAccessibilitySequencePtr solventAccessibility = new SolventAccessibilitySequence();
    solventAccessibility->setLength(currentContent.size());
    res->setSolventAccessibilitySequence(solventAccessibility);

    for (size_t i = 0; i < currentContent.size(); ++i)
    {
      SparseVectorPtr vector = currentContent[i].dynamicCast<SparseVector>();
      jassert(vector);

      // amino acid
      SparseVectorPtr aminoAcid = vector->getSubVector(T("AA"));
      jassert(aminoAcid && aminoAcid->getValues().size() == 1);
      String oneLetterCode = aminoAcid->getDictionary()->getFeatures()->getString(aminoAcid->getValues()[0].first);
      jassert(oneLetterCode.length() == 1);
      jassert(aminoAcidsSequence[i] == oneLetterCode[0]);
      aminoAcids->setAminoAcid(i, oneLetterCode);

      // pssm
      SparseVectorPtr pssmVector = vector->getSubVector(T("pssm"));
      jassert(pssmVector && pssmVector->hasValues());
      for (size_t j = 0; j < pssmVector->getNumValues(); ++j)
        pssm->setScore(i, AminoAcid(pssmVector->getValueNameByPosition(j)), pssmVector->getValueByPosition(j));

      // solvent accesibility
      juce::tchar sa = solventAccesibilitySequence[i];
      jassert(sa == 'e' || sa == 'b');
      solventAccessibility->setExposedProbability(i, sa == 'e' ? 1.0 : 0.0);

      // dssp
      juce::tchar dssp = dsspSequence[i];
      SecondaryStructureElement dsspElement(dssp, true);
      threeStateSecondary->setElement(i, dsspElement.toThreeState());
      eightStateSecondary->setElement(i, dsspElement);

      // secondary structure
      String checkLabel = currentLabels[i];
      jassert(checkLabel == T("C") || checkLabel == T("H") || checkLabel == T("E"));
      jassert(checkLabel[0] == dsspElement.toThreeState().getOneLetterCode());
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

bool formDaFuckingCB513base()
{
  File sourceDirectory("C:\\Projets\\Proteins\\data\\CB513");
  File pssmDirectory("C:\\Projets\\Proteins\\scripts");
  File outputDirectory("C:\\Projets\\Proteins\\data\\CB513cool");
  
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
    while (parser->isValid())
    {
      ProteinPtr protein = parser->nextAndCast<Protein>();
      if (!protein)
        break;
      std::cout << "Protein " << proteinNumber++ << ": " << protein->getName() << std::endl;
      File outputFile = outputDirectory.getChildFile(protein->getName() + T(".protein"));
      protein->saveToFile(outputFile);
/*
      std::cout << "PROT: " << protein->toString() << std::endl;
      ProteinPtr protein2 = Object::loadFromFileAndCast<Protein>(outputFile);
      std::cout << "PROT2: " << protein2->toString() << std::endl;*/
    }
  }
  return true;
}

int main()
{
  declareProteinClasses();
  return formDaFuckingCB513base() ? 0 : 1;
}
