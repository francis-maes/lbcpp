/*-----------------------------------------.---------------------------------.
| Filename: DirectoryFileStream.h          | Stream of directory child files |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 12:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_STREAM_DIRECTORY_FILES_H_
# define LBCPP_DATA_STREAM_DIRECTORY_FILES_H_

# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class DirectoryFileStream : public Stream
{
public:
  DirectoryFileStream(const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false)
    : directory(directory), wildCardPattern(wildCardPattern), searchFilesRecursively(searchFilesRecursively)
    {initialize();}

  DirectoryFileStream() : nextFilePosition(0) {}

  virtual TypePtr getElementsType() const
    {return fileType;}

  virtual bool rewind()
    {nextFilePosition = 0; return true;}

  virtual bool isExhausted() const
    {return nextFilePosition >= files.size();}

  virtual Variable next()
  {
    if (isExhausted())
      return Variable();
    jassert(nextFilePosition < files.size());
    File file = *files[nextFilePosition];
    ++nextFilePosition;
    return Variable(file.getFullPathName(), fileType);
  }

private:
  File directory;
  String wildCardPattern;
  bool searchFilesRecursively;

  juce::OwnedArray<File> files;
  int nextFilePosition;

  void initialize()
  {
    directory.findChildFiles(files, File::findFiles, searchFilesRecursively, wildCardPattern);
    nextFilePosition = 0;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_DIRECTORY_FILES_H_
