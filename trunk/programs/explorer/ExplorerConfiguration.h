/*-----------------------------------------.---------------------------------.
| Filename: ExplorerConfiguration.h        | Persistent Configuration        |
| Author  : Francis Maes                   |                                 |
| Started : 07/05/2010 13:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_CONFIGURATION_H_
# define EXPLORER_CONFIGURATION_H_

namespace lbcpp
{

class RecentFileVector;
typedef ReferenceCountedObjectPtr<RecentFileVector> RecentFileVectorPtr;

class RecentFileVector : public Object
{
public:
  RecentFileVector() : recentFiles(vector(fileClass)) {}

  virtual string toShortString() const
    {return T("RecentFileVector");}

  juce::File getRecentDirectory() const
    {return recentDirectory;}

  void setRecentDirectory(const juce::File& directory)
    {recentDirectory = directory;}

  size_t getNumRecentFiles() const
    {return recentFiles->getNumElements();}

  juce::File getRecentFile(size_t index) const
    {jassert(index < recentFiles->getNumElements()); return File::get(recentFiles->getElement(index));}

  void addRecentFile(const juce::File& file);
  
  void clearRecentFiles()
    {recentFiles->clear();}

private:
  friend class RecentFileVectorClass;

  enum {maxRecentFiles = 8};
  juce::File recentDirectory;
  VectorPtr recentFiles;
};

class ExplorerConfiguration;
typedef ReferenceCountedObjectPtr<ExplorerConfiguration> ExplorerConfigurationPtr;

class ExplorerConfiguration : public Object
{
public:
  ExplorerConfiguration() : recentProjects(new RecentFileVector()) {}

  static juce::File getApplicationDataDirectory(ExecutionContext& context);
  static juce::File getConfigurationFile(ExecutionContext& context);

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

#endif // !EXPLORER_CONFIGURATION_H_
