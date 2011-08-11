-- Upper Bound Open Loop Action Search -- 

require 'AST'
require 'Stochastic'


-- Reverse polish notation Sequential Decision Process

local RPNDecisionProblem = subspecified {

  parameter variables = {default={"a", "b", "c", "d"}},
  parameter unaryOperations = {default={"unm"}},
  parameter binaryOperations = {default={"add", "sub", "mul", "div"}},

  x0 = {},

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

  isFinal = function (stack)
    return type(stack) ~= "table"
  end,

  -- Available actions function
  U = function (stack)
    if type(stack) ~= "table" then
      return nil -- this is a final state
    end
    local res = {}
    local m = #stack
    if m >= 2 then
      for i,v in ipairs(binaryOperations) do
        table.insert(res, AST.binaryOperation(v))
      end
    end
    if m >= 1 then
      for i,v in ipairs(unaryOperations) do
        table.insert(res, AST.unaryOperation(v))
      end
      table.insert(res, AST.returnStatement())
    end
    for i,v in ipairs(variables) do
      table.insert(res, AST.identifier(v))
    end
    return res
  end
}

-- Test

problem = RPNDecisionProblem{}

local function randomEpisode(x0)
  local x = x0 or problem.x0
  while not problem.isFinal(x) do
    local U = problem.U(x)
    assert(#U > 0) 
    local u = U[Stochastic.uniformInteger(1, #U)]
    x = problem.f(x, u)
  end
  return x  
end


for i=1,10 do
  print (randomEpisode():print())
end

-------------------------

subspecified function UbolaSearch(x0, f)

  parameter numEpisodes = {default = 10000}
  parameter numEpisodesPerIteration = {default = 100}

  local episodeNumber = 1

  local function episode()

    

  end

  local numIterations = math.ceil(numEpisodes / numEpisodesPerIteration)
  for iter=1,numIterations do
    context:enter("Episodes " .. episodeNumber .. " -- " .. (episodeNumber + numEpisodesPerIteration - 1))
    for e=1,numEpisodesPerIteration do
      episode()
      episodeNumber = episodeNumber + 1
    end
    context:leave()
  end
end

local x0 = {}
local f = nil

local ubola = UbolaSearch{}
--ubola(x0, f)