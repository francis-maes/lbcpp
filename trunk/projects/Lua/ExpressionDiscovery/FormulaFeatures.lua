-- Formula Features

require 'AST'
require 'Vector'
require 'Dictionary'

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