/*-----------------------------------------.---------------------------------.
| Filename: RegressionDataTextParser.h     | Default parser for              |
| Author  : Francis Maes                   |   regression examples           |
| Started : 22/06/2009 18:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_STREAM_REGRESSION_TEXT_PARSER_H_
# define LBCPP_DATA_STREAM_REGRESSION_TEXT_PARSER_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

class RegressionDataTextParser : public LearningDataTextParser
{
public:
  RegressionDataTextParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr features)
    : LearningDataTextParser(context, file), features(features)
    {elementsType = pairClass(sparseDoubleVectorClass(features), doubleType);}

  RegressionDataTextParser() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    SparseDoubleVectorPtr featuresVector = parseFeatureList(features, columns, 1);
    if (!featuresVector)
      return false;
    setResult(new Pair(elementsType, featuresVector, columns[0].getDoubleValue()));
    return true;
  }

private:
  DefaultEnumerationPtr features;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_STREAM_REGRESSION_TEXT_PARSER_H_
