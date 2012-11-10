/*-----------------------------------------.---------------------------------.
| Filename: ExplorerConfiguration.h        | Persistent Configuration        |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_CONFIGURATION_H_
# define LBCPP_EXPLORER_CONFIGURATION_H_

namespace lbcpp
{

class RecentFileVector;
typedef ReferenceCountedObjectPtr<RecentFileVector> RecentFileVectorPtr;

class RecentFileVector : public Object
{
public:
  RecentFileVector() : recentFiles(vector(newFileClass)) {}

  virtual String toShortString() const
    {return T("RecentFileVector");}

  File getRecentDirectory() const
    {return recentDirectory;}

  void setRecentDirectory(const File& directory)
    {recentDirectory = directory;}

  size_t getNumRecentFiles() const
    {return recentFiles->getNumElements();}

  File getRecentFile(size_t index) const
    {jassert(index < recentFiles->getNumElements()); return NewFile::get(recentFiles->getElement(index).getObject());}

  void addRecentFile(const File& file);
  
  void clearRecentFiles()
    {recentFiles->clear();}

private:
  friend class RecentFileVectorClass;

  enum {maxRecentFiles = 8};
  File recentDirectory;
  VectorPtr recentFiles;
};

class ExplorerConfiguration;
typedef ReferenceCountedObjectPtr<ExplorerConfiguration> ExplorerConfigurationPtr;

class ExplorerConfiguration : public Object
{
public:
  ExplorerConfiguration() : recentProjects(new RecentFileVector()) {}

  static File getApplicationDataDirectory(ExecutionContext& context);
  static File getConfigurationFile(ExecutionContext& context);

  static ExplorerConfigurationPtr& getInstancePtr();
  static ExplorerConfigurationPtr getInstance();

  static void save(ExecutionContext& context)
    {getInstance()->saveToFile(context, getConfigurationFile(context));}

  RecentFileVectorPtr getRecentProjects() const
    {return recentProjects;}

private:
  friend class ExplorerConfigurationClass;

  RecentFileVectorPtr recentProjects;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_CONFIGURATION_H_
