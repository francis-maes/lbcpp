local function getAttributes(node)
  local res = {}
  local n = #(node.results)
  for i=1,n do
    local pair = node.results[i]
    res[pair.first] = pair.second
  end
  return res
end

local function findSubNode(node, description)
  local n = #(node.subItems)
  for i=1,n do
    if node.subItems[i]:toShortString() == description then
      return node.subItems[i]
    end
  end
  return nil
end

local function getScores(file)
  local trace = lbcpp.Object.fromFile(file)
  if trace == nil then
    return nil
  end
  return trace.root.subItems[1].subItems[4].results[1].second.scores
end

local function writeLine(f, line)
  f:write(line)
  f:write("\n")
  --print (">>" .. line)
end

local function getScoreToMinimize(scoreObject)
  return scoreObject:getScore()
end

local function getSensitivity(scoreObject)
  return 1 - (scoreObject.truePositive / (scoreObject.truePositive + scoreObject.falseNegative))
end

local function getSpecificity(scoreObject)
  return 1 - (scoreObject.trueNegative / (scoreObject.trueNegative + scoreObject.falsePositive))
end

local function getBestSensitivity(scoreObject)
  return getSensitivity(scoreObject.bestConfusionMatrix)
end

local function getBestSpecificity(scoreObject)
  return getSpecificity(scoreObject.bestConfusionMatrix)
end

require 'Statistics'
require 'Context'

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/WindowSizes/"
local scoresOfInterest = {}
scoresOfInterest["1 - Qp (Greedy 6)"] = {index = 5, getScore = getScoreToMinimize}
scoresOfInterest["1 - Qp"] = {index = 6, getScore = getScoreToMinimize}
scoresOfInterest["1 - Q2"] = {index = 1, getScore = getScoreToMinimize}
scoresOfInterest["1 - Q2 (Bias form test)"] = {index = 2, getScore = getScoreToMinimize}
scoresOfInterest["1 - Sensitivity"] = {index = 1, getScore = getSensitivity}
scoresOfInterest["1 - Specificity"] = {index = 1, getScore = getSpecificity}
scoresOfInterest["1 - Sensitivity (Bias form test)"] = {index = 3, getScore = getBestSensitivity}
scoresOfInterest["1 - Specificity (Bias from test)"] = {index = 3, getScore = getBestSpecificity}

local function averageOverFolds(prefixFileName)
  local stats = {}
  for scoreName in pairs(scoresOfInterest) do
    stats[scoreName] = Statistics.meanAndVariance()
  end
  for i = 0, 9 do
    local fileName = prefixFileName .. i .. ".trace"
    local scores = getScores(dir .. fileName)
    if scores == nil then
      context:warning("Missing file: " .. fileName)
    else
      for scoreName, scoreValue in pairs(scoresOfInterest) do
        local score = scoreValue.getScore(scores[scoreValue.index]) * 100
        stats[scoreName]:observe(score)
        context:result(scoreName .. " - Fold " .. i, score)
      end
    end
  end

  for scoreName, scoreValue in pairs(scoresOfInterest) do
    context:result(scoreName .. " - Mean", stats[scoreName]:getMean())
    context:result(scoreName .. " - StdDev", stats[scoreName]:getStandardDeviation())
  end
  return stats
end

local f = assert(io.open("/Users/jbecker/Documents/Workspace/LBC++/bin/Debug/result_WindowSize_x3_HugeK_1000T_NMIN1", "w"))

local line = "# \"Window Size\""
for scoreName in pairs(scoresOfInterest) do
  line = line .. "\t\"" .. scoreName .. "(Mean)\"\t" .. "\"" .. scoreName .. "(StdDev)\""
end
writeLine(f, line)

windowSizes = {0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 27, 29, 31, 33, 35, 37, 39, 45}
for i = 1,#windowSizes do
  local prefix = "x3_Win" .. windowSizes[i] .. "_HugeK_1000T_NMIN1_Fold"
  context:enter("Window Size: " .. windowSizes[i])
  context:result("WindowSize", windowSizes[i])
  local stats = averageOverFolds(prefix)
  context:leave()

  line = windowSizes[i]
  for scoreName in pairs(scoresOfInterest) do
    line = line .. "\t" .. stats[scoreName]:getMean() .. "\t" .. stats[scoreName]:getStandardDeviation()
  end
  writeLine(f, line)
end

f:close()