/*-----------------------------------------.---------------------------------.
| Filename: DirectoryPairFileStream.h      | Stream of pair of files coming  |
| Author  : Francis Maes                   |  from two aligned directories   |
| Started : 18/10/2010 10:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_STREAM_DIRECTORY_PAIR_FILES_H_
# define LBCPP_DATA_STREAM_DIRECTORY_PAIR_FILES_H_

# include "DirectoryFileStream.h"

namespace lbcpp
{

class DirectoryPairFileStream : public Stream
{
public:
  DirectoryPairFileStream(const File& mainDirectory, const File& secondDirectory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false)
    : mainDirectory(mainDirectory), secondDirectory(secondDirectory), wildCardPattern(wildCardPattern), searchFilesRecursively(searchFilesRecursively)
    {initialize();}

  DirectoryPairFileStream() : nextFilePosition(0) {}

  virtual TypePtr getElementsType() const
    {return pairClass(fileType, fileType);}

  virtual bool rewind()
    {nextFilePosition = 0; return true;}

  virtual bool isExhausted() const
    {return nextFilePosition >= (int)files.size();}

  virtual Variable next()
  {
    if (isExhausted())
      return Variable();
    jassert(nextFilePosition < (int)files.size());
    std::pair<File, File> res = files[nextFilePosition];
    ++nextFilePosition;
    return Variable::pair(res.first, res.second);
  }

private:
  File mainDirectory;
  File secondDirectory;
  String wildCardPattern;
  bool searchFilesRecursively;

  std::vector< std::pair<File, File> > files;
  int nextFilePosition;

  void initialize()
  {
    std::set<String> mainFiles;
    DirectoryFileStream::findChildFiles(mainDirectory, wildCardPattern, searchFilesRecursively, mainFiles);

    files.reserve(mainFiles.size());
    for (std::set<String>::const_iterator it = mainFiles.begin(); it != mainFiles.end(); ++it)
    {
      File file1(*it);
      File file2 = secondDirectory.getChildFile(file1.getRelativePathFrom(mainDirectory));
      if (file2.existsAsFile())
        files.push_back(std::make_pair(file1, file2));
    }

    nextFilePosition = 0;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_DIRECTORY_PAIR_FILES_H_
