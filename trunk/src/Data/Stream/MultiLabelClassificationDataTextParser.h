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
  MultiLabelClassificationDataTextParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features, DefaultEnumerationPtr labels)
    : LearningDataTextParser(context, file), features(features), labels(labels)
    {elementsType = pairClass(sparseDoubleVectorClass(features), sparseDoubleVectorClass(labels, probabilityType));}

  MultiLabelClassificationDataTextParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  SparseDoubleVectorPtr parseLabelsList(const String& text) const
  {
    StringArray tokens;
    tokens.addTokens(text, T(","), NULL);
    SparseDoubleVectorPtr res = new SparseDoubleVector(labels, probabilityType);
    for (int i = 0; i < tokens.size(); ++i)
    {
      size_t label = labels->findOrAddElement(context, tokens[i]);
      res->setElement(label, 1.0);
    }
    return res;
  }

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    SparseDoubleVectorPtr labelsVector = parseLabelsList(columns[0]);
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!labelsVector || !featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, labelsVector));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  DefaultEnumerationPtr labels;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_MULTI_LABEL_CLASSIFICATION_TEXT_PARSER_H_
