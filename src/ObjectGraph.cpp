/*-----------------------------------------.---------------------------------.
| Filename: ObjectGraph.cpp                | Graph of objects                |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 17:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/ObjectGraph.h>
using namespace lbcpp;

void ObjectGraph::enumerateNodes(std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const
{
  size_t n = getNumRoots();
  for (size_t i = 0; i < n; ++i)
    enumerateNodesRec(getRoot(i), nodes, inverseTable);
}

void ObjectGraph::enumerateNodesRec(ObjectPtr node, std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const
{
  std::map<ObjectPtr, size_t>::iterator it = inverseTable.find(node);
  if (it == inverseTable.end())
  {
    inverseTable[node] = nodes.size();
    nodes.push_back(node);
  }
  size_t n = getNumSuccessors(node);
  for (size_t i = 0; i < n; ++i)
    enumerateNodesRec(getSuccessor(node, i), nodes, inverseTable);
}

void ObjectGraph::save(OutputStream& ostr) const
{
  /*
  ** Enumerate nodes
  */
  std::vector<ObjectPtr> nodes;
  std::map<ObjectPtr, size_t> inverseTable;
  enumerateNodes(nodes, inverseTable);
  
  /*
  ** Save all nodes
  */
  write(ostr, nodes.size());
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    ObjectPtr node = nodes[i];
    saveNode(ostr, node);
    
    // Save node successors
    size_t n = getNumSuccessors(node);
    write(ostr, n);
    for (size_t j = 0; j < n; ++j)
    {
      std::map<ObjectPtr, size_t>::iterator it = inverseTable.find(getSuccessor(node, j));
      jassert(it != inverseTable.end());
      jassert(it->second < nodes.size());
      write(ostr, it->second);
    }
  }
  
  /*
  ** Save roots
  */
  size_t n = getNumRoots();
  write(ostr, n);
  for (size_t i = 0; i < n; ++i)
  {
    std::map<ObjectPtr, size_t>::iterator it = inverseTable.find(getRoot(i));
    jassert(it != inverseTable.end());
    jassert(it->second < nodes.size());
    write(ostr, it->second);    
  }
}

bool ObjectGraph::load(InputStream& istr)
{
  size_t numNodes;
  if (!read(istr, numNodes))
    return false;
  std::vector< ObjectPtr > nodes(numNodes);
  std::vector< std::vector<size_t> > successors(numNodes);
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    ObjectPtr node = loadNode(istr);
    if (!node)
    {
      Object::error("ObjectGraph::load", "Could not load node number " + lbcpp::toString(i));
      return false;
    }
    if (!read(istr, successors[i]))
      return false;
    nodes[i] = node;
  }
  
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    std::vector<size_t>& succ = successors[i];
    std::vector<ObjectPtr> succObjects(succ.size());
    for (size_t j = 0; j < succ.size(); ++j)
    {
      jassert(succ[j] < numNodes);
      succObjects[j] = nodes[succ[j]];
    }
    setSuccessors(nodes[i], succObjects);
  }
  
  std::vector<size_t> roots;
  if (!read(istr, roots))
    return false;
  std::vector<ObjectPtr> rootObjects(roots.size());
  for (size_t i = 0; i < roots.size(); ++i)
  {
    jassert(roots[i] < numNodes);
    rootObjects[i] = nodes[roots[i]];
  }
  setRoots(rootObjects);
  return true;
}
