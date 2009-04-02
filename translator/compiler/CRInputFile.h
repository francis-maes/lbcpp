/*-----------------------------------------.---------------------------------.
| Filename: CRInputFile.h                  | CR-Algorithm input file         |
| Author  : Francis Maes                   |                                 |
| Started : 10/02/2009 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_COMPILER_CR_ALGO_FILE_H_
# define CRALGORITHM_COMPILER_CR_ALGO_FILE_H_

# include "../common.h"
# include "../tools/PreprocessorAndCommentsChunker.h"

class CRInputFile
{
public:
  CRInputFile(const std::string& filename);
  ~CRInputFile();
  
  bool parse(bool verbose);
  bool checkSyntax();
  
  void writeIntermediaryRepresentations(const std::string& basename);
  
  PTree::Node* getRootNode() const
    {return rootNode;}
    
  void prettyPrint(PTree::Node* node, std::ostream& target, bool addSourceLineNumbers = true);
  
  unsigned long getFirstLineOfCode() const;
  unsigned long getLastLineOfCode() const;
    
  SymbolFactory& getSymbols()
    {return symbols;}
    
  CodeChunkVector& getChunks() 
    {return chunks;}
  
  void addChunk(size_t lineNumber, const std::string& content);

private:
  std::string filename;

  std::istream* openFile() const;
  
  CodeChunkVector chunks;
  Buffer*       buffer;
  PTree::Node*  rootNode;
  SymbolFactory symbols;
};

#endif // !CRALGORITHM_COMPILER_CR_ALGO_FILE_H_
