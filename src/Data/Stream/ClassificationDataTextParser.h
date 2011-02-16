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
  ClassificationDataTextParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features, DefaultEnumerationPtr labels)
    : LearningDataTextParser(context, file), features(features), labels(labels)
    {elementsType = pairClass(sparseDoubleVectorClass(features), labels);}

  ClassificationDataTextParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    size_t label = labels->findOrAddElement(context, columns[0]);
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, Variable(label, labels)));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  DefaultEnumerationPtr labels;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_CLASSIFICATION_TEXT_PARSER_H_
