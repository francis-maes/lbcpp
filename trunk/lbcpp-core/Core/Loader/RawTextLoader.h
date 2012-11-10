/*-----------------------------------------.---------------------------------.
| Filename: RawTextLoader.h                | Raw text loader                 |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 16:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LOADER_RAW_TEXT_H_
# define LBCPP_CORE_LOADER_RAW_TEXT_H_

# include <lbcpp/Core/Loader.h>
# include <lbcpp/Core/String.h>

namespace lbcpp
{

class RawTextLoader : public Loader
{
public:
  virtual String getFileExtensions() const
    {return "txt";}

  virtual ClassPtr getTargetClass() const
    {return newStringClass;}
  
  virtual bool canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
    {return guessIfIsText(istr);}

  virtual ObjectPtr loadFromStream(ExecutionContext& context, juce::InputStream& istr, const String& streamName) const
    {return new NewString(istr.readEntireStreamAsString());}
};

}; /* namespace lbcpp */

#endif // LBCPP_CORE_LOADER_RAW_TEXT_H_
