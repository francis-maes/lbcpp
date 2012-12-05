/*-----------------------------------------.---------------------------------.
| Filename: RawTextLoader.h                | Raw text loader                 |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 16:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LOADER_RAW_TEXT_H_
# define LBCPP_CORE_LOADER_RAW_TEXT_H_

# include <oil/Core/Loader.h>
# include <oil/Core/String.h>

namespace lbcpp
{

class RawTextLoader : public Loader
{
public:
  virtual string getFileExtensions() const
    {return "txt";}

  virtual ClassPtr getTargetClass() const
    {return stringClass;}
  
  virtual bool canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
    {return guessIfIsText(istr);}

  virtual ObjectPtr loadFromStream(ExecutionContext& context, juce::InputStream& istr, const string& streamName) const
    {return new String(istr.readEntireStreamAsString());}
};

}; /* namespace lbcpp */

#endif // LBCPP_CORE_LOADER_RAW_TEXT_H_
