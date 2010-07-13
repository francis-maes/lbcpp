/*-----------------------------------------.---------------------------------.
| Filename: FASTAFileParser.h              | FASTA File parser               |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_FORMATS_FASTA_FILE_PARSER_H_
# define LBCPP_PROTEINS_FORMATS_FASTA_FILE_PARSER_H_

# include "../Protein.h"
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class FASTAFileParser : public TextParser
{
public:
  FASTAFileParser(const File& file)
    : TextParser(file) {}
  
  virtual TypePtr getElementsType() const
    {return proteinClass();}
  
  virtual void parseBegin()
    {}

  virtual bool parseLine(const String& line)
  {
    String str = line.trim();
    if (str.isEmpty())
      return true;
    if (str[0] == '>')
    {
      flush();
      currentName = str.substring(1);
    }
    else
      currentAminoAcidSequence += str;
    return true;
  }
  
  virtual bool parseEnd()
  {
    flush();
    return true;
  }

private:
  String currentName;
  String currentAminoAcidSequence;
  
  void flush()
  {
    if (currentName.isNotEmpty() && currentAminoAcidSequence.isNotEmpty())
    {
      ProteinPtr res = new Protein(currentName);
      res->setPrimaryStructure(currentAminoAcidSequence);
      setResult(res);
    }
    currentName = String::empty;
    currentAminoAcidSequence = String::empty;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_FORMATS_FASTA_FILE_PARSER_H_
