/*-----------------------------------------.---------------------------------.
| Filename: ClassificationDataTextParser.h | Default parser for              |
| Author  : Francis Maes                   |   classification examples       |
| Started : 22/06/2009 18:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_STREAM_CLASSIFICATION_TEXT_PARSER_H_
# define LBCPP_DATA_STREAM_CLASSIFICATION_TEXT_PARSER_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

class ClassificationDataTextParser : public LearningDataTextParser
{
public:
  ClassificationDataTextParser(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr outputLabels)
    : LearningDataTextParser(context, file), inputClass(inputClass), outputLabels(outputLabels)
  {
    elementsType = pairClass(inputClass, outputLabels);
  }
  ClassificationDataTextParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    size_t label = outputLabels->findOrAddElement(context, columns[0]);
    ObjectPtr features = parseFeatureList(inputClass, columns, 1);
    if (!features)
      return false;
    setResult(new Pair(elementsType, features, Variable(label, outputLabels)));
    return true;
  }

private:
  DynamicClassPtr inputClass;
  DefaultEnumerationPtr outputLabels;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_CLASSIFICATION_TEXT_PARSER_H_
