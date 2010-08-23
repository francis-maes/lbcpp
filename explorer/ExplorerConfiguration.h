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
    {return recentFiles->getNumElements();}

  File getRecentFile(size_t index) const
    {jassert(index < recentFiles->getNumElements()); return recentFiles->getElement(index).getFile();}

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

class ExplorerConfiguration : public VariableVector
{
public:
  ExplorerConfiguration() : VariableVector() {}

  static File getApplicationDataDirectory();
  static File getConfigurationFile();

  static VariableVectorPtr getInstance();

  static void save()
    {Variable(getInstance()).saveToFile(getConfigurationFile());}

  static Variable& getConfiguration(const String& typeName)
  {
    // FIXME !
    VariableVectorPtr object = getInstance();
    for (size_t i = 0; i < object->getNumElements(); ++i)
    {
      Variable& variable = object->getElement(i);
      if (variable.getTypeName() == typeName)
        return variable;
    }
    TypePtr type = Type::get(typeName);
    jassert(type);
    object->append(Variable::create(type));
    return object->getElement(object->getNumElements() - 1);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_CONFIGURATION_H_
