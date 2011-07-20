
-- note: includes gg/mlp Lua parsing Libraries taken from Metalua.
require "lexer"
require "gg"
require "mlp_lexer"
require "mlp_misc"
require "mlp_table"
require "mlp_meta"
require "mlp_expr"
require "mlp_stat"
require "mlp_ext"
mlc = {} -- make gg happy
local mlp = assert(_G.mlp)

module ("LuaChunk", package.seeall)

_M.parseExpression = mlp.expr
_M.parseStatement = mlp.stat
_M.parseStatementBlock = mlp.block

function _M.stringLexer(src, filename)
  return mlp.lexer:newstream(src, filename)
end

function _M.fileLexer(filename)
  -- FIXME return mlp.lexer:newstream(src, filename)
end

function _M.metaLuaAstToLbcppAst(ast)
  if (ast) then
    local res = Object.create("LuaASTNode")

    if (ast.tag) then
      res.tag = ast.tag
    end
    local childNodes = res.childNodes
    local variables = res.variables
    
    for i=1, # ast do
      local var = ast[i]
      if (type(var) == "table") then
        childNodes:append(_M.metaLuaAstToLbcppAst(ast[i]))
      else
        variables:append(ast[i])
      end
    end
    
    res.childNodes = childNodes
    res.variables = variables
    return res
  else
    return nil
  end
end

function _M.parse(codeType, lexer)
  if (codeType == 0) then
    return _M.metaLuaAstToLbcppAst(_M.parseExpression(lexer))
  elseif (codeType == 1) then
    return _M.metaLuaAstToLbcppAst(_M.parseStatement(lexer))
  elseif (codeType == 2) then
    return _M.metaLuaAstToLbcppAst(_M.parseStatementBlock(lexer))
  else
    error("Unknown code type")
  end
end

function _M.parseFromString(codeType, code, codeName)
  local lexer = _M.stringLexer(code, codeName)
  return _M.parse(codeType, lexer)
end
