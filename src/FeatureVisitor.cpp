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
void FeatureVisitor::featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
{
  if (featureEnter(dictionary, scopeIndex))
  {
    featureGenerator->accept(FeatureVisitorPtr(this));
    featureLeave();
  }
}

void FeatureVisitor::featureCall(FeatureDictionaryPtr dictionary, FeatureGeneratorPtr featureGenerator)
{
  featureGenerator->accept(FeatureVisitorPtr(this));
}

/*
** PathBasedFeatureVisitor
*/
bool PathBasedFeatureVisitor::featureEnter(FeatureDictionaryPtr dictionary, size_t index)
{
  currentPath.push_back(index);
  std::string scopeName = dictionary->getScopes()->getString(index);
  currentName.push_back(appendName(currentName.size() ? currentName.back() : "", scopeName));
  return true;
}

void PathBasedFeatureVisitor::featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
{
  std::string featureName = dictionary->getFeatures()->getString(index);
  std::string fullName = appendName(currentName.size() ? currentName.back() : "", featureName);
  currentPath.push_back(index);
  featureSense(currentPath, fullName, value);
  currentPath.pop_back();
}

void PathBasedFeatureVisitor::featureLeave()
{
  assert(currentPath.size() > 0);
  currentPath.pop_back();
  currentName.pop_back();
}

std::string PathBasedFeatureVisitor::appendName(const std::string& path, const std::string& name)
  {return path.size() ? path + "." + name : name;}    
