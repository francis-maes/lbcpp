/*-----------------------------------------.---------------------------------.
| Filename: PDBFileGenerator.h             | PDB File Generator              |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_FORMATS_PDB_FILE_GENERATOR_H_
# define LBCPP_PROTEIN_FORMATS_PDB_FILE_GENERATOR_H_

# include "../Protein.h"
# include <lbcpp/Data/Consumer.h>

namespace lbcpp
{

class PDBFileGenerator : public TextPrinter
{
public:
  PDBFileGenerator(ExecutionContext& context, const File& file);

  virtual void consume(ExecutionContext& context, const Variable& variable);

  static String makeHeaderLine(const String& classification, const String& date, const String& idCode);
    
  static String producePDBString(ProteinPtr protein);

  static String makeSeqResLine(size_t serialNumber, const String& chainId, size_t numResidues,
                               const std::vector<String>& residues, size_t& firstResidueIndex);



  static String makeAtomLine(size_t atomNumber, const String& atomName, const String& residueName, const String& chainID,
                             size_t residueNumber, double x, double y, double z, double occupancy, double temperatureFactor,
                             const String& segmentIdentifier, const String& elementSymbol, const String& atomCharge);

  static String makeEndLine();

protected:
  static String toFixedLengthString(size_t i, int length);

  static String toFixedLengthString(const String& str, int length);
  static String toFixedLengthStringRightJustified(const String& str, int length);
  static String toFixedLengthStringLeftJustified(const String& str, int length);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FORMATS_PDB_FILE_GENERATOR_H_
