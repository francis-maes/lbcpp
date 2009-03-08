/*-----------------------------------------.---------------------------------.
| Filename: StringDescriptionVisitor.hpp   | A visitor to describe vectors   |
| Author  : Francis Maes                   |  as strings                     |
| Started : 27/02/2009 22:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_
# define CRALGO_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_

# include "StaticFeatureVisitor.hpp"

namespace cralgo
{

class StringDescriptionVisitor : public StaticFeatureVisitor<StringDescriptionVisitor>
{
public: 
  StringDescriptionVisitor() : indent(0) {}

  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
  {
    addIndent(indent);
    res += dictionary.getScopes().getString(number) + "\n";
    ++indent;
    return true;
  }

  void featureSense(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
  {
    if (currentFeatures.size())
      currentFeatures += ", ";
    currentFeatures += dictionary.getFeatures().getString(number) + " = " + cralgo::toString(value);
  }

  void featureLeave()
  {
    if (currentFeatures.size())
    {
      addIndent(indent);
      res += currentFeatures + "\n";
      currentFeatures = "";
    }
    --indent;
  }
  
  std::string getResult() const
  {
    if (currentFeatures.size() && !res.size())
      return currentFeatures;
    if (res.size() && !currentFeatures.size())
      return res;
    if (!res.size() && !currentFeatures.size())
      return "<empty>";
    return res + "\n" + currentFeatures;
  }

private:
  std::string currentFeatures;
  std::string res;
  int indent;

  inline void addIndent(int indent)
    {for (int i = 0; i < indent; ++i) res += "  ";}
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_
