/*-----------------------------------------.---------------------------------.
| Filename: ObjectGraph.h                  | Base class for graph of objects |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   ObjectGraph.h
**@author Francis MAES
**@date   Mon Jun 15 19:58:26 2009
**
**@brief  Base class for directed graph representation declaration.
**
**
*/

#ifndef LBCPP_OBJECT_GRAPH_H_
# define LBCPP_OBJECT_GRAPH_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

/**
** @class ObjectGraph
** @brief Base class for directed graph representation.
*/

class ObjectGraph : public Object
{
public:
  /**
  ** Returns the number of roots.
  **
  ** @return number of roots.
  */
  virtual size_t getNumRoots() const = 0;

  /**
  ** Returns root of node @a index.
  **
  ** @param index : node index.
  **
  ** @return root (object pointer) of node @a index.
  */
  virtual ObjectPtr getRoot(size_t index) const = 0;

  /**
  ** Roots setter.
  **
  ** @param successors : successors list.
  */
  virtual void setRoots(const std::vector<ObjectPtr>& successors) = 0;

  /**
  ** Returns the number of successors of the node @a node.
  **
  ** @param node : origin node.
  **
  ** @return the number of successors of the node @a node.
  */
  virtual size_t getNumSuccessors(ObjectPtr node) const = 0;

  /**
  ** Returns the successor of the node @a node at the index @a index.
  **
  ** @param node : origin node.
  ** @param index : successor index.
  **
  ** @return a pointer on the @a index@em th successor of @a node.
  */
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const = 0;

  /**
  ** Successors setter.
  **
  ** @param node : target node.
  ** @param successors : node successor list.
  */
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors) = 0;

  /**
  ** Saves @a node contain into the output stream @a ostr.
  **
  ** @param ostr : output stream.
  ** @param node : target node.
  */
  virtual void saveNode(std::ostream& ostr, const ObjectPtr node) const = 0;

  /**
  ** Loads a node from an input stream.
  **
  ** @param istr : input stream.
  **
  ** @return an Object pointer.
  */
  virtual ObjectPtr loadNode(std::istream& istr) const = 0;

public:
  /**
  ** Saves the current directed graph.
  **
  ** @see ObjectGraph::saveNode
  **
  ** @param ostr : output stream.
  */
  virtual void save(std::ostream& ostr) const;

  /**
  ** Loads a directed graph contain from an input stream.
  **
  ** @see ObjectGraph::loadNode
  ** @see ObjectGraph::setRoots
  ** @see ObjectGraph::setSuccessors
  **
  ** @param istr : input stream.
  **
  ** @return a boolean.
  */
  virtual bool load(std::istream& istr);

  /**
  ** Fills up @a nodes table with graph node contain and also fills
  ** up the inverse table. The inverse table is a map that links node
  ** contain and their index in the table.
  **
  ** @param nodes : node contain table.
  ** @param inverseTable : inverse node contain table.
  */
  void enumerateNodes(std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const;

protected:
  void enumerateNodesRec(ObjectPtr node, std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_GRAPH_H_
