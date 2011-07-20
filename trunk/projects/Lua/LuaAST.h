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

  virtual String toString() const
  {
    String res = tag;
    if (res.isNotEmpty() && (childNodes.size() || variables.size()))
    {
      bool isFirst = true;
      for (size_t i = 0; i < variables.size(); ++i)
      {
        if (isFirst)
        {
          res += T("(");
          isFirst = false;
        }
        else
          res += T(",");
        res += variables[i].toString();
      }
      for (size_t i = 0; i < childNodes.size(); ++i)
      {
        if (isFirst)
        {
          res += T("(");
          isFirst = false;
        }
        else
          res += T(",");
        res += childNodes[i]->toString();
      }
      res += T(")");
    }
    return res;
  }

  virtual String toShortString() const
    {return toString();}

protected:
  friend class LuaASTNodeClass;

  String tag;
  std::vector<Variable> variables;
  std::vector<LuaASTNodePtr> childNodes;
};

extern ClassPtr luaASTNodeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_H_
