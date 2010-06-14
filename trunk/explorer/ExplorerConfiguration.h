/*-----------------------------------------.---------------------------------.
| Filename: ExplorerConfiguration.h        | Persistent Configuration        |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_CONFIGURATION_H_
# define LBCPP_EXPLORER_CONFIGURATION_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class ExplorerRecentFiles;
typedef ReferenceCountedObjectPtr<ExplorerRecentFiles> ExplorerRecentFilesPtr;

class ExplorerRecentFiles : public Object
{
public:
  ExplorerRecentFiles() {}

  static ExplorerRecentFilesPtr getInstance();

  virtual String getName() const
    {return T("ExplorerRecentFiles");}

  File getRecentDirectory() const
    {return recentDirectory;}

  void setRecentDirectory(const File& directory)
    {recentDirectory = directory;}

  size_t getNumRecentFiles() const
    {return recentFiles.size();}

  File getRecentFile(size_t index) const
    {jassert(index < recentFiles.size()); return recentFiles[index];}

  void addRecentFile(const File& file);

protected:
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;

private:
  enum {maxRecentFiles = 8};
  File recentDirectory;
  std::vector<File> recentFiles;
};

class ExplorerConfiguration : public StringToObjectMap
{
public:
  static File getApplicationDataDirectory();
  static File getConfigurationFile();

  static StringToObjectMapPtr getInstance();

  static void save()
    {getInstance()->saveToFile(getConfigurationFile());}

  template<class Type>
  static ReferenceCountedObjectPtr<Type>& getAndCast(const String& name)
    {return *(ReferenceCountedObjectPtr<Type>* )&(getInstance()->getObjects()[name]);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_CONFIGURATION_H_
