/*-----------------------------------------.---------------------------------.
| Filename: FileType.h                     | A utility function to determine |
| Author  : Francis Maes                   |  the type of a file             |
| Started : 17/06/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_UTILITIES_FILE_TYPE_H_
# define EXPLORER_UTILITIES_FILE_TYPE_H_

# include "../Components/common.h"

namespace lbcpp
{

enum FileType
{
  nonexistantFile,
  binaryFile,
  textFile,
  xmlFile,
  lbcppXmlFile,
  directory,
};

inline FileType getFileType(const File& file)
{
  if (!file.exists())
    return nonexistantFile;
  if (file.isDirectory())
    return directory;
  //if (file.getSize() > 100 * 1024 * 1024) // do not open files that do more than 100 Mb
  //  return binaryFile;

  InputStream* istr = file.createInputStream();
  if (!istr)
    return nonexistantFile;

  int indexOfZero = -1;
  String beforeZero;
  for (int i = 0; i < 256; ++i)
  {
    if (istr->isExhausted())
      break;
    char c = istr->readByte();
    if (c == 0)
    {
      indexOfZero = i;
      break;
    }
    else
      beforeZero += c;
  }
  delete istr;

  static const char* xmlBegin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  if (beforeZero.length() > (int)strlen(xmlBegin) && beforeZero.startsWith(String(xmlBegin)))
    return lbcppXmlFile;

  return indexOfZero < 0 ? binaryFile : textFile;
}

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_FILE_TYPE_H_
