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
--  scoresOfInterest["1 - Qp (Greedy 6)"] = {index = 5, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Qp"] = {index = 6, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Qp (Greedy 20)"] = {index = 26, getScore = getScoreToMinimize}
--  for L = 1, 24 do
--    scoresOfInterest["Greedy " .. L] = {index = 6 + L, getScore = getScoreToMinimize}
--  end
--  scoresOfInterest["1 - Q2"] = {index = 1, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Q2 (Bias form test)"] = {index = 2, getScore = getScoreToMinimize}
--  scoresOfInterest["1 - Sensitivity"] = {index = 1, getScore = getSensitivity}
--  scoresOfInterest["1 - Specificity"] = {index = 1, getScore = getSpecificity}
--  scoresOfInterest["1 - Sensitivity (Bias form test)"] = {index = 3, getScore = getBestSensitivity}
--  scoresOfInterest["1 - Specificity (Bias from test)"] = {index = 3, getScore = getBestSpecificity}

  scoresOfInterest["Q2"] = {index = 1, getScore = getScoreToMaximize}
--  scoresOfInterest["Qp (Perfect)"] = {index = 8, getScore = getScoreToMaximize}
--  scoresOfInterest["OxyDSB Qp (Perfect)"] = {index = 10, getScore = getScoreToMaximize}
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

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/1203XX-DSBWithExtraTreesAndPerfectMatching/"
local numFolds = 9

--local winSizes = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35}--, 37, 39, 41, 43, 45}

dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/120622-CBS/"
--main("CBS", {"hlpssm75", "hlpssm75_hlsa10", "hlpssm75_hlsa10_hgsa", "hlpssm75_hlsa10_hgsa_csp12"}, dir .. "x3_", "_K0_1000T_NMIN1_CBS_Fold", numFolds)


dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/121018-PlosOne-MajorRevision/"
--main("CBP", {"hgpssm", "hgpssm_hgaa"}, dir .. "FeatureEvaluation-Kmax/", ".param.x3.Kmax.fold", numFolds)
--main("CBS", {"wpssm11", "wpssm11_hgpssm", "wpssm11_hgpssm_nc", "wpssm11_hgpssm_nc_hgaa"}, dir, ".param.x3.fold", numFolds)

--main("SPX - CBS", {""}, dir .. "MultiTask/SPX/CBS/SPX-CBS.wpssm11_hgpssm_nc.x3.Kmax.fold", "", numFolds)
--main("SPXC - CBS", {""}, dir .. "MultiTask/SPXC/CBS/SPXC-CBS.wpssm11_hgpssm_nc.x3.Kmax.fold", "", numFolds)

main("SPX - ODSB", {""}, dir .. "MultiTask/SPX/ODSB/SPX-ODSB.wpssm15_csp17.x3.fold", "", numFolds)
main("SPXC - ODSB", {""}, dir .. "MultiTask/SPXC/ODSB/SPXC-ODSB.wpssm15_csp17.x3.fold", "", numFolds)

--main("SPX - DSB", {""}, dir .. "MultiTask/SPX/DSB/SPX-DSB.wpssm15_csp17.x3.fold", "", numFolds)
--main("SPXC - DSB", {""}, dir .. "MultiTask/SPXC/DSB/SPXC-DSB.wpssm15_csp17.x3.fold", "", numFolds)

--main("SPX - aCSB-DSB", {""}, dir .. "MultiTask/SPX/ActualCBS-ODSB/SPX-aCBS-ODSB.wpssm15_csp17.x3.fold", "", numFolds)
--main("SPXC - aCSB-DSB", {""}, dir .. "MultiTask/SPXC/ActualCBS-ODSB/SPXC-aCBS-ODSB.wpssm15_csp17.x3.fold", "", numFolds)

--main("SPXC - CSBToCBP", {""}, dir .. "MultiTask/SPXC/CBSToCBP/SPXC-CBSToCBP.wpssm11_hgpssm_nc.x3.Kmax.fold", "", numFolds)


--main("CBP > K", {1,2,5,10,20,25}, dir .. "K/hgpssm_nc.K", ".x3.fold", numFolds)
--main("CBP > Nmin", {1,3,5,7,11,15,21,51,101,155,201}, dir .. "Nmin/hgpssm_nc.Nmin", ".x3.fold", numFolds)
--main("CBP > Trees", {1,2,5,10,20,50,100,200,500,1000,2000,5000,10000}, dir .. "Trees/hgpssm_nc.T", ".x3.fold", numFolds)

dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/121018-PlosOne-MajorRevision/CBS/"
--main("CBS > K", {1,2,5,10,20,50,100,200,256}, dir .. "K/wpssm11_hgpssm_nc.K", ".x3.fold", numFolds)
--main("CBP > Nmin", {1,3,5,7,11,15,21,51,101,155,201}, dir .. "Nmin/wpssm11_hgpssm_nc.Nmin", ".x3.fold", numFolds)
--main("CBP > Trees", {1,2,5,10,20,50,100,200,500,1000,2000,5000,10000}, dir .. "Trees/wpssm11_hgpssm_nc.T", ".x3.fold", numFolds)


dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/121018-PlosOne-MajorRevision/CBP/"
dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/121018-PlosOne-MajorRevision/CBS/"

local C = {0, 5, 10, 15}
local G = {-14, -7, 0, 7, 14}

--for i = 1, #C do
--  context:enter("C" .. C[i])
--  main("CBP > C" .. C[i] .. " G", G, dir .. "SVM/hgpssm_nc.C" .. C[i] .. ".G", ".SVM.fold", numFolds)
--  main("CBS > C" .. C[i] .. " G", G, dir .. "SVM/wpssm11_hgpssm_nc.C" .. C[i] .. ".G", ".SVM.fold", numFolds)

--  context:leave()
--end

-- ----- SP39 ----- --

numFolds = 3
--main("SP39", {"500T"}, dir .. "Test/x3_Win19_K2000_", "_NMIN1", numFolds)