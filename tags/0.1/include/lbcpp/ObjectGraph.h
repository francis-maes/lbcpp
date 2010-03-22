/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
** @brief Base class for directed graphs with nodes inherited from Object.
** This base class defines a wrapper from objects to
** a generic directed graph representation.
** It provides generic tools for serialization of the graph
** and for enumeration of its content.
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
  ** Returns one of the root nodes.
  **
  ** @param index : node index in range [0, getNumRoots()[
  **
  ** @return the root node @a index
  */
  virtual ObjectPtr getRoot(size_t index) const = 0;

  /**
  ** Returns the number of successors of a given node.
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
  ** @return a pointer on the @a index successor of @a node.
  */
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const = 0;

protected:
  /**
  ** Roots setter.
  **
  ** This function is used for serialization purpose.
  **
  ** @param roots : the vector of root nodes.
  ** @see load
  */
  virtual void setRoots(const std::vector<ObjectPtr>& roots) = 0;

  /**
  ** Successors setter.
  **
  ** This function is used for serialization purpose.
  **
  ** @param node : target node.
  ** @param successors : node successor list.
  ** @see load
  */
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors) = 0;

  /**
  ** Loads the content of a node from a C++ input stream.
  **
  ** @param istr : input stream.
  **
  ** @return an Object pointer.
  */
  virtual ObjectPtr loadNode(std::istream& istr) const = 0;

  /**
  ** Saves the content of a node to a C++ output stream.
  **
  ** @param ostr : output stream.
  ** @param node : node to save.
  */
  virtual void saveNode(std::ostream& ostr, const ObjectPtr node) const = 0;

public:
  /**
  ** Saves the current directed graph to a C++ output stream.
  **
  ** @param ostr : output stream.
  ** @see saveNode
  */
  virtual void save(std::ostream& ostr) const;

  /**
  ** Loads a directed graph from a C++ input stream.
  **
  ** @param istr : input stream.
  ** @return a boolean.
  ** @see loadNode
  ** @see setRoots
  ** @see setSuccessors
  */
  virtual bool load(std::istream& istr);

  /**
  ** Enumerate the nodes contained in this graph.
  **
  ** The nodes are filled into the @a nodes vector, ensuring that
  ** each element of @a nodes is unique. This function also
  ** fills up @a inverseTable, a map that associate nodes to their index in @a nodes.
  **
  ** For any node <i>n</i> of the graph, we have:
  ** @code
  ** nodes[inverseTable[n]] == n
  ** @endcode
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
