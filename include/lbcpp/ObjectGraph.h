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
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_OBJECT_GRAPH_H_
# define LBCPP_OBJECT_GRAPH_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

/*!
** @class ObjectGraph
** @brief
*/

class ObjectGraph : public Object
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual size_t getNumRoots() const = 0;

  /*!
  **
  **
  ** @param index
  **
  ** @return
  */
  virtual ObjectPtr getRoot(size_t index) const = 0;

  /*!
  **
  **
  ** @param successors
  */
  virtual void setRoots(const std::vector<ObjectPtr>& successors) = 0;

  /*!
  **
  **
  ** @param node
  **
  ** @return
  */
  virtual size_t getNumSuccessors(ObjectPtr node) const = 0;

  /*!
  **
  **
  ** @param node
  ** @param index
  **
  ** @return
  */
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const = 0;

  /*!
  **
  **
  ** @param node
  ** @param successors
  */
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors) = 0;

  /*!
  **
  **
  ** @param ostr
  ** @param node
  */
  virtual void saveNode(std::ostream& ostr, const ObjectPtr node) const = 0;

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual ObjectPtr loadNode(std::istream& istr) const = 0;

public:
  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const;

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr);

  /*!
  **
  **
  ** @param nodes
  ** @param ObjectPtr
  ** @param inverseTable
  */
  void enumerateNodes(std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const;

protected:
  /*!
  **
  **
  ** @param node
  ** @param nodes
  ** @param ObjectPtr
  ** @param inverseTable
  */
  void enumerateNodesRec(ObjectPtr node, std::vector<ObjectPtr>& nodes, std::map<ObjectPtr, size_t>& inverseTable) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_GRAPH_H_
