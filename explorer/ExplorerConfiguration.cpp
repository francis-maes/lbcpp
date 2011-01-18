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
** RecentFileVector
*/
void RecentFileVector::addRecentFile(const File& file)
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
File ExplorerConfiguration::getApplicationDataDirectory(ExecutionContext& context)
{
  //return File(T("C:\\temp"));
  File directory = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(T("LBC++"));
  if (!directory.exists() && !directory.createDirectory())
  {
    context.errorCallback(T("ExplorerConfiguration::getApplicationDataDirectory"), T("Could not create application data directory"));
    return File::nonexistent;
  }
  return directory;
}

File ExplorerConfiguration::getConfigurationFile(ExecutionContext& context)
  {return getApplicationDataDirectory(context).getChildFile(T("config.xml"));}

ExplorerConfigurationPtr& ExplorerConfiguration::getInstancePtr()
{
  static ExplorerConfigurationPtr configuration;
  return configuration;
}

ExplorerConfigurationPtr ExplorerConfiguration::getInstance()
{
  ExplorerConfigurationPtr& configuration = getInstancePtr();
  if (!configuration)
  {
    File configurationFile = getConfigurationFile(defaultExecutionContext());
    if (configurationFile.exists())
      configuration = ExplorerConfiguration::createFromFile(defaultExecutionContext(), configurationFile);
    if (!configuration)
      configuration = new ExplorerConfiguration();
  }
  return configuration;
}
