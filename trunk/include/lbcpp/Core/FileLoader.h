/*-----------------------------------------.---------------------------------.
| Filename: FileLoader.h                   | File Loader Base Class          |
| Author  : Francis Maes                   |                                 |
| Started : 10/11/2012 13:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_FILE_LOADER_H_
# define LBCPP_DATA_FILE_LOADER_H_

# include "../Core/Object.h"
# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class FileLoader : public Object
{
public:
  virtual String getFileExtensions() const = 0; // separated by semi colons, e.g. "jpg;tga;bmp"
  virtual ClassPtr getTargetClass() const = 0; // returns the type of the loaded object

  virtual bool canUnderstand(ExecutionContext& context, const File& file) const;
  virtual bool canUnderstand(ExecutionContext& context, juce::InputStream& istr) const;

  virtual ObjectPtr loadFromFile(ExecutionContext& context, const File& file) const;
  virtual ObjectPtr loadFromStream(ExecutionContext& context, juce::InputStream& istr, const String& streamName) const;

  static juce::InputStream* openFile(ExecutionContext& context, const File& file, bool doErrorMessages = true);
  static String readFirstLine(juce::InputStream& istr, size_t maxLength = 256);
  static bool guessIfIsText(juce::InputStream& istr);
};

extern ClassPtr fileLoaderClass;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_FILE_LOADER_H_
