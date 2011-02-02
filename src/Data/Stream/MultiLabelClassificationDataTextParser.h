/*-----------------------------------------.---------------------------------.
| Filename: MultiLabelClassific...Parser.h | Default parser for              |
| Author  : Francis Maes                   |   multi-label classification    |
| Started : 15/01/2011 15:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_STREAM_MULTI_LABEL_CLASSIFICATION_TEXT_PARSER_H_
# define LBCPP_DATA_STREAM_MULTI_LABEL_CLASSIFICATION_TEXT_PARSER_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

class MultiLabelClassificationDataTextParser : public LearningDataTextParser
{
public:
  MultiLabelClassificationDataTextParser(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr outputLabels)
    : LearningDataTextParser(context, file), inputClass(inputClass), outputLabels(outputLabels)
  {
    elementsType = pairClass(inputClass, enumBasedDoubleVectorClass(outputLabels, probabilityType));
  }
  MultiLabelClassificationDataTextParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  ObjectPtr parseLabelsList(EnumerationPtr labels, const String& text) const
  {
    StringArray tokens;
    tokens.addTokens(text, T(","), NULL);
    ObjectPtr res = new SparseDoubleObject(enumBasedDoubleVectorClass(outputLabels, probabilityType).staticCast<DynamicClass>().get());
    for (int i = 0; i < tokens.size(); ++i)
    {
      size_t label = outputLabels->findOrAddElement(context, tokens[i]);
      res->setVariable(label, 1.0);
    }
    return res;
  }

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    ObjectPtr labels = parseLabelsList(outputLabels, columns[0]);
    ObjectPtr features = parseFeatureList(inputClass, columns, 1);
    if (!labels || !features)
      return false;
    setResult(new Pair(elementsType, features, labels));
    return true;
  }

private:
  DynamicClassPtr inputClass;
  DefaultEnumerationPtr outputLabels;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_MULTI_LABEL_CLASSIFICATION_TEXT_PARSER_H_
