/*-----------------------------------------.---------------------------------.
| Filename: ClassificationExamplesParser.h | Default parser for              |
| Author  : Francis Maes                   |   classification examples       |
| Started : 22/06/2009 18:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_PARSER_H_
# define LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_PARSER_H_

# include <lbcpp/Object/ObjectStream.h>

namespace lbcpp
{
/*
class ClassificationExamplesParser : public LearningDataObjectParser
{
public:
  ClassificationExamplesParser(const File& file, FeatureDictionaryPtr features, FeatureDictionaryPtr labels)
    : LearningDataObjectParser(file, features), labels(labels) {}

  virtual String getContentClassName() const
    {return T("ClassificationExample");}

  virtual bool parseDataLine(const std::vector<String>& columns)
  {
    String label = columns[0];
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    setResult(new ClassificationExample(x, labels->addFeature(label)));
    return true;
  }
  
private:
  FeatureDictionaryPtr labels;
};*/

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_CLASSIFICATION_EXAMPLES_PARSER_H_
