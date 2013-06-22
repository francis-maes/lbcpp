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

local function getBalancedAccuracy(scoreObject)
  return (scoreObject:recall + scoreObject:specificity) / 2
end

local function getBestBalancedAccuracy(scoreObject)
  return (scoreObject.bestConfusionMatrix:recall + scoreObject.bestConfusionMatrix:specificity) / 2
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

--  scoresOfInterest["AUC"] =           {index = 1, getScore = |x| x.areaUnderCurve}
--  scoresOfInterest["Balanced Acc."] = {index = 2, getScore = getBalancedAccuracy}
--  scoresOfInterest["Recall"] =        {index = 2, getScore = |x| x:recall}
--  scoresOfInterest["Specificity"] =   {index = 2, getScore = |x| x:specificity}
  scoresOfInterest["F1"] =            {index = 2, getScore = |x| x:f1}

--  scoresOfInterest["AUC"] =           {index = 5, getScore = |x| x.areaUnderCurve}
--  scoresOfInterest["Balanced Acc."] = {index = 5, getScore = getBestBalancedAccuracy}
--  scoresOfInterest["Recall"] =        {index = 5, getScore = |x| x.bestConfusionMatrix:recall}
--  scoresOfInterest["Specificity"] =   {index = 5, getScore = |x| x.bestConfusionMatrix:specificity}
--  scoresOfInterest["F1"] =            {index = 5, getScore = |x| x.bestConfusionMatrix:f1}
--  scoresOfInterest["Th. SensSpec"] =  {index = 1, getScore = |x| x.bestConfusionMatrix.threshold}



--  scoresOfInterest["Acc 5FPR"] =  {index = 1, getScore = getAccuracyAt5FPR}
--  scoresOfInterest["MCC"] =       {index = 2, getScore = getScoreToMaximize}
--  scoresOfInterest["Precision"] = {index = 2, getScore = getPrecision}
--  scoresOfInterest["Recall"] =    {index = 2, getScore = getRecall}

  for i = 1,#varValues do
    context:enter(varName .. ": " .. varValues[i])
    -- Load traces
    local traces = {}
    for fold = 0, numFolds do
      local fileName = filePrefix .. varValues[i] .. filePostfix .. fold .. ".trace"
      traces[fold] = lbcpp.Object.fromFile(fileName)
      if traces[fold] == nil then
        context:warning("Missing file: " .. varValues[i] .. filePostfix .. fold .. ".trace")
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
--main("DR", features, dir, ".fold", numFolds)

features = {49}
--main("DR - SEP(SA) Size", features, dir .. "pssm23sepsa", ".fold", numFolds)

dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/drExperiments/130527-CASP10FeatureEvaluation/Trace/"
features = {"SegSA21", "SegSA21_HlocalPSSM60", "SegSA21_HlocalPSSM60_WinSA21", "pssm21", "pssm21sepsa21", "pssm21sepsa21hlaa60", "pssm21sepsa21hlaa60ss311", "pssm21sepsa21hlaa60ss311aa1"}

main("DR - DISpro", features, dir .. "Merge.Low-DoubleBias-F1.", ".fold", numFolds)