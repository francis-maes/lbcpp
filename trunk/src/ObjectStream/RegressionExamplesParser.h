/*-----------------------------------------.---------------------------------.
| Filename: RegressionExamplesParser.h     | Default parser for              |
| Author  : Francis Maes                   |   regression examples           |
| Started : 22/06/2009 18:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_REGRESSION_EXAMPLES_PARSER_H_
# define LBCPP_OBJECT_STREAM_REGRESSION_EXAMPLES_PARSER_H_

# include <lbcpp/ObjectStream.h>

namespace lbcpp
{

class RegressionExamplesParser : public LearningDataObjectParser
{
public:
  RegressionExamplesParser(const File& file, FeatureDictionaryPtr features)
    : LearningDataObjectParser(file, features) {}

  virtual String getContentClassName() const
    {return T("RegressionExample");}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    double y = columns[0].getDoubleValue();
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    setResult(new RegressionExample(x, y));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_REGRESSION_EXAMPLES_PARSER_H_
