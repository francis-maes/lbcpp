/*-----------------------------------------.---------------------------------.
| Filename: ResultsTreeView.h              | Results Tree View               |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2012 12:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_TREE_VIEW_RESULTS_H_
# define LBCPP_USER_INTERFACE_TREE_VIEW_RESULTS_H_

# include "GenericTreeView.h"

namespace lbcpp
{

class ResultsTreeView : public GenericTreeView
{
public:
  ResultsTreeView(VectorPtr vector, const string& name) : GenericTreeView(vector, name, false)
    {buildTree();}

  virtual bool mightHaveSubObjects(const ObjectPtr& object)
    {return object == this->object;}
    
  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object)
  {
    OVectorPtr vector = object.staticCast<OVector>();
    std::vector< std::pair<string, ObjectPtr> > res(vector->getNumElements());
    for (size_t i = 0; i < res.size(); ++i)
    {
      PairPtr p = vector->get(i).staticCast<Pair>();
      res[i] = std::make_pair(String::get(p->getFirst()), p->getSecond());
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_RESULTS_H_
