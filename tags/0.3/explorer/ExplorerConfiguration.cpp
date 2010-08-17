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
  ExplorerRecentFilesPtr& res = ExplorerConfiguration::getAndCast<ExplorerRecentFiles>(T("ExplorerRecentFiles"));
  if (!res)
    res = new ExplorerRecentFiles();
  return res;
}

void ExplorerRecentFiles::addRecentFile(const File& file)
{
  recentFiles.insert(recentFiles.begin(), file);
  for (size_t i = 1; i < recentFiles.size(); ++i)
    if (recentFiles[i] == file)
    {
      recentFiles.erase(recentFiles.begin() + i);
      break;
    }
  if (recentFiles.size() > maxRecentFiles)
    recentFiles.pop_back();
}

bool ExplorerRecentFiles::load(InputStream& istr)
  {return lbcpp::read(istr, recentDirectory) && lbcpp::read(istr, recentFiles);}

void ExplorerRecentFiles::save(OutputStream& ostr) const
  {lbcpp::write(ostr, recentDirectory); lbcpp::write(ostr, recentFiles);}

/*
** ExplorerConfiguration
*/
File ExplorerConfiguration::getApplicationDataDirectory()
{
  File directory = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(T("LBC++"));
  if (!directory.exists() && !directory.createDirectory())
  {
    Object::error(T("ExplorerConfiguration::getApplicationDataDirectory"), T("Could not create application data directory"));
    return File::nonexistent;
  }
  return directory;
}

File ExplorerConfiguration::getConfigurationFile()
  {return getApplicationDataDirectory().getChildFile(T("config.data"));}

StringToObjectMapPtr ExplorerConfiguration::getInstance()
{
  static StringToObjectMapPtr configuration;
  if (!configuration)
  {
    File configurationFile = getConfigurationFile();
    if (configurationFile.exists())
      configuration = Object::createFromFileAndCast<StringToObjectMap>(configurationFile);
    if (!configuration)
      configuration = new ExplorerConfiguration();
  }
  return configuration;
}
