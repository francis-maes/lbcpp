/*-----------------------------------------.---------------------------------.
| Filename: LbcppLoader.h                  | LBC++ files loader              |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LOADER_LBCPP_H_
# define LBCPP_CORE_LOADER_LBCPP_H_

# include <oil/Core/Loader.h>
# include <oil/Execution/ExecutionTrace.h>

namespace lbcpp
{

class LbcppLoader : public Loader
{
public:
  virtual string getFileExtensions() const
    {return "xml";}

  virtual ClassPtr getTargetClass() const
    {return objectClass;}

  virtual bool canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
  {
    string firstLine = readFirstLine(istr);
    static const char* xmlBegin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    return firstLine.startsWith(string(xmlBegin));
  }

  virtual ObjectPtr loadFromFile(ExecutionContext& context, const juce::File& file) const
  {
    XmlImporter importer(context, file);
    return importer.isOpened() ? importer.load() : ObjectPtr();
  }
};

class TraceLoader : public LbcppLoader
{
public:
  virtual string getFileExtensions() const
    {return "trace";}

  virtual ClassPtr getTargetClass() const
    {return executionTraceClass;}
};

}; /* namespace lbcpp */

#endif // LBCPP_CORE_LOADER_LBCPP_H_
