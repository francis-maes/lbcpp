/*-----------------------------------------.---------------------------------.
| Filename: LbcppLoader.h                  | LBC++ files loader              |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LOADER_LBCPP_H_
# define LBCPP_CORE_LOADER_LBCPP_H_

# include <lbcpp/Core/Loader.h>
# include <lbcpp/Execution/ExecutionTrace.h>

namespace lbcpp
{

class LbcppLoader : public Loader
{
public:
  virtual String getFileExtensions() const
    {return "xml";}

  virtual ClassPtr getTargetClass() const
    {return objectClass;}

  virtual bool canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
  {
    String firstLine = readFirstLine(istr);
    static const char* xmlBegin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    return firstLine.startsWith(String(xmlBegin));
  }

  virtual ObjectPtr loadFromFile(ExecutionContext& context, const File& file) const
    {return Variable::createFromFile(context, file).getObject();}
};

class TraceLoader : public LbcppLoader
{
public:
  virtual String getFileExtensions() const
    {return "trace";}

  virtual ClassPtr getTargetClass() const
    {return executionTraceClass;}
};

}; /* namespace lbcpp */

#endif // LBCPP_CORE_LOADER_LBCPP_H_
