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
  String proteinName;
  typedef std::map<char, ProteinPtr> ProteinMap;
  ProteinMap proteins; // by chain ID

  int currentSeqResSerialNumber;
  int currentModelSerialNumber;
  int currentAtomSerialNumber;
  int currentResidueSerialNumber;
  char currentResidueInsertionCode;

  size_t currentResidueIndex;

  bool parseHeaderLine(const String& line);

  bool parseSeqResLine(const String& line);

  bool parseModelLine(const String& line);
  bool parseAtomLine(const String& line);
  bool parseTerLine(const String& line);
  bool parseHetAtomLine(const String& line);

  static String getSubString(const String& line, int firstColumn, int lastColumn);
  static bool getChar(const String& line, int column, char& result);
  static bool getInteger(const String& line, int firstColumn, int lastColumn, int& result);
  static bool getDouble(const String& line, int firstColumn, int lastColumn, double& result);

  bool getChainId(const String& line, int column, char& res) const;
  ProteinPtr getProteinFromChainId(const String& line, int column);
  bool parseAndCheckAtomSerialNumber(const String& line, int firstColumn, int lastColumn);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PDB_FILE_PARSER_H_
