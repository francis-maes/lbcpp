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
  DirectoryFileStream(ExecutionContext& context, const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false)
    : Stream(context), directory(directory), wildCardPattern(wildCardPattern), searchFilesRecursively(searchFilesRecursively)
    {initialize();}

  DirectoryFileStream() {}

  virtual TypePtr getElementsType() const
    {return fileType;}

  virtual bool rewind()
    {nextFileIterator = files.begin(); return true;}

  virtual bool isExhausted() const
    {return nextFileIterator == files.end();}

  virtual Variable next()
  {
    if (isExhausted())
      return Variable();
    File file(*nextFileIterator);
    ++nextFileIterator;
    ++position;
    return file;
  }

  static void findChildFiles(const File& directory, const String& wildCardPattern, bool searchRecursively, std::set<String>& res)
  {
    juce::OwnedArray<File> files;
    directory.findChildFiles(files, File::findFiles, searchRecursively, wildCardPattern);
    for (int i = 0; i < files.size(); ++i)
      res.insert(files[i]->getFullPathName());
  }

  virtual ProgressionStatePtr getCurrentPosition() const
    {return new ProgressionState((double)this->position, (double)files.size(), T("Files"));}

private:
  File directory;
  String wildCardPattern;
  bool searchFilesRecursively;

  std::set<String> files;
  std::set<String>::const_iterator nextFileIterator;
  size_t position;

  void initialize()
  {
    files.clear();    
    findChildFiles(directory, wildCardPattern, searchFilesRecursively, files);
    nextFileIterator = files.begin();
    position = 0;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_DIRECTORY_FILES_H_
