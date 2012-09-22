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

local function getScores(traces, L)
  -- Init. results
  res = Statistics.meanAndVariance()
  -- Read scores
  for fold, trace in pairs(traces) do
    if trace ~= nil then
      local node = findSubNode(trace.root.subItems[1], "EvaluateTest")
      -- Observe scores
      scoreNode = node.results[1].second.scores[6 + L]
      res:observe(scoreNode:getScore())
    end
  end
  return res
end

-- file name: filePrefix .. "_Foldx.trace"
local function main(filePrefix, numFolds)
    -- Load traces
    local traces = {}
    for fold = 0, numFolds do
      local fileName = filePrefix .. "_Fold" .. fold .. ".trace"
      traces[fold] = lbcpp.Object.fromFile(fileName)
      if traces[fold] == nil then
        context:warning("Missing file: " .. fileName)
      end
    end

    for greedy = 1, 24 do
      context:enter("L: " .. greedy)
      scoreStat = getScores(traces, greedy)
      context:result("L", greedy)
      context:result("Mean", scoreStat:getMean())
      context:result("StdDev", scoreStat:getStandardDeviation())
      context:leave()
    end
end

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/"
local numFolds = 9

main(dir .. "K200_MultiGreedy/x3_Win19_K200_1000T_NMIN1", numFolds)

--[[
context:enter("NoTask")
main(dir .. "Tasks/x3_NoTask_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
context:enter("AllTask")
main(dir .. "Tasks/x3_AllTask_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
context:enter("SS3")
main(dir .. "Tasks/x3_ss3_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
context:enter("SS8")
main(dir .. "Tasks/x3_ss8_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
context:enter("SA")
main(dir .. "Tasks/x3_sa_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
context:enter("DR")
main(dir .. "Tasks/x3_dr_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
context:enter("StAl")
main(dir .. "Tasks/x3_stal_Win19_K0_1000T_NMIN1", numFolds)
context:leave()
]]--