/*-----------------------------------------.---------------------------------.
| Filename: XmlTreeView.h                  | Xml Tree View                   |
| Author  : Francis Maes                   |                                 |
| Started : 16/11/2012 12:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_TREE_VIEW_XML_H_
# define LBCPP_USER_INTERFACE_TREE_VIEW_XML_H_

# include "GenericTreeView.h"

namespace lbcpp
{

class XmlTreeView : public GenericTreeView
{
public:
  XmlTreeView(XmlElementPtr xml, const string& name) : GenericTreeView(xml, name)
    {buildTree();}

  // todo: display attributes

  virtual bool mightHaveSubObjects(const ObjectPtr& object)
  {
    XmlElementPtr element = object.staticCast<XmlElement>();
    return !element->isTextElement();
  }

  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object)
  {
    XmlElementPtr element = object.staticCast<XmlElement>();
    std::vector< std::pair<string, ObjectPtr> > res(element->getNumChildElements());
    for (size_t i = 0; i < res.size(); ++i)
    {
      XmlElementPtr elt = element->getChildElement(i);
      res[i] = std::make_pair(elt->isTextElement() ? elt->getText() : elt->getTagName(), elt);
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_TREE_VIEW_XML_H_
