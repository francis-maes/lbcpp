/*-----------------------------------------.---------------------------------.
| Filename: ExplorerConfiguration.cpp      | Persistent Configuration        |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 13:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ExplorerConfiguration.h"
using namespace lbcpp;

/*
** RecentFileVector
*/
void RecentFileVector::addRecentFile(const juce::File& file)
{
  recentFiles->prependElement(File::create(file));
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
juce::File ExplorerConfiguration::getApplicationDataDirectory(ExecutionContext& context)
{
  //return File(JUCE_T("C:\\temp"));
  juce::File directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(JUCE_T("LBC++"));
  if (!directory.exists() && !directory.createDirectory())
  {
    context.errorCallback(JUCE_T("ExplorerConfiguration::getApplicationDataDirectory"), JUCE_T("Could not create application data directory"));
    return juce::File::nonexistent;
  }
  return directory;
}

juce::File ExplorerConfiguration::getConfigurationFile(ExecutionContext& context)
  {return getApplicationDataDirectory(context).getChildFile(JUCE_T("config.xml"));}

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
    juce::File configurationFile = getConfigurationFile(defaultExecutionContext());
    if (configurationFile.exists())
      configuration = ExplorerConfiguration::createFromFile(defaultExecutionContext(), configurationFile);
    if (!configuration)
      configuration = new ExplorerConfiguration();
  }
  return configuration;
}
