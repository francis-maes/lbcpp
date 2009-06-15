/*-----------------------------------------.---------------------------------.
| Filename: ObjectGraph.h                  | Base class for graph of objects |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_OBJECT_GRAPH_H_
# define LBCPP_OBJECT_GRAPH_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

class ObjectGraph : public Object
{
public:
  virtual size_t getNumRoots() const = 0;
  virtual ObjectPtr getRoot(size_t index) const = 0;
  virtual void setRoots(const std::vector<ObjectPtr>& successors) = 0;
  
  virtual size_t getNumSuccessors(ObjectPtr node) const = 0;
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const = 0;
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors) = 0;
  
  virtual void saveNode(std::ostream& ostr, const ObjectPtr node) const = 0;
  virtual ObjectPtr loadNode(std::istream& istr) const = 0;
  
public:
  virtual void save(std::ostream& ostr) const;
  virtual bool load(std::istream& istr);
  
  void enumerateNodes(std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const;
  
protected:
  void enumerateNodesRec(ObjectPtr node, std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_GRAPH_H_
