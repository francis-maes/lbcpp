/*-----------------------------------------.---------------------------------.
| Filename: PDBFileParser.h                | PDB File Parser                 |
| Author  : Francis Maes                   |                                 |
| Started : 18/04/2010 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_FORMATS_PDB_FILE_PARSER_H_
# define LBCPP_PROTEIN_FORMATS_PDB_FILE_PARSER_H_

# include "../Protein.h"

namespace lbcpp
{

class PDBFileParser : public TextParser
{
public:
  PDBFileParser(const File& file, bool beTolerant);
  
  virtual TypePtr getElementsType() const
    {return proteinClass();}

  virtual void parseBegin();
  virtual bool parseLine(const String& line);
  virtual bool parseEnd();

  std::vector<ProteinPtr> getAllChains() const;

protected:
  bool beTolerant;

  struct Chain
  {
    ProteinPtr protein;
    std::vector< std::vector<ResiduePtr> > tertiaryStructureBlocks;
  };

  String proteinName;
  String experimentData;
  typedef std::map<char, Chain> ChainMap;
  ChainMap chains; // by chain ID
  std::set<char> skippedChains;

  int currentSeqResSerialNumber;
  int currentModelSerialNumber;
  int currentAtomSerialNumber;
  int currentResidueSerialNumber;
  char currentResidueInsertionCode;

  size_t currentResidueIndex;

  bool parseHeaderLine(const String& line);
  bool parseExpDataLine(const String& line);
  bool parseRemarkLine(const String& line);

  bool parseSeqResLine(const String& line);

  bool parseModelLine(const String& line);
  bool parseAtomLine(const String& line);
  bool parseTerLine(const String& line);
  bool parseHetAtomLine(const String& line);

  static String getSubString(const String& line, int firstColumn, int lastColumn);
  static bool getChar(const String& line, int column, char& result, ErrorHandler& callback);
  static bool getInteger(const String& line, int firstColumn, int lastColumn, int& result, ErrorHandler& callback);
  static bool getDouble(const String& line, int firstColumn, int lastColumn, double& result, ErrorHandler& callback);

  bool getChainId(const String& line, int column, char& res) const;
  Chain* getChain(const String& line, int column);
  bool parseAndCheckAtomSerialNumber(const String& line, int firstColumn, int lastColumn);

  TertiaryStructurePtr finalizeChain(char chainId, ProteinPtr protein, const std::vector< std::vector<ResiduePtr> >& tertiaryStructureBlocks);
  VectorPtr finalizeDisorderSequence(ProteinPtr protein);

  bool checkResidueConsistency(ResiduePtr residue);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FORMATS_PDB_FILE_PARSER_H_
