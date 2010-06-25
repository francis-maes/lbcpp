/*-----------------------------------------.---------------------------------.
| Filename: StringDescriptionVisitor.hpp   | A visitor to describe vectors   |
| Author  : Francis Maes                   |  as strings                     |
| Started : 27/02/2009 22:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_
# define LBCPP_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_

# include "../FeatureVisitorStatic.hpp"

namespace lbcpp {
namespace impl {

class StringDescriptionVisitor : public WeightStackBasedFeatureVisitor<StringDescriptionVisitor>
{
public: 
  StringDescriptionVisitor() : indent(0) {}

  typedef WeightStackBasedFeatureVisitor<StringDescriptionVisitor> BaseClass;

  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary, double weight)
  {
    if (!BaseClass::featureEnter(dictionary, number, subDictionary, weight))
      return false;
   //res += "DICO = " + dictionary->getName() + " (" + lbcpp::toString((int)dictionary.get()) + ")\n";
    flushCurrentFeatures();
    addIndent(indent);
    res += dictionary->getScopes()->getString(number) + "\n";
    ++indent;
    return true;
  }

  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value)
  {
    value *= BaseClass::currentWeight;
    if (value)
    {
      if (currentFeatures.isNotEmpty())
        currentFeatures += T(", ");
      currentFeatures += dictionary->getFeatures()->getString(number) + T(" = ") + lbcpp::toString(value);
    }
  }
  
  void featureLeave()
  {
    BaseClass::featureLeave();
    flushCurrentFeatures();
    --indent;
  }

  void flushCurrentFeatures()
  {
    if (currentFeatures.isNotEmpty())
    {
      addIndent(indent);
      res += currentFeatures + T("\n");
      currentFeatures = String::empty;
    }
  }

  String getResult() const
  {
    if (currentFeatures.isNotEmpty() && res.isEmpty())
      return T("{") + currentFeatures + T("}");
      
    if (currentFeatures.isEmpty())
      return res.isEmpty() ? T("<empty>") : res;
    
    return res + T("\n") + currentFeatures;
  }

private:
  String currentFeatures;
  String res;
  int indent;

  inline void addIndent(int indent)
    {for (int i = 0; i < indent; ++i) res += T("  ");}
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_
