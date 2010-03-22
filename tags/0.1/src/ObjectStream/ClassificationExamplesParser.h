/*-----------------------------------------.---------------------------------.
| Filename: ClassificationExamplesParser.h | Default parser for              |
| Author  : Francis Maes                   |   classification examples       |
| Started : 22/06/2009 18:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_PARSER_H_
# define LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_PARSER_H_

# include <lbcpp/ObjectStream.h>

namespace lbcpp
{

class ClassificationExamplesParser : public LearningDataObjectParser
{
public:
  ClassificationExamplesParser(const std::string& filename, FeatureDictionaryPtr features, StringDictionaryPtr labels)
    : LearningDataObjectParser(filename, features), labels(labels) {}

  virtual std::string getContentClassName() const
    {return "ClassificationExample";}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    std::string label;
    if (!TextObjectParser::parse(columns[0], label))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    setResult(new ClassificationExample(x, labels->add(label)));
    return true;
  }
  
private:
  StringDictionaryPtr labels;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_PARSER_H_
