-- Formula Features

require 'AST'
require 'Vector'
require 'Dictionary'

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

allPathFeaturesDictionary = Dictionary.new()
function formulaAllPathFeaturesToSparseVector(formula)
  local res = Vector.newSparse()
  local iterator = coroutine.wrap(| | formulaAllPathFeatures(formula))
  for feature, value in iterator do
    res:increment(allPathFeaturesDictionary:add(feature), value or 1.0)
  end
  return res
end

-----------------------------------------------------------------------
-----------------------------------------------------------------------
-----------------------------------------------------------------------

-- Formula Features
function formulaStringFeatures(ast)

  local function makeTerminalFeatures(ast)
    local cn = ast.className
    if cn == "lua::LiteralNumber" then
      return {isNumber = 1, ["number=" .. ast.value] = 1}
    elseif cn == "lua::Identifier" then
      return {isIdentifier = 1, ["identifier=" .. ast.identifier] = 1}
    elseif cn == "lua::UnaryOperation" then
      local ops = {"not", "len", "unm"}
      return ops[ast.op + 1]
    elseif cn == "lua::BinaryOperation" then
      local ops = {"add", "sub", "mul", "div", 
                   "mod", "pow", "concat", "eq",
                   "lt", "le", "and", "or"}
      return ops[ast.op + 1]
    end
  end

  local function featureTimesFeatures(feature, features)
    local res = {}
    for name,value in pairs(features) do
      res[feature .. '(' .. name .. ')'] = value
    end
    return res
  end

  local function featureTimesFeaturesTimesFeatures(feature, features1, features2)
    local res = {}
    for name1,value1 in pairs(features1) do
      for name2,value2 in pairs(features2) do
        res[feature .. '(' .. name1 .. ' && ' .. name2 .. ')'] = value1 * value2
      end
    end
    return res
  end

  local function featureTimesFeaturesTimesFeaturesCommutative(feature, features1, features2)
    local res = {}
    for name1,value1 in pairs(features1) do
      for name2,value2 in pairs(features2) do
        local conj
        if name1 < name2 then
          conj = name1 .. ' && ' .. name2
        else
          conj = name2 .. ' && ' .. name1
        end
        res[feature .. '(' .. conj .. ')'] = value1 * value2
      end
    end
    return res
  end

  local function makeCompositeFeatures(ast)
    local tf = makeTerminalFeatures(ast)
    local n = ast:getNumSubNodes()
    local res
    if n == 0 then
      res = tf
    elseif n == 1 then
      local subFeatures = makeCompositeFeatures(ast:getSubNode(1))
      res = featureTimesFeatures(tf, subFeatures)
    elseif n == 2 then
      local subFeatures1 = makeCompositeFeatures(ast:getSubNode(1))
      local subFeatures2 = makeCompositeFeatures(ast:getSubNode(2))
      local isCommutative = (ast.op == 0 or ast.op == 2)
      if isCommutative then
        res = featureTimesFeaturesTimesFeaturesCommutative(tf, subFeatures1, subFeatures2)
      else
        res = featureTimesFeaturesTimesFeatures(tf, subFeatures1, subFeatures2)
      end
    end
    res["unit"] = 1.0
    return res
  end

  return makeCompositeFeatures(ast)
end

local formulaFeatureDictionary = Dictionary.new()
function formulaFeatures(formulaAST)

  features = formulaStringFeatures(formulaAST)  

  local dictionary = formulaFeatureDictionary
  local res = Vector.newSparse()

  --local dbg = ""
  for name,value in pairs(features) do
    --dbg = dbg .. "\n" .. name
    res[dictionary:add(name)] = value
  end
  --print(formulaAST:print(), "=>", dbg)
  return res
end