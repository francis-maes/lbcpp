-- note: includes gg/mlp Lua parsing Libraries taken from Metalua.

-- interface:
--
-- parseFromString(codeType, code, codeName)
-- parseFromFile(codeType, filename)

require "Language.lexer"
require "Language.gg"
require "Language.mlp_lexer"
require "Language.mlp_misc"
require "Language.mlp_table"
require "Language.mlp_meta"
require "Language.mlp_expr"
require "Language.mlp_stat"
require "Language.mlp_ext"
require "AST"

mlc = {} -- make gg happy
local mlp = assert(_G.mlp)

-- error handler
if __errorHandler == nil then
  __errorHandler = function (msg)
    context:error(msg)
    return msg
  end
end

module ("LuaChunk", package.seeall)

local function setLineInfoInNode(node, ali)
  local function convertLineInfo(lineinfo)
    if lineinfo then
--      local comments = (lineinfo.comments and lineinfo.comments or "") -- comments shoult be concatenated
      return lbcpp.Object.create("lua::LineInfo", lineinfo[1], lineinfo[2], lineinfo[4])
    else
      return nil
    end
  end
  node:setLineInfo(convertLineInfo(ali and ali.first), convertLineInfo(ali and ali.last))
end

local function makeObjectVector(class, inputTable, startIndex)
  local res = lbcpp.Object.create('ObjectVector<' .. class .. '>')
  for i=startIndex,#inputTable do
    res:append(inputTable[i])
  end
  return res
end

local function makeSubArray(array, startIndex)
  local res = {}
  for i=startIndex,#array do
    table.insert(res, array[i])
  end
  return res
end

local function createBlock(statements)

  local function ensureIsStatement(node)
    local cl = node.className
    if cl == "lua::Call" or cl == "lua::Invoke" then
      return lbcpp.Object.create("lua::ExpressionStatement", node)
    else
      return node
    end
  end

  for i=1,#statements do
    statements[i] = ensureIsStatement(statements[i])
  end
  return lbcpp.Object.create("lua::Block", makeObjectVector("lua::Statement", statements, 1))
end

local metaLuaAstToLbcppAst

local function metaLuaAstToLbcppAstInternal(ast)

  local function convertSubNodes(ast)
    local res = {}
    for i=1,#ast do
      if type(ast[i]) == "table" then
        table.insert(res, metaLuaAstToLbcppAst(ast[i]))
      end
    end
    return res
  end


  local function makeBlock(subAst)
    return createBlock(convertSubNodes(subAst))
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
    return lbcpp.Object.create("lua::List", makeObjectVector("lua::Node", subNodes, 1))
  end

  --print ("tag: " .. ast.tag .. " num sub nodes = " .. (#subNodes))

  -- statements
  if ast.tag == "Do" then
    return lbcpp.Object.create("lua::" .. ast.tag, subNodes[1])
  elseif ast.tag == "Set" then
    return lbcpp.Object.create("lua::Set", subNodes[1], subNodes[2])
  elseif ast.tag == "While" then
    --print ("While: num sub nodes " .. #subNodes .. " subtags = " .. getTag(ast[1]) .. " " .. getTag(ast[2]))
    return lbcpp.Object.create("lua::While", subNodes[1], makeBlock(ast[2]))
  elseif ast.tag == "Repeat" then
    return lbcpp.Object.create("lua::Repeat", makeBlock(ast[1]), subNodes[2])
  elseif ast.tag == "If" then
    local conditions = lbcpp.Object.create("ObjectVector<lua::Expression>")
    local blocks = lbcpp.Object.create("ObjectVector<lua::Block>")
    local i = 1
    assert (#subNodes == #ast)
    while i <= #subNodes do
      if i < #subNodes then
        conditions:append(subNodes[i])
        blocks:append(makeBlock(ast[i + 1]))
      else
        blocks:append(makeBlock(ast[i]))
      end
      i = i + 2
    end
    return lbcpp.Object.create("lua::If", conditions, blocks)
  elseif ast.tag == "ForNum" then
    if #subNodes == 5 then
      return lbcpp.Object.create("lua::ForNum", subNodes[1], subNodes[2], subNodes[3], subNodes[4], makeBlock(ast[5]))
    else
      assert (#subNodes == 4)
      return lbcpp.Object.create("lua::ForNum", subNodes[1], subNodes[2], subNodes[3], nil, makeBlock(ast[4]))
    end
  elseif ast.tag == "ForIn" then
    return lbcpp.Object.create("lua::ForIn", subNodes[1], subNodes[2], makeBlock(ast[3]))
  elseif ast.tag == "Local" then
    if #subNodes == 2 then
      return lbcpp.Object.create("lua::Local", subNodes[1], subNodes[2])
    else
      assert(#subNodes == 1)
      return lbcpp.Object.create("lua::Local", subNodes[1])
    end
  elseif ast.tag == "Localrec" then
    assert (#subNodes == 2)
    return lbcpp.Object.create("lua::Local", subNodes[1], subNodes[2], true)
  elseif ast.tag == "Break" then
    return lbcpp.Object.create("lua::Break");
  elseif ast.tag == "Return" then
    return AST.returnStatement(subNodes)
  elseif ast.tag == "Parameter" then
    return lbcpp.Object.create("lua::Parameter", subNodes[1], subNodes[2])
  
  -- expressions  
  elseif ast.tag == "Nil" or ast.tag == "Dots" then
    return lbcpp.Object.create("lua::" .. ast.tag)
  elseif ast.tag == "False" or ast.tag == "True" then
    return lbcpp.Object.create("lua::LiteralBoolean", ast.tag == "True")
  elseif ast.tag == "Id" then
    return AST.identifier(ast[1])
  elseif ast.tag == "Derivable" then
    return lbcpp.Object.create("lua::Identifier", ast[1][1], true)
  elseif ast.tag == "Number" then
    return AST.literalNumber(ast[1])
  elseif ast.tag == "String" then
    return AST.literalString(ast[1])
  elseif ast.tag == "Function" then
    return lbcpp.Object.create("lua::Function", subNodes[1], makeBlock(ast[2]))
  elseif ast.tag == "Table" then
    return lbcpp.Object.create("lua::Table", makeObjectVector("lua::Expression", subNodes, 1))
  elseif ast.tag == "Pair" then
    return lbcpp.Object.create("lua::Pair", subNodes[1], subNodes[2])
  elseif ast.tag == "Op" then
    if #subNodes == 2 then
      return AST.binaryOperation(ast[1], subNodes[1], subNodes[2])
    elseif #subNodes == 1 then
      return AST.unaryOperation(ast[1], subNodes[1])
    else
      error("invalid number of operands: " .. #subNodes)
    end
  elseif ast.tag == "Paren" then
    return lbcpp.Object.create("lua::Parenthesis", subNodes[1])
  elseif ast.tag == "Call" then
    return AST.call(subNodes[1], makeSubArray(subNodes, 2))
  elseif ast.tag == "Invoke" then
    return lbcpp.Object.create("lua::Invoke", subNodes[1], subNodes[2], makeObjectVector("lua::Expression", subNodes, 3))
  elseif ast.tag == "Index" then
    return lbcpp.Object.create("lua::Index", subNodes[1], subNodes[2])
  elseif ast.tag == "Subspecified" then
    return lbcpp.Object.create("lua::Subspecified", subNodes[1])
  else
    error("unknown tag " .. ast.tag .. " (numSubNodes = " .. #subNodes .. " numAttributes = " .. (#ast - #subNodes) .. ")")
  end
end

metaLuaAstToLbcppAst = function(ast)
  local res = metaLuaAstToLbcppAstInternal(ast)
  setLineInfoInNode(res, ast.lineinfo)
  return res
end

local function parse(codeType, lexer)
  if (codeType == 0) then
    return metaLuaAstToLbcppAst(mlp.expr(lexer))
  elseif (codeType == 1) then
    return metaLuaAstToLbcppAst(mlp.stat(lexer))
  elseif (codeType == 2) then
    -- transform result into block
    local list = metaLuaAstToLbcppAst(mlp.block(lexer))
    return createBlock(list.nodes)
  else
    error("Unknown code type")
  end
end

function _M.parseFromString(codeType, code, codeName)
  local lexer = mlp.lexer:newstream(code, codeName)
  return parse(codeType, lexer)
end

function _M.parseFromFile(codeType, filename)
  local f = assert(io.open(filename, "r"))
  local code = f:read("*all")
  f:close()
  return _M.parseFromString(codeType, code, filename)
end

----- Extend the AST interface
function AST.parseExpressionFromString(string, chunkName)
  return LuaChunk.parseFromString(0, string, chunkName)
end

-------

-- 'derivable' extension
mlp.lexer:add{ "derivable" }
mlp.func_param_content:add{ "derivable", mlp.id, builder = "Derivable" }

-- 'subspecified' extension
mlp.lexer:add{ "subspecified", "parameter" }
mlp.expr:add{ "subspecified", mlp.expr, builder = "Subspecified" }
   
local function subspecified_funcdef_builder(x)
   local name, method, func = x[1], x[2], x[3]
   if method then 
      name = { tag="Index", name, method, lineinfo = {
         first = name.lineinfo.first,
         last  = method.lineinfo.last } }
      _G.table.insert (func[1], 1, {tag="Id", "self"}) 
   end
   func = {tag = "Subspecified", func}
   local r = { tag="Set", {name}, {func} } 
   r[1].lineinfo = name.lineinfo
   r[2].lineinfo = func.lineinfo
   return r
end 

mlp.stat:add{"subspecified", "function", mlp.func_name, mlp.method_name, mlp.func_val, builder=subspecified_funcdef_builder }

local parameter_declaration = gg.sequence{ "parameter", mlp.id, "=", mlp.table, builder = "Parameter" }
mlp.stat:add(parameter_declaration) -- { "parameter", mlp.id, "=", mlp.table, builder = "Parameter" }

local traditional_table_field = mlp.table_field
function mlp.table_field (lx)
   if lx:is_keyword (lx:peek(), "parameter") then
     return parameter_declaration (lx)
   else
     return traditional_table_field (lx)
   end
end