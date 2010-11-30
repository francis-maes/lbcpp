/*-----------------------------------------.---------------------------------.
| Filename: SaveToFileConsumer.h           | Save To File Consumer           |
| Author  : Julien Becker                  |                                 |
| Started : 30/11/2010 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONSUMER_SAVE_TO_FILE_H_
# define LBCPP_DATA_CONSUMER_SAVE_TO_FILE_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{
  
class SaveToFileConsumer : public Consumer
{
public:
  SaveToFileConsumer(const File& outputDirectory) : outputDirectory(outputDirectory) {}
  SaveToFileConsumer() {}

  virtual void consume(ExecutionContext& context, const Variable& variable)
  {
    String fileName;
    if (variable.isObject())
    {
      NameableObjectPtr obj = variable.getObjectAndCast<NameableObject>(context);
      if (obj)
        fileName = obj->getName();
    }
    if (fileName == String::empty)
      fileName = variable.toString();

    variable.saveToFile(context, outputDirectory.getChildFile(fileName + T(".xml")));
  }

protected:
  friend class SaveToFileConsumerClass;
  
  File outputDirectory;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONSUMER_SAVE_TO_FILE_H_
