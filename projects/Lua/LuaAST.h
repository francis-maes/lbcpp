/*-----------------------------------------.---------------------------------.
| Filename: LuaAST.h                       | Lua AST                         |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 12:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_AST_H_
# define LBCPP_LUA_AST_H_

# include <lbcpp/Lua/Lua.h>

namespace lbcpp
{

class LuaASTNode;
typedef ReferenceCountedObjectPtr<LuaASTNode> LuaASTNodePtr;

class LuaASTNode : public Object
{
public:
  const String& getTag() const
    {return tag;}

  size_t getNumChildNodes() const
    {return childNodes.size();}

  LuaASTNodePtr getChildNode(size_t index) const
    {jassert(index < childNodes.size()); return childNodes[index];}

protected:
  friend class LuaASTNodeClass;

  String tag;
  std::vector<LuaASTNodePtr> childNodes;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_H_
