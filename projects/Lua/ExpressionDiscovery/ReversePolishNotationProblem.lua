-- Francis Maes, 12/08/2011
-- Reverse Polish Notation Decision Problem

require '../ExpressionDiscovery/DecisionProblem'
require 'AST'
require 'Vector'

-- Transforms a formula represented through its abstract syntax tree
--  into an executable function
function buildExpression(variables, expression)
  local code = "return function ("
  for j,variable in ipairs(variables) do
    code = code .. variable
    if j < #variables then
      code = code .. ","
    end
  end
  code = code .. ") return " .. expression:print() .. " end"
  --print (code)
  chunk,err = loadstring(code)
  if not chunk then
    error(err)
  end
  return chunk()
end

local function expressionSize(expression)
  local res = 1
  local n = expression:getNumSubNodes()
  for i=1,n do
    res = res + expressionSize(expression:getSubNode(i))
  end
  return res
end

local function stackOrExpressionSize(x)
  if type(x) == "table" then
    local res = 0
    for i,expression in ipairs(x) do
      res = res + expressionSize(expression)
    end
    return res
  else
    return expressionSize(x)     
  end
end

-- Reverse polish notation Sequential Decision Process

DecisionProblem.ReversePolishNotation = subspecified {

  parameter variables = {default={"a", "b", "c", "d"}},
  parameter constants = {default={1}},
  parameter unaryOperations = {default={"unm"}},
  parameter binaryOperations = {default={"add", "sub", "mul", "div"}},
  parameter objective = {},
  parameter maxSize = {default = 0, min = 0},

  x0 = {},
  discount = 1,

  isFinal = function (stack)
    return type(stack) ~= "table"
  end,

  -- Transition Function
  f = function (stack, ast)

    if ast.className == "lua::Return" then
      assert (#stack > 0)
      return stack[#stack]
    end

    local n = ast:getNumSubNodes()
    local m = #stack -- stack size
    assert(n <= m)
    local res = {}

    -- copy m-n first elements (pop n elements)
    for i=1,m-n do  
      res[i] = stack[i] 
    end

    -- fill operands using the popped elements
    if n > 0 then
      ast = ast:clone()
      for i=1,n do
        ast:setSubNode(i, stack[m-n+i])
      end
    end
   
    -- insert ast
    table.insert(res, ast)
    
    -- return new stack
    return res

  end,

  -- Available actions function
  U = function (stack)
    if type(stack) ~= "table" then
      return nil -- this is a final state
    end
    local ms = maxSize -- FIXME: there is a bug in the lua here !!! 
    local minArity = 0
    local maxArity = #stack
    if ms > 0 then
      local s = stackOrExpressionSize(stack)
      if s >= ms then
        return {AST.returnStatement()} -- maxSize is reached, must finish now
      end
      local remaining = ms - s
      minArity = #stack + 1 - remaining
    end

    local res = {}
    if minArity <= 2 and 2 <= maxArity then
      for i,v in ipairs(binaryOperations) do
        table.insert(res, AST.binaryOperation(v))
      end
    end
    if minArity <= 1 and 1 <= maxArity then
      for i,v in ipairs(unaryOperations) do
        table.insert(res, AST.unaryOperation(v))
      end
    end
    if minArity <= 0 and 0 <= maxArity then
      for i,v in ipairs(variables) do
        table.insert(res, AST.identifier(v))
      end
      for i,c in ipairs(constants) do
        table.insert(res, AST.literalNumber(c))
      end
    end
    if 1 <= maxArity then
      table.insert(res, AST.returnStatement())
    end
    return res
  end,

  -- Reward function
  g = function (stack, ast)
    if ast.className == "lua::Return" then
      assert (#stack > 0)
      local ast = stack[#stack]
      local f = buildExpression(variables, ast)
      local o = objective(f, ast:print())
      return math.exp(-o)
    else
      return 0
    end
  end,

  -- Features
  actionFeatures = function (x,u)
    return Vector.newSparse()
  end,

  -- String descriptions
  stateToString = function (x)
    if type(x) == "table" then
      local res = "{"
      for i,f in ipairs(x) do
        res = res .. f:print()
        if i < #x then
          res = res .. ", "
        end
      end
      return res .. "}"
    else
      return x:print()
    end
  end,

  actionToString = function (u)
    if u.className == "lua::UnaryOperation" then
      local strings = {"not", "len", "unm"}
      return strings[u.op + 1]
    elseif u.className == "lua::BinaryOperation" then
      local strings = {"add", "sub", "mul", "div",
                       "mod", "pow", "concat", "eq",
                       "lt", "le", "and", "or"}
      return strings[u.op + 1]
    else
      return u:print()
    end
  end
}