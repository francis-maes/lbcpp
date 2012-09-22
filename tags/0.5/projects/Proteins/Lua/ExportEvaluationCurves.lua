require 'Statistics'

local function getScores(file)
  local trace = lbcpp.Object.fromFile(file)
  if trace == nil then
    return nil
  end
  local node = trace.root.subItems[1].subItems
  return node[#node].results[1].second.scores
end

local function getSensitivity(scoreObject)
  return scoreObject.truePositive / (scoreObject.truePositive + scoreObject.falseNegative)
end

local function getSpecificity(scoreObject)
  return scoreObject.trueNegative / (scoreObject.trueNegative + scoreObject.falsePositive)
end

local function getPrecision(scoreObject)
  return scoreObject.truePositive / (scoreObject.truePositive + scoreObject.falsePositive)
end

local function discrtizeValues(indexRanges, values)
  local res = {}
  local lastValues = {}
  for k, i in ipairs(indexRanges) do
    res[i] = Statistics.meanAndVariance()
    lastValues[i] = 1
  end
  
  for fpr, tpr in pairs(values) do
    local index = math.floor(fpr * 1000)
    if index < 5 then
      index = 0
    elseif index < 10 then
      index = 5
    else
      index = math.floor(fpr * 100) * 10
    end
    if index ~= 0 and index ~= 1000 then
      res[index]:observe(tpr)
    end
  end

  return res
end

local function generateRocCurve(scores)
  -- Threshold on FPR
  local indexRanges = {}
  table.insert(indexRanges, 5)
  for i = 10, 990, 10 do
    table.insert(indexRanges, i)
  end

  local stats = {}
  for k, i in ipairs(indexRanges) do
    stats[i] = Statistics.meanAndVariance()
  end

  for fold = 0, #scores do
    local matrices = scores[fold][2].confusionMatrices
    local values = {}
    for i = 1, #matrices do
      local fpr = 1 - getSpecificity(matrices[i])
      local tpr = getSensitivity(matrices[i])
--- ATTENTION ---
      fpr = getSensitivity(matrices[i])
      tpr = getPrecision(matrices[i])
--- ATTENTION ---
      values[fpr] = tpr
    end
    stat = discrtizeValues(indexRanges, values)
    for k, i in ipairs(indexRanges) do
      if stat[i].samplesCount ~= 0 then
        stats[i]:observe(stat[i]:getMean())
      end
    end
  end

  context:enter(0)
  context:result("1 - Specificity", 0)
  context:result("Sensitivity-Mean", 0)
  context:result("Sensitivity-StdDev", 0)
  context:leave()

  for k, i in ipairs(indexRanges) do
    if stats[i].samplesCount ~= 0 then
      context:enter(i)
      context:result("1 - Specificity", i / 1000)
      context:result("Sensitivity-Mean", stats[i]:getMean())
      context:result("Sensitivity-StdDev", stats[i]:getStandardDeviation())
      context:leave()
    end
  end

  context:enter(1000)
  context:result("1 - Specificity", 1)
  context:result("Sensitivity-Mean", 0)
  context:result("Sensitivity-StdDev", 0)
  context:leave()
end

local function computeAuc(scores)
  local res = Statistics.meanAndVariance()
  for fold = 0, #scores do
    local matrices = scores[fold][2].confusionMatrices
    local lastFPR = 1
    local lastTPR = 1
    local auc = 0
    for i = 1, #matrices do
      fpr = 1 - getSpecificity(matrices[i])
      tpr = getSensitivity(matrices[i])
      auc = auc + (lastFPR - fpr) * (tpr + lastTPR) / 2
      lastFPR = fpr
      lastTPR = tpr
      --print(fpr .. " " .. tpr .. " " .. auc)
    end
    res:observe(auc)
  end
  return res
end

local function getQpStat(scores)
  local res = Statistics.meanAndVariance()
  for fold = 0, #scores do
    res:observe(scores[fold][5]:getScore())
  end
  return res
end

--local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/WindowSizes/"
local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/K/"

local numFolds = 9

local windowSizes = {19} --{0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45}
for i = 1,#windowSizes do
  context:enter("Window Size: " .. windowSizes[i])
  -- Load traces
  context:enter("Load traces")
  local scores = {}
  for fold = 0, numFolds do
    print("Loading fold " .. fold)
    scores[fold] = getScores(dir .. "x3_Win" .. windowSizes[i] .. "_K200_1000T_NMIN1_Fold" .. fold .. ".trace")
  end
  context:leave()
  -- Generate ROC Curve
  context:enter("ROC Curve")
  generateRocCurve(scores)
  context:leave()
  -- Compute AUC
  aucStat = computeAuc(scores)
  -- Compute Qp
  qpStat = getQpStat(scores)

  context:result("WindowSize", windowSizes[i])
  context:result("AUC (Mean)", aucStat:getMean())
  context:result("AUC (StdDev)", aucStat:getStandardDeviation())
  context:result("Qp (Mean)", 1 - qpStat:getMean())
  context:result("Qp (StdDev)", qpStat:getStandardDeviation())
  context:leave()
end