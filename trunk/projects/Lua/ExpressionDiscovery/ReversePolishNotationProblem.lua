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

function stackOrExpressionSize(x)
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

function simplifyAndMakeFormulaUnique(formula)
  local function getLiteralNumber(formula)
    return formula.className == "lua::LiteralNumber" and formula.value or nil
  end
  local function isUnm(formula)
    return formula.className == "lua::UnaryOperation" and AST.getUnaryOperationOp(formula) == "unm"
  end
  local function isDiv(formula)
    return formula.className == "lua::BinaryOperation" and AST.getBinaryOperationOp(formula) == "div"
  end

  local n = formula:getNumSubNodes()

  if n == 0 then
    return formula
  elseif n == 1 then
    local subFormula = simplifyAndMakeFormulaUnique(formula:getSubNode(1))
    local number = getLiteralNumber(subFormula)
    
    if isUnm(formula) then
      -- simplify constant expression
      if number ~= nil then
        return AST.literalNumber(-number)
      end
      -- Simplify - - x into x
      if isUnm(subFormula) then
        return subFormula:getSubNode(1)
      end
    end

    return AST.unaryOperation(AST.getUnaryOperationOp(formula), subFormula)

  elseif n == 2 then
    local op = AST.getBinaryOperationOp(formula)
    local subFormula1 = simplifyAndMakeFormulaUnique(formula:getSubNode(1))
    local subFormula2 = simplifyAndMakeFormulaUnique(formula:getSubNode(2))
    local subFormulaStr1 = subFormula1:print()
    local subFormulaStr2 = subFormula2:print()

    -- break commutativity
    if AST.isBinaryOperationCommutative(formula) and subFormulaStr1 > subFormulaStr2 then
      local tmp = subFormula1
      subFormula1 = subFormula2
      subFormula2 = tmp
      tmp = subFormulaStr1
      subFormulaStr1 = subFormulaStr2
      subFormulaStr2 = tmp
    end

    local number1 = getLiteralNumber(subFormula1)
    local number2 = getLiteralNumber(subFormula2)

    if op == "add" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 + number2)
      end
      -- Simplify 0 + x into x
      if number1 == 0 then
        return subFormula2
      end
    elseif op == "sub" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 - number2)
      end
      -- Simplify x - x into 0
      if subFormulaStr1 == subFormulaStr2 then  
        return AST.literalNumber(0)
      end
      -- Simplify x - 0 into x
      if number2 == 0 then
        return subFormula1
      end
    elseif op == "mul" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 * number2)
      end
      -- Simplify 1 * x into x
      if number1 == 1 then
        return subFormula2
      end      
    elseif op == "div" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 / number2)
      end
      -- Simplify x / 1 into x
      if number2 == 1 then
        return subFormula1
      end
      -- Simplify x / x into 1
      if subFormulaStr1 == subFormulaStr2 then  
        return AST.literalNumber(1)
      end
      -- Simplfy (a/b)/(c/d) into (a*d)/(b*c)
      if isDiv(subFormula1) and isDiv(subFormula2) then
        return simplifyAndMakeFormulaUnique(AST.binaryOperation("div",
                   AST.binaryOperation("mul", subFormula1:getSubNode(1), subFormula2:getSubNode(2)),
                   AST.binaryOperation("mul", subFormula1:getSubNode(2), subFormula2:getSubNode(1))))
      end
      -- Simplify (a/b)/c into a / (b*c)
      if isDiv(subFormula1) then
        return simplifyAndMakeFormulaUnique(AST.binaryOperation("div",
                   subFormula1:getSubNode(1),
                   AST.binaryOperation("mul", subFormula1:getSubNode(2), subFormula2)))
      end
      -- Simplify a / (b/c) into (a*c)/b
      if isDiv(subFormula2) then
        return simplifyAndMakeFormulaUnique(AST.binaryOperation("div",
                   AST.binaryOperation("mul", subFormula1, subFormula2:getSubNode(2)),
                   subFormula2:getSubNode(1)))
      end
    end    

    return AST.binaryOperation(op, subFormula1, subFormula2)
  end  
end

function formulaToTrajectory(formula)
  local x = {}
  local res = {x}
  
  local function buildTrajectory(node)
    local n = node:getNumSubNodes()
    -- Randomize order in case of commutative operators
    if n == 2 and AST.isBinaryOperationCommutative(node) and context:random() < 0.5 then
      buildTrajectory(node:getSubNode(2))
      buildTrajectory(node:getSubNode(1))
    else
      for i=1,n do  
        buildTrajectory(node:getSubNode(i))
      end
    end

    local u = node:clone()
    assert(u)
    table.insert(res, u) -- action
    x = problem.f(x, u)
    assert(x)
    table.insert(res, x) -- state
  end
  
  buildTrajectory(formula)
  table.insert(res, AST.returnStatement())

  --local dbg = formula:print() .. " <=="
  --for i=1,#res/2 do
  --  dbg = dbg .. " x=" .. problem.stateToString(res[i*2-1]) .. " u=" .. problem.actionToString(res[i*2])
  --end
  --print (dbg)

  return res
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
      local o = objective(f, ast:print(), ast)
      return math.exp(-o)
    else
      return 0
    end
  end,

  -- String descriptions
  stateToString = function (x)
    if x == nil then
      return "NIL"
    elseif type(x) == "table" then
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
    assert(u)
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