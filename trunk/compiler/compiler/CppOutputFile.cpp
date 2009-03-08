/*-----------------------------------------.---------------------------------.
| Filename: CppOutputFile.cpp              | C++ output file                 |
| Author  : Francis Maes                   |                                 |
| Started : 11/02/2009 16:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CppOutputFile.h"
#include "CRInputFile.h"
#include "../tools/ErrorManager.h"

CppOutputFile::CppOutputFile(const std::string& filename) : filename(filename)
{
}

static std::string createDblInclusionMacroName(const std::string& filename)
{
  std::string res;
  for (size_t i = 0; i < filename.size(); ++i)
  {
    char c = filename[i];
    if (c >= 'A' && c <= 'Z')
      res += c;
    else if (c >= 'a' && c <= 'z')
      res += c + 'A' - 'a';
    else if (c >= '0' && c <= '9')
      res += i ? c : '_';
    else if (c == '+')
      res += "p";
    else
      res += "_";
  }
  return res += "_";
}

bool CppOutputFile::write(PTree::Node* node, CRInputFile& sourceFile, bool addSourceLineNumbers)
{
  std::ostream* ostr = openFile();
  if (!ostr)
    return false;
  std::string dblInclusionMacroName = createDblInclusionMacroName(filename);
  std::string preprocessorBegin =
    "#ifndef " + dblInclusionMacroName + "\n" +
    "# define " + dblInclusionMacroName + "\n" +
    "# include <cralgo/cralgo.h>\n" +
    "# include <cralgo/impl/impl.h>\n\n";
  sourceFile.addChunk(sourceFile.getFirstLineOfCode() - 1, preprocessorBegin);
  sourceFile.prettyPrint(node, *ostr, addSourceLineNumbers);
  (*ostr) << "\n#endif // !" + dblInclusionMacroName << std::endl;
  delete ostr;
  return true;
}

std::ostream* CppOutputFile::openFile() const
{
  std::ofstream* res = new std::ofstream(filename.c_str());
  if (!res->is_open())
  {
    ErrorManager::getInstance().addError("Could not open file", filename);
    delete res;
    return NULL;
  }
  return res;
}
