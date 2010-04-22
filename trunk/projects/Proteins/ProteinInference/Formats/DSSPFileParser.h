/*-----------------------------------------.---------------------------------.
| Filename: DSSPFileParser.cpp             | DSSP Parser                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 19:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
# define LBCPP_PROTEIN_DSSP_FILE_PARSER_H_

# include "../Protein.h"
# include "../SecondaryStructureDictionary.h"

namespace lbcpp
{

class DSSPFileParser : public TextObjectParser
{
public:
  DSSPFileParser(const File& file, ProteinPtr protein)
    : TextObjectParser(file), protein(protein)
  {
    std::cout << "AA: " << protein->getAminoAcidSequence()->toString() << std::endl;
  }

  virtual void parseBegin()
  {
    serialNumber = 0;
  }

  virtual bool parseLine(const String& line)
  {
    if (line.isEmpty())
      return true; // skip empty lines

    size_t n = protein->getLength();

    if (serialNumber == 0)
    {
      if (line.startsWith(T(" #  RESIDUE AA STRUCTURE BP1 BP2  ACC")))
      {
        dsspSecondaryStructureSequence = new LabelSequence(T("DSSPSecondaryStructureSequence"), DSSPSecondaryStructureDictionary::getInstance(), n);
        solventAccesibilitySequence = new LabelSequence(T("SolventAccessibilitySequence"), SolventAccesibility2StateDictionary::getInstance(), n);
        ++serialNumber;
      }
      return true;
    }
    
    // FIXME
    return true;
  }

  virtual bool parseEnd()
  {
    if (!serialNumber)
    {
      Object::error(T("DSSPFileParser::parseEnd"), T("No residues in dssp file"));
      return false;
    }

    setResult(dsspSecondaryStructureSequence);
    protein->setObject(dsspSecondaryStructureSequence);
    // todo: convert to three states
    return true;
  }
  
protected:
  ProteinPtr protein;
  LabelSequencePtr dsspSecondaryStructureSequence;
  LabelSequencePtr solventAccesibilitySequence;
  int serialNumber;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
