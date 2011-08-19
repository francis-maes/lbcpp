-- Francis Maes, 11/08/2011
-- Abstract syntax tree

AST = {}


local function makeObjectVector(class, nodes, startIndex)
  local res = lbcpp.Object.create('ObjectVector<' .. class .. '>')
  for i = startIndex or 1, #nodes do
    res:append(nodes[i])
  end
  return res
end


--
-- Statements
--

function AST.returnStatement(expressions)
  if expressions == nil then
    return lbcpp.Object.create("lua::Return")
  else
    return lbcpp.Object.create("lua::Return", makeObjectVector("lua::Expression", expressions))
  end
end

--
-- Expressions
--

function AST.literalNumber(number)
  return lbcpp.Object.create("lua::LiteralNumber", number)
end

function AST.literalString(str)
  return lbcpp.Object.create("lua::LiteralString", str)
end

function AST.identifier(str)
  return lbcpp.Object.create("lua::Identifier", str)
end

function AST.unaryOperation(op, expr)
  local function convert(opid)
    opids = {['not'] = 0, len = 1, unm = 2}
    return opids[opid]
  end
  return lbcpp.Object.create("lua::UnaryOperation", convert(op), expr)
end

function AST.binaryOperation(op, left, right)
  local function convertBinaryOp(opid)
    opids = {add = 0, sub = 1, mul = 2, div = 3,
            mod = 4, pow = 5, concat = 6, eq = 7,
            lt = 8, le = 9, ['and'] = 10, ['or'] = 11}
    return opids[opid]
  end
  return lbcpp.Object.create("lua::BinaryOperation", convertBinaryOp(op), left, right)
end

return AST