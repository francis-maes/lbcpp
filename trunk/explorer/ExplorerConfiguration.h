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
  ExplorerRecentFiles() : recentFiles(new Vector(fileType())) {}

  static ExplorerRecentFilesPtr getInstance();

  virtual String getName() const
    {return T("ExplorerRecentFiles");}

  File getRecentDirectory() const
    {return recentDirectory;}

  void setRecentDirectory(const File& directory)
    {recentDirectory = directory;}

  size_t getNumRecentFiles() const
    {return recentFiles->size();}

  File getRecentFile(size_t index) const
    {jassert(index < recentFiles->size()); return recentFiles->getVariable(index).getFile();}

  void addRecentFile(const File& file);
  
  void clearRecentFiles()
    {recentFiles->clear();}

private:
  friend class ExplorerRecentFilesClass;

  enum {maxRecentFiles = 8};
  File recentDirectory;
  VectorPtr recentFiles;
};

extern ClassPtr explorerConfigurationClass();

class ExplorerConfiguration : public DynamicObject
{
public:
  ExplorerConfiguration() : DynamicObject(explorerConfigurationClass()) {}

  static File getApplicationDataDirectory();
  static File getConfigurationFile();

  static DynamicObjectPtr getInstance();

  static void save()
    {Variable(getInstance()).saveToFile(getConfigurationFile());}

  static Variable& getConfiguration(const String& typeName)
  {
    DynamicObjectPtr object = getInstance();
    for (size_t i = 0; i < object->getNumVariables(); ++i)
    {
      Variable& variable = object->getVariable(i);
      if (variable.getTypeName() == typeName)
        return variable;
    }
    TypePtr type = Type::get(typeName);
    jassert(type);
    object->appendVariable(Variable::create(type));
    return object->getVariable(object->getNumVariables() - 1);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_CONFIGURATION_H_
