
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


local function convertUnaryOp(opid)
  opids = {['not'] = 0, len = 1, unm = 2}
  return opids[opid]
end

local function convertBinaryOp(opid)
  opids = {add = 0, sub = 1, mul = 2, div = 3,
           mod = 4, pow = 5, concat = 6, eq = 7,
           lt = 8, le = 9, ['and'] = 10, ['or'] = 11}
  return opids[opid]
end


function _M.metaLuaAstToLbcppAst(ast)

  local function convertSubNodes(ast)
    local res = {}
    for i=1,#ast do
      if type(ast[i]) == "table" then
        table.insert(res, _M.metaLuaAstToLbcppAst(ast[i]))
      end
    end
    return res
  end

  local function makeObjectVector(class, inputTable, startIndex)
    local res = Object.create('ObjectVector<' .. class .. '>')
    for i=startIndex,#inputTable do
      res:append(inputTable[i])
    end
    return res
  end

  local function makeBlock(subAst)
    local statements = convertSubNodes(subAst)
    for i=1,#statements do
      if statements[i].className == "lua::Call" then
        local f = statements[i]["function"]
        local g = statements[i].arguments
        statements[i] = Object.create("lua::CallStatement", f, g)
      end
    end
    return Object.create("lua::Block", makeObjectVector("lua::Statement", statements, 1))
  end

  local function getTag(ast)
    return ast.tag and ast.tag or "List"
  end

  if ast==nil then
    return nil
  end

  -- convert sub nodes  
  local subNodes = convertSubNodes(ast)
   
  -- create this node
  if ast.tag == nil then
    return Object.create("lua::List", makeObjectVector("lua::Node", subNodes, 1))
  end

  --print ("tag: " .. ast.tag .. " num sub nodes = " .. (#subNodes))

  -- statements
  if ast.tag == "Do" then
    return Object.create("lua::" .. ast.tag, subNodes[1])
  elseif ast.tag == "Set" then
    return Object.create("lua::Set", subNodes[1], subNodes[2])
  elseif ast.tag == "While" then
    --print ("While: num sub nodes " .. #subNodes .. " subtags = " .. getTag(ast[1]) .. " " .. getTag(ast[2]))
    return Object.create("lua::While", subNodes[1], makeBlock(ast[2]))
  elseif ast.tag == "Return" then
    return Object.create("lua::Return", makeObjectVector("lua::Expression", subNodes, 1))
  
  -- expressions  
  elseif ast.tag == "Nil" or ast.tag == "False" or ast.tag == "True" or ast.tag == "False" then
    return Object.create("lua::" .. ast.tag)
  elseif ast.tag == "Id" then
    return Object.create("lua::Identifier", ast[1])
  elseif ast.tag == "Derivable" then
    return Object.create("lua::Identifier", ast[1][1], true)
  elseif ast.tag == "Number" or ast.tag == "String" then
    return Object.create("lua::Literal" .. ast.tag, ast[1])
  elseif ast.tag == "Function" or ast.tag == "Index" then
    return Object.create("lua::" .. ast.tag, subNodes[1], makeBlock(ast[2]))
  elseif ast.tag == "Table" then
    return Object.create("lua::Table", makeObjectVector("lua::Expression", subNodes, 1))
  elseif ast.tag == "Pair" then
    return Object.create("lua::Pair", subNodes[1], subNodes[2])
  elseif ast.tag == "Op" then
    if #subNodes == 2 then
      return Object.create("lua::BinaryOperation", convertBinaryOp(ast[1]), subNodes[1], subNodes[2])
    elseif #subNodes == 1 then
      return Object.create("lua::UnaryOperation", convertUnaryOp(ast[1]), subNodes[1])
    else
      error("invalid number of operands: " .. #subNodes)
    end
  elseif ast.tag == "Paren" then
    return Object.create("lua::Parenthesis", subNodes[1])
  elseif ast.tag == "Call" then
    return Object.create("lua::Call", subNodes[1], makeObjectVector("lua::Expression", subNodes, 2))
  else
    error("unknown tag " .. ast.tag .. " (numSubNodes = " .. #subNodes .. " numAttributes = " .. (#ast - #subNodes) .. ")")
  end
end

function _M.oldMetaLuaAstToLbcppAst(ast)
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
    -- transform result into block
    local list = _M.metaLuaAstToLbcppAst(_M.parseStatementBlock(lexer))
    local res = Object.create("lua::Block")
    res.statements = list.nodes
    return res
  else
    error("Unknown code type")
  end
end

function _M.parseFromString(codeType, code, codeName)
  local lexer = _M.stringLexer(code, codeName)
  return _M.parse(codeType, lexer)
end

-- 'derivable' extension
DerivableFunction = {__call = function (tbl, ...) return tbl.f(...) end} -- metatable

mlp.lexer:add{ "derivable" }
mlp.func_param_content:add{ "derivable", mlp.id, builder = "Derivable" }
