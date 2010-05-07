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

class ExplorerConfiguration : public StringToObjectMap
{
public:
  static File getApplicationDataDirectory()
  {return File(T("C:\\temp"));
    File directory = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(T("LBC++"));
    if (!directory.exists() && !directory.createDirectory())
    {
      Object::error(T("ExplorerConfiguration::getApplicationDataDirectory"), T("Could not create application data directory"));
      return File::nonexistent;
    }
    return directory;
  }

  static File getConfigurationFile()
    {return getApplicationDataDirectory().getChildFile(T("config.data"));}

  static StringToObjectMapPtr getInstance()
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

  static void save()
    {getInstance()->saveToFile(getConfigurationFile());}

  template<class Type>
  static ReferenceCountedObjectPtr<Type>& getAndCast(const String& name)
    {return *(ReferenceCountedObjectPtr<Type>* )&(getInstance()->getObjects()[name]);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_CONFIGURATION_H_
