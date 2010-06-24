/*-----------------------------------------.---------------------------------.
| Filename: TertiaryStructureCASPFileGe...h| CASP Prediction Files generator |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 17:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_TERTIARY_STRUCTURE_CASP_FILE_GENERATOR_H_
# define LBCPP_PROTEINS_FORMATS_TERTIARY_STRUCTURE_CASP_FILE_GENERATOR_H_

# include "CASPFileGenerator.h"

namespace lbcpp
{

class TertiaryStructureCASPFileGenerator : public CASPFileGenerator
{
public:
  TertiaryStructureCASPFileGenerator(const File& file, const String& method)
    : CASPFileGenerator(file, method) {}

  virtual String getFormatSpecificationCode() const
    {return T("TS");}

  virtual void printPredictionData(ProteinPtr protein)
  {
    printRecord(T("PARENT"), T("N/A"));

    ProteinTertiaryStructurePtr tertiaryStructure = protein->getTertiaryStructure();
    jassert(tertiaryStructure);
    size_t atomNumber = 1;
    for (size_t i = 0; i < tertiaryStructure->size(); ++i)
    {
      ProteinResidueAtomsPtr residue = tertiaryStructure->getResidue(i);
      for (size_t j = 0; j < residue->getNumAtoms(); ++j)
        printAtom(residue, residue->getAtom(j), i + 1, atomNumber++);
    }

    print(T("TER"), true);
  }


  void printAtom(ProteinResidueAtomsPtr residue, ProteinAtomPtr atom, size_t residueNumber, size_t atomNumber)
  {
    String line = PDBFileGenerator::makeAtomLine(atomNumber, atom->getName(), residue->getName(),
                  String::empty, residueNumber, atom->getX(), atom->getY(), atom->getZ(),
                      1.0, -1.0, String::empty, String::empty, String::empty);
    print(line, true);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_TERTIARY_STRUCTURE_CASP_FILE_GENERATOR_H_
