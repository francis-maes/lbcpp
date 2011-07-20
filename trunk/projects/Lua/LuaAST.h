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
  LuaASTNode(const String& tag, const std::vector<Variable>& variables, const std::vector<LuaASTNodePtr>& childNodes)
    : tag(tag), variables(variables), childNodes(childNodes) {}
  LuaASTNode(const String& tag, const std::vector<LuaASTNodePtr>& childNodes)
    : tag(tag), childNodes(childNodes) {}
  LuaASTNode(const std::vector<LuaASTNodePtr>& childNodes)
    : childNodes(childNodes) {}
  LuaASTNode(const String& rawCode)
    : tag(T("Raw")), variables(1, rawCode) {}
  LuaASTNode() {}

  static LuaASTNodePtr newOp(const String& opid, const LuaASTNodePtr& operand)
  {
    std::vector<LuaASTNodePtr> childNodes(1);
    childNodes[0] = operand;
    return new LuaASTNode(T("Op"), std::vector<Variable>(1, opid), childNodes);
  }

  static LuaASTNodePtr newOp(const String& opid, const LuaASTNodePtr& left, const LuaASTNodePtr& right)
  {
    std::vector<LuaASTNodePtr> childNodes(2);
    childNodes[0] = left;
    childNodes[1] = right;
    return new LuaASTNode(T("Op"), std::vector<Variable>(1, opid), childNodes);
  }

  static LuaASTNodePtr newParen(const LuaASTNodePtr& content)
    {return new LuaASTNode(T("Paren"), std::vector<LuaASTNodePtr>(1, content));}

  static LuaASTNodePtr newNumber(double value)
    {return new LuaASTNode(T("Number"), std::vector<Variable>(1, value), std::vector<LuaASTNodePtr>());}

  const String& getTag() const
    {return tag;}

  size_t getNumChildNodes() const
    {return childNodes.size();}

  LuaASTNodePtr getChildNode(size_t index) const
    {jassert(index < childNodes.size()); return childNodes[index];}

  const std::vector<LuaASTNodePtr>& getChildNodes() const
    {return childNodes;}

  size_t getNumArguments() const
    {return variables.size();}

  Variable getArgument(size_t index) const
    {return variables[index];}

  const std::vector<Variable>& getArguments() const
    {return variables;}

  virtual String toString() const
  {
    String res = tag;
    if (tag.isNotEmpty())
      res += T("(");

    bool needComma = false;
    for (size_t i = 0; i < variables.size(); ++i)
    {
      if (needComma)
        res += T(",");
      res += variables[i].toString();
      needComma = true;
    }
    for (size_t i = 0; i < childNodes.size(); ++i)
    {
      if (needComma)
        res += T(",");
      res += childNodes[i]->toString();
      needComma = true;
    }

    if (tag.isNotEmpty())
      res += T(")");
    return res;
  }

  virtual String toShortString() const
    {return toString();}

  String print() const;

protected:
  friend class LuaASTNodeClass;

  String tag;
  std::vector<Variable> variables;
  std::vector<LuaASTNodePtr> childNodes;
};

extern ClassPtr luaASTNodeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_AST_H_
