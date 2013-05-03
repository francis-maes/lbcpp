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

local function getScoreToMaximize(scoreObject)
  return 1 - scoreObject:getScore()
end

local function getRecall(scoreObject)
  return scoreObject:recall()
end

local function getBestSensitivity(scoreObject)
  return getSensitivity(scoreObject.bestConfusionMatrix)
end

local function getBestSpecificity(scoreObject)
  return getSpecificity(scoreObject.bestConfusionMatrix)
end

local function getAreaUnderCurve(scoreObject)
  return scoreObject.areaUnderCurve
end

local function getAccuracyAt5FPR(scoreObject)
  return scoreObject.accuracyAt5Fpr
end

local function getPrecision(scoreObject)
  return scoreObject:precision
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

  scoresOfInterest["AUC"] =       {index = 1, getScore = getAreaUnderCurve}
  scoresOfInterest["Acc 5FPR"] =  {index = 1, getScore = getAccuracyAt5FPR}
  scoresOfInterest["MCC"] =       {index = 2, getScore = getScoreToMaximize}
  scoresOfInterest["Precision"] = {index = 2, getScore = getPrecision}
  scoresOfInterest["Recall"] =    {index = 2, getScore = getRecall}

  for i = 1,#varValues do
    context:enter(varName .. ": " .. varValues[i])
    -- Load traces
    local traces = {}
    for fold = 0, numFolds do
      local fileName = filePrefix .. varValues[i] .. filePostfix .. fold .. ".trace"
      traces[fold] = lbcpp.Object.fromFile(fileName)
      if traces[fold] == nil then
        context:warning("Missing file: " .. fileName)
      end
    end

    context:result(varName, varValues[i])
    trainScoreStat = {} --getScores(traces, "EvaluateTrain", scoresOfInterest)
    testScoreStat = getScores(traces, "EvaluateTest", scoresOfInterest)
--    complexityStat = getComplexity(traces)

    -- Results
    for scoreName, scoreStat in pairs(trainScoreStat) do
      context:result("Train: " .. scoreName .. " (Mean)", scoreStat:getMean())
      context:result("Train: " .. scoreName .. " (StdDev)", scoreStat:getStandardDeviation())
    end

    for scoreName, scoreStat in pairs(testScoreStat) do
      context:result("Test: " .. scoreName .. " (Mean)", scoreStat:getMean())
      context:result("Test: " .. scoreName .. " (StdDev)", scoreStat:getStandardDeviation())
    end

--    context:result("Complexity (Mean)", complexityStat:getMean())
--    context:result("Complexity (StdDev)", complexityStat:getStandardDeviation())

    context:leave()
  end
end

local numFolds = 9

dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/drExperiments/130423-FeatureEvaluation-DR/Trace/"
features = {"pssm21", "pssm21sepsa21", "pssm21sepsa21hlaa60", "pssm21sepsa21hlaa60ss311", "pssm21sepsa21hlaa60ss311aa1"}
--main("DR", features, dir, ".LongDR.fold", numFolds)

features = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35}
main("DR - PSSM Size", features, dir .. "pssm", ".fold", numFolds)
