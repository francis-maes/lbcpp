/*-----------------------------------------.---------------------------------.
| Filename: DirectoryLoader.h              | Directory loader                |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 19:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LOADER_DIRECTORY_H_
# define LBCPP_CORE_LOADER_DIRECTORY_H_

# include <lbcpp/Core/Loader.h>
# include <lbcpp/Core/String.h>

namespace lbcpp
{

class DirectoryLoader : public Loader
{
public:
  virtual String getFileExtensions() const
    {return String::empty;}

  virtual ClassPtr getTargetClass() const
    {return directoryClass;}

  virtual bool canUnderstand(ExecutionContext& context, const juce::File& file) const
    {return file.isDirectory();}

  virtual ObjectPtr loadFromFile(ExecutionContext& context, const juce::File& file) const
    {return new Directory(file);}
};

}; /* namespace lbcpp */

#endif // LBCPP_CORE_LOADER_DIRECTORY_H_
