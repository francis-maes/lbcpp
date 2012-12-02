/*-----------------------------------------.---------------------------------.
| Filename: XmlLoader.h                    | XML File loader                 |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 17:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_LOADER_XML_H_
# define LBCPP_CORE_LOADER_XML_H_

# include <lbcpp/Core/Loader.h>
# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{

class XmlLoader : public Loader
{
public:
  virtual string getFileExtensions() const
    {return "xml";}

  virtual ClassPtr getTargetClass() const
    {return xmlElementClass;}

  virtual bool canUnderstand(ExecutionContext& context, juce::InputStream& istr) const
  {
    string firstLine = readFirstLine(istr).trim();
    return firstLine.isNotEmpty() && firstLine[0] == '<';
  }

  virtual ObjectPtr loadFromFile(ExecutionContext& context, const juce::File& file) const
  {
    juce::XmlDocument document(file);
    juce::XmlElement* root = document.getDocumentElement();
    string lastParseError = document.getLastParseError();
    if (!root)
    {
      context.errorCallback(T("XmlLoader::loadFromFile"),
        lastParseError.isEmpty() ? T("Could not parse file ") + file.getFullPathName() : lastParseError);
      return ObjectPtr();
    }
    if (lastParseError.isNotEmpty())
      context.warningCallback(T("XmlLoader::loadFromFile"), lastParseError);
    return XmlElement::createFromJuceXml(root, true);
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_CORE_LOADER_XML_H_
