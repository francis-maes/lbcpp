/*-----------------------------------------.---------------------------------.
| Filename: PDBFileParser.h                | PDB File Parser                 |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PDB_FILE_PARSER_H_
# define LBCPP_PROTEIN_PDB_FILE_PARSER_H_

# include "Protein.h"

namespace lbcpp
{

class PDBFileParser : public TextObjectParser
{
public:
  PDBFileParser(const File& file);
  
  virtual void parseBegin();
  virtual bool parseLine(const String& line);
  virtual bool parseEnd();

protected:
  ProteinPtr protein;
  int currentSeqResSerialNumber;
  int currentAtomSerialNumber;

  bool parseHeaderLine(const String& line);
  bool parseSeqResLine(const String& line);
  bool parseAtomLine(const String& line);

  static String getSubString(const String& line, int firstColumn, int lastColumn);
  static bool getInteger(const String& line, int firstColumn, int lastColumn, int& result);
  static bool getDouble(const String& line, int firstColumn, int lastColumn, double& result);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PDB_FILE_PARSER_H_
