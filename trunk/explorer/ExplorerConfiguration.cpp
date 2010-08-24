/*-----------------------------------------.---------------------------------.
| Filename: ExplorerConfiguration.cpp      | Persistent Configuration        |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 13:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ExplorerConfiguration.h"
using namespace lbcpp;

/*
** ExplorerRecentFiles
*/
ExplorerRecentFilesPtr ExplorerRecentFiles::getInstance()
{
  Variable& res = ExplorerConfiguration::getConfiguration(T("ExplorerRecentFiles"));
  if (!res)
    res = Variable(new ExplorerRecentFiles());
  return res.getObjectAndCast<ExplorerRecentFiles>();
}

void ExplorerRecentFiles::addRecentFile(const File& file)
{
  recentFiles->prepend(file);
  for (size_t i = 1; i < recentFiles->getNumElements(); ++i)
    if (getRecentFile(i) == file)
    {
      recentFiles->remove(i);
      break;
    }
  if (recentFiles->getNumElements() > maxRecentFiles)
    recentFiles->remove(maxRecentFiles);
}

/*
** ExplorerConfiguration
*/
File ExplorerConfiguration::getApplicationDataDirectory()
{
 // return File(T("C:\\temp"));
  File directory = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(T("LBC++"));
  if (!directory.exists() && !directory.createDirectory())
  {
    Object::error(T("ExplorerConfiguration::getApplicationDataDirectory"), T("Could not create application data directory"));
    return File::nonexistent;
  }
  return directory;
}

File ExplorerConfiguration::getConfigurationFile()
  {return getApplicationDataDirectory().getChildFile(T("config.xml"));}

VariableVectorPtr& ExplorerConfiguration::getInstancePtr()
{
  static VariableVectorPtr configuration;
  return configuration;
}

VariableVectorPtr ExplorerConfiguration::getInstance()
{
  VariableVectorPtr& configuration = getInstancePtr();
  if (!configuration)
  {
    File configurationFile = getConfigurationFile();
    if (configurationFile.exists())
      configuration = Variable::createFromFile(configurationFile).getObjectAndCast<VariableVector>();
    if (!configuration)
      configuration = new ExplorerConfiguration();
  }
  return configuration;
}
