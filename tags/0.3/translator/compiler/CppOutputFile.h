/*-----------------------------------------.---------------------------------.
| Filename: CppOutputFile.h                | C++ output file                 |
| Author  : Francis Maes                   |                                 |
| Started : 11/02/2009 16:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_COMPILER_CPP_OUTPUT_FILE_H_
# define CRALGORITHM_COMPILER_CPP_OUTPUT_FILE_H_

# include "../common.h"

class CRInputFile;

class CppOutputFile
{
public:
  CppOutputFile(const std::string& filename);
  
  bool write(PTree::Node* node, CRInputFile& sourceFile, bool addSourceLineNumbers = true);
  
private:
  std::ostream* openFile() const;

  std::string filename;
};

#endif // !CRALGORITHM_COMPILER_CPP_OUTPUT_FILE_H_
