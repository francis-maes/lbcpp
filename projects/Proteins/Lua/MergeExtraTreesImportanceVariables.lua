require 'Context'
require 'Statistics'

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/x3_Win40_HugeK_1000T_10FCV/"

local function getRankScores(file)
  local trace = lbcpp.Object.fromFile(file)
  local varImpTrace = trace.root.subItems[1].subItems[3].subItems[4].subItems[1]
  res = {}
  for i = 1, #varImpTrace.subItems do
    res[i] = {score = varImpTrace.subItems[i].results[2].second,
              index = varImpTrace.subItems[i].results[3].second,
              name = varImpTrace.subItems[i].results[4].second}
  end
  return res
end

local function compareVariableImportance(a, b)
  return a.stat:getMean() > b.stat:getMean()
end

local sumVarImp = nil
for i = 0, 9 do
  local fileName = "x3_Win40_HugeK_1000T_Fold" .. i .. ".trace"
  print("Loading trace ... " .. fileName)
  varImp = getRankScores(dir .. fileName)
  if sumVarImp == nil then
      sumVarImp = {}
      for j = 1, #varImp do
        sumVarImp[j] = {}
        sumVarImp[j].name = varImp[j].name
        sumVarImp[j].index = varImp[j].index
        sumVarImp[j].stat = Statistics.meanAndVariance()
        sumVarImp[j].stat.name = varImp[j].name
        sumVarImp[j].stat:observe(varImp[j].score)
      end
  else
    for j = 1, #sumVarImp do
      assert(sumVarImp[j].name == varImp[j].name, "Error: " .. sumVarImp[j].name .. " == " .. varImp[j].name)
      sumVarImp[j].stat:observe(varImp[j].score)
    end
  end
end

print("Save merge of variable importances")
local res = lbcpp.Object.create("GenericVector<Pair<PositiveInteger, ScalarVariableMeanAndVariance>>")
res:resize(#sumVarImp)
for i = 1, #sumVarImp do
  local p = lbcpp.Object.create("Pair<PositiveInteger, ScalarVariableMeanAndVariance>")
  p.first = sumVarImp[i].index
  p.second = sumVarImp[i].stat
  res[i] = p
end
res:save("/Users/jbecker/Documents/Workspace/LBC++/bin/Debug/mergeVarImp.xml")

table.sort(sumVarImp, compareVariableImportance)

context:enter("Variable Importances")
for i = 1, #sumVarImp do
  context:enter("Variable " .. sumVarImp[i].name)
  context:result("Rank", i)
  context:result("Score - Mean", sumVarImp[i].stat:getMean())
  context:result("Score - StdDev", sumVarImp[i].stat:getStandardDeviation())
  context:result("Score - Mean-StdDev", sumVarImp[i].stat:getMean() - sumVarImp[i].stat:getStandardDeviation())
  context:result("Score - Mean+StdDev", sumVarImp[i].stat:getMean() + sumVarImp[i].stat:getStandardDeviation())
  context:result("Score - Mean/StdDev", sumVarImp[i].stat:getMean() / sumVarImp[i].stat:getStandardDeviation())
  context:result("Index", sumVarImp[i].index)
  context:result("Variable", sumVarImp[i].name)
  context:leave()
end
context:leave()

function addStat(a, b)
  a.samplesSum = a.samplesSum + b.samplesSum
  a.samplesCount = a.samplesCount + b.samplesCount
  a.samplesSumOfSquares = a.samplesSumOfSquares + b.samplesSumOfSquares
end

local varImpByName = {}
varImpByName["p"] = {numChilds = 0, stat = Statistics.meanAndVariance()}
for i = 1, #sumVarImp do
  local currentName = "p"
  varImpByName[currentName].numChilds = varImpByName[currentName].numChilds + 1
  addStat(varImpByName[currentName].stat, sumVarImp[i].stat)

  for varName in string.gmatch(sumVarImp[i].name, "[^.]+") do
    currentName = currentName .. "." .. varName
    if varImpByName[currentName] == nil then
      varImpByName[currentName] = {}
      varImpByName[currentName].numChilds = -1
      varImpByName[currentName].stat = Statistics.meanAndVariance()
    end
    varImpByName[currentName].numChilds = varImpByName[currentName].numChilds + 1
    addStat(varImpByName[currentName].stat, sumVarImp[i].stat)
  end
end

local function compareScore(a, b)
  return a.score > b.score
end

-- Sort by Sum

local toSort = {}
local index = 1
for k in pairs(varImpByName) do
  toSort[index] = {score = varImpByName[k].stat.samplesSum, name = k}
  index = index + 1
end

table.sort(toSort, compareScore)

context:enter("Group Variable Importances (by Sum)")
local rank = 1
for i = 1, #toSort do
  local name = toSort[i].name
  context:enter("Group " .. name)
  context:result("Rank", rank)

  context:result("Score - Sum", varImpByName[name].stat.samplesSum)
  context:result("Score - Mean", varImpByName[name].stat:getMean())
  context:result("Score - StdDev", varImpByName[name].stat:getStandardDeviation())
  context:result("NumChildren", varImpByName[name].numChilds)
  context:result("Variable", name)

  context:leave()
  rank = rank + 1
end
context:leave()

-- Sort by Mean

local toSort = {}
local index = 1
for k in pairs(varImpByName) do
  toSort[index] = {score = varImpByName[k].stat:getMean(), name = k}
  index = index + 1
end

table.sort(toSort, compareScore)

context:enter("Group Variable Importances (by Mean)")
local rank = 1
for i = 1, #toSort do
  local name = toSort[i].name
  context:enter("Group " .. name)
  context:result("Rank", rank)

  context:result("Score - Mean", varImpByName[name].stat:getMean())
  context:result("Score - StdDev", varImpByName[name].stat:getStandardDeviation())
  context:result("Score - Sum", varImpByName[name].stat.samplesSum)
  context:result("NumChildren", varImpByName[name].numChilds)
  context:result("Variable", name)

  context:leave()
  rank = rank + 1
end
context:leave()
