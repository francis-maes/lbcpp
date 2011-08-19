
-- Main experiment script

--require '../ExpressionDiscovery/ReversePolishNotationProblem'
--require '../ExpressionDiscovery/DecisionProblem'
--require '../ExpressionDiscovery/FormulaFeatures'
--require 'Random'
--require 'Evaluator'
require 'AST'

-- Super Feature Function

function formulaAllPathFeatures(formula)

  -- transforms one node into a symbol
  local function nodeSymbol(node)
    local cn = node.className
    if cn == "lua::Return" then
      return '@'
    elseif cn == "lua::LiteralNumber" then
      return node.value
    elseif cn == "lua::Identifier" then
      return node.identifier
    elseif cn == "lua::UnaryOperation" then
      local ops = {"~", "#", "_"}
      return ops[node.op + 1]
    elseif cn == "lua::BinaryOperation" then
      local ops = {"+", "-", "*", "/", 
                   "%", "^", ".", "=",
                   "<", "[", "&", "|"}
      return ops[node.op + 1]
    end
  end

  -- transforms one parent-child link into a symbol (that may be empty)
  local function linkSymbol(parentNode, childNode)
    local commutativeOps = {true, false, true, false,
                            false, false, false, true,
                            false, false, true, true}
    if parentNode.className == "lua::BinaryOperation" and not commutativeOps[parentNode.op + 1] then
      if parentNode:getSubNode(1) == childNode then
        return "L"
      elseif parentNode:getSubNode(2) == childNode then
        return "R"
      else
        assert(false)
      end
    else
      return ""
    end
  end

  -- this function is called for pair of nodes (n_i, n_j) where i <= j
  local function featuresFromTo(sourceStack, destStack, numCommonNodes)
    assert(numCommonNodes <= #sourceStack and numCommonNodes <= #destStack)
    assert(sourceStack[numCommonNodes] == destStack[numCommonNodes])

    -- construct path from source node to destination node
    local path = ""
    for i=#sourceStack,numCommonNodes+1,-1 do
      path = path .. nodeSymbol(sourceStack[i]) .. linkSymbol(sourceStack[i-1], sourceStack[i])
    end
    path = path .. '|' .. nodeSymbol(sourceStack[numCommonNodes]) .. '|'
    for i=numCommonNodes+1,#destStack,1 do
      path = path .. linkSymbol(destStack[i-1], destStack[i]) .. nodeSymbol(destStack[i])
    end

    -- compute reverse path and use lexicographic order to choose the unique name of the path
    local reversePath = string.reverse(path) 
    coroutine.yield(path > reversePath and path or reversePath)
  end

  -- here starts the double loop over nodes (n_i, n_j)
  local sourceIndex = 1
  local destIndex = 1
  local function iterateOverDest(sourceStack, destStack, numCommonNodes)

    if sourceIndex <= destIndex then
      featuresFromTo(sourceStack, destStack, numCommonNodes)
    end

    destIndex = destIndex + 1

    local dest = destStack[#destStack]
    for i=1,dest:getNumSubNodes() do
      table.insert(destStack, dest:getSubNode(i))
      local isCommonNode = destStack[#destStack] == sourceStack[#destStack]
      if isCommonNode then
        numCommonNodes = numCommonNodes + 1
      end

      iterateOverDest(sourceStack, destStack, numCommonNodes)
        
      if isCommonNode then
        numCommonNodes = numCommonNodes - 1
      end
      table.remove(destStack, #destStack)
    end
  end

  local function iterateOverSource(sourceStack)
    destIndex = 1
    iterateOverDest(sourceStack, {formula}, 1)
    sourceIndex = sourceIndex + 1

    local source = sourceStack[#sourceStack]
    for i=1,source:getNumSubNodes() do
      table.insert(sourceStack, source:getSubNode(i))
      iterateOverSource(sourceStack)
      table.remove(sourceStack, #sourceStack)
    end
  end
  -- end of the double loop

  -- add root node 
  formula = AST.returnStatement({formula}) 

  -- launch the double loop
  iterateOverSource({formula}) 

end

--myFormula = AST.binaryOperation('add', AST.binaryOperation('mul', AST.identifier('a'), AST.identifier('b')), AST.identifier('c'))
myFormula = AST.binaryOperation('div',
   AST.binaryOperation('sub', AST.identifier('a'), AST.identifier('b')),
   AST.binaryOperation('mod', AST.identifier('a'), AST.identifier('c')))
--myFormula = AST.binaryOperation('mod', AST.identifier('a'), AST.identifier('b'))
 
print (myFormula:print())
context:enter("computing features")
iterator = coroutine.wrap(| | formulaAllPathFeatures(myFormula))
for feature, value in iterator do
  print(feature)--, value or 1.0)
end
context:leave()


--[[

--context.randomGenerator = Random.new(0)


-- decorate objective function
local trainingDataset = {}
local testingDataset = {}
local allExamples = {}

local numEvaluations = 0
local function decorateObjectiveFunction(objective)
  return function (candidate, candidateDescription, candidateAST)
    local score = allExamples[candidateDescription]
    if score == nil then
      score = objective(candidate)
      --print ("Evaluate " .. candidateDescription .. " ==> " .. score)
      local dataset = numEvaluations % 2 == 0 and trainingDataset or testingDataset
      numEvaluations = numEvaluations + 1
      table.insert(dataset, {candidateAST, score})
      allExamples[candidateDescription] = score
    end
    return score
  end
end

-- Application : symbolic regression

local function makeSymbolicRegressionObjective()
  local dataset = {}
  for i = 1,100 do
    local example = {
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian(), 
      Stochastic.standardGaussian()
    }
    --table.insert(example, example[1] * example[2] + example[3])  -- a b * c +
    table.insert(example, (example[1] + 5) * example[2] + example[3] - 10) -- a * b + b * 5 + c - 10
    table.insert(dataset, example)
  end
  return |f| Evaluator.meanSquaredError(f, dataset)
end


problem = DecisionProblem.ReversePolishNotation{
  constants = {1,2,5,10},
  objective = decorateObjectiveFunction(makeSymbolicRegressionObjective()),
  maxSize = 10
}.__get


-- Generate Datasets

while #testingDataset < 100 do
  DecisionProblem.randomEpisode(problem, x)
end

table.sort(trainingDataset, |a,b| a[2] < b[2])
table.sort(testingDataset, |a,b| a[2] < b[2])

for i,example in ipairs(trainingDataset) do
  print(example[1]:print(), example[2])
  if i == 25 then break end
end

]]