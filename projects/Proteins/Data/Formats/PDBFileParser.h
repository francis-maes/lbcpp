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
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class PDBFileParser : public TextParser
{
public:
  PDBFileParser(ExecutionContext& context, const File& file, bool beTolerant);
  
  virtual TypePtr getElementsType() const
    {return proteinClass;}

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

  bool parseHeaderLine(ExecutionContext& context, const String& line);
  bool parseExpDataLine(ExecutionContext& context, const String& line);
  bool parseRemarkLine(ExecutionContext& context, const String& line);

  bool parseSeqResLine(ExecutionContext& context, const String& line);

  bool parseModelLine(ExecutionContext& context, const String& line);
  bool parseAtomLine(ExecutionContext& context, const String& line);
  bool parseTerLine(ExecutionContext& context, const String& line);
  bool parseHetAtomLine(ExecutionContext& context, const String& line);

  static String getSubString(const String& line, int firstColumn, int lastColumn);
  static bool getChar(ExecutionContext& context, const String& line, int column, char& result);
  static bool getInteger(ExecutionContext& context, const String& line, int firstColumn, int lastColumn, int& result);
  static bool getDouble(ExecutionContext& context, const String& line, int firstColumn, int lastColumn, double& result);

  bool getChainId(ExecutionContext& context, const String& line, int column, char& res) const;
  Chain* getChain(ExecutionContext& context, const String& line, int column);
  bool parseAndCheckAtomSerialNumber(ExecutionContext& context, const String& line, int firstColumn, int lastColumn);

  TertiaryStructurePtr finalizeChain(ExecutionContext& context, char chainId, ProteinPtr protein, const std::vector< std::vector<ResiduePtr> >& tertiaryStructureBlocks);
  //VectorPtr finalizeDisorderSequence(ProteinPtr protein);

  bool checkResidueConsistency(ExecutionContext& context, ResiduePtr residue);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FORMATS_PDB_FILE_PARSER_H_
