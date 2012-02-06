require 'Statistics'
require 'Context'

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

local function getScores(traces, nodeName, scoresOfInterest)
  -- Init. results
  local res = {}
  for scoreName in pairs(scoresOfInterest) do
    res[scoreName] = Statistics.meanAndVariance()
  end
  -- Read scores
  for fold, trace in pairs(traces) do
    if trace ~= nil then
      local node = findSubNode(trace.root.subItems[1], nodeName)
      if node == nil then
        if nodeName == "EvaluateTrain" then
          return res
        else
          node = trace.root.subItems[1].subItems[4]
        end
      end
      -- Observe scores
      scoresNode = node.results[1].second
      for scoreName, scoreInfo in pairs(scoresOfInterest) do
        res[scoreName]:observe(scoreInfo.getScore(scoresNode.scores[scoreInfo.index]))
      end
    end
  end
  return res
end

local function getComplexity(traces)
  local res = Statistics.meanAndVariance()
  for fold, trace in pairs(traces) do
    if trace ~= nil then
      local complexity = findSubNode(trace.root.subItems[1], "Training").subItems[4].subItems[1].results[5].second
      res:observe(complexity)
    end
  end
  return res
end

-- file name: filePrefix .. varValues[.] .. filePostfix .. "_Foldx.trace"
local function main(varName, varValues, filePrefix, filePostfix, numFolds)
  local scoresOfInterest = {}
  scoresOfInterest["1 - Qp (Greedy 6)"] = {index = 5, getScore = getScoreToMinimize}
  scoresOfInterest["1 - Qp"] = {index = 6, getScore = getScoreToMinimize}
  scoresOfInterest["1 - Q2"] = {index = 1, getScore = getScoreToMinimize}
  scoresOfInterest["1 - Q2 (Bias form test)"] = {index = 2, getScore = getScoreToMinimize}
  scoresOfInterest["1 - Sensitivity"] = {index = 1, getScore = getSensitivity}
  scoresOfInterest["1 - Specificity"] = {index = 1, getScore = getSpecificity}
  scoresOfInterest["1 - Sensitivity (Bias form test)"] = {index = 3, getScore = getBestSensitivity}
  scoresOfInterest["1 - Specificity (Bias from test)"] = {index = 3, getScore = getBestSpecificity}

  for i = 1,#varValues do
    context:enter(varName .. ": " .. varValues[i])
    -- Load traces
    local traces = {}
    for fold = 0, numFolds do
      local fileName = filePrefix .. varValues[i] .. filePostfix .. "_Fold" .. fold .. ".trace"
      traces[fold] = lbcpp.Object.fromFile(fileName)
      if traces[fold] == nil then
        context:warning("Missing file: " .. fileName)
      end
    end

    context:result(varName, varValues[i])
    trainScoreStat = getScores(traces, "EvaluateTrain", scoresOfInterest)
    testScoreStat = getScores(traces, "EvaluateTest", scoresOfInterest)
    complexityStat = getComplexity(traces)

    -- Results
    for scoreName, scoreStat in pairs(trainScoreStat) do
      context:result("Train: " .. scoreName .. " (Mean)", scoreStat:getMean())
      context:result("Train: " .. scoreName .. " (StdDev)", scoreStat:getStandardDeviation())
    end

    for scoreName, scoreStat in pairs(testScoreStat) do
      context:result("Test: " .. scoreName .. " (Mean)", scoreStat:getMean())
      context:result("Test: " .. scoreName .. " (StdDev)", scoreStat:getStandardDeviation())
    end

    context:result("Complexity (Mean)", complexityStat:getMean())
    context:result("Complexity (StdDev)", complexityStat:getStandardDeviation())

    context:leave()
  end
end

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/"
local numFolds = 9
local nmin = {1, 3} --, 5, 7, 9, 11, 15, 19, 23, 27, 31, 55, 75, 101, 155, 301, 501}
local nminPrefix = dir .. "Nmin/x3_Win19_HugeK_1000T_NMIN"
--main("Nmin", nmin, prefix, "", numFolds)

local k = {1, 10, 20, 50, 100, 200, 500, 1000, 2000}
main("K", k, dir .. "K/x3_Win19_K", "_1000T_NMIN1", numFolds)



