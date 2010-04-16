/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitor.cpp             | Feature Visitor                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2010 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/FeatureVisitor.h>
using namespace lbcpp;

/*
** FeatureVisitor
*/
void FeatureVisitor::featureCall(FeatureDictionaryPtr dictionary, size_t scopeNumber, FeatureGeneratorPtr featureGenerator, double weight)
{
  if (featureEnter(dictionary, scopeNumber, featureGenerator->getDictionary(), weight))
  {
    featureGenerator->accept(FeatureVisitorPtr(this));
    featureLeave();
  }
}

/*
** PathBasedFeatureVisitor
*/
bool PathBasedFeatureVisitor::featureEnter(FeatureDictionaryPtr dictionary, size_t index, FeatureDictionaryPtr subDictionary, double weight)
{
  double w = getCurrentWeight() * weight;
  if (!w)
    return false;
  currentWeight.push_back(w);
  currentPath.push_back(index);
  String scopeName = dictionary->getScopes()->getString(index);
  currentName.push_back(appendName(currentName.size() ? currentName.back() : "", scopeName));
  return true;
}

void PathBasedFeatureVisitor::featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
{
  value *= getCurrentWeight();
  if (!value)
    return;
  String featureName = dictionary->getFeatures()->getString(index);
  String fullName = appendName(currentName.size() ? currentName.back() : "", featureName);
  currentPath.push_back(index);
  featureSense(currentPath, fullName, value);
  currentPath.pop_back();
}

void PathBasedFeatureVisitor::featureLeave()
{
  jassert(currentPath.size() > 0);
  currentPath.pop_back();
  currentName.pop_back();
  currentWeight.pop_back();
}

String PathBasedFeatureVisitor::appendName(const String& path, const String& name)
  {return path.isNotEmpty() ? path + "." + name : name;}    
