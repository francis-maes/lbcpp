require 'Statistics'

local function getScores(file)
  local trace = lbcpp.Object.fromFile(file)
  if trace == nil then
    return nil
  end
  return trace.root.subItems[1].subItems[4].results[1].second.scores
end

local function getAveragedScore(prefix)
  local stat = Statistics.meanAndVariance()
  for i = 0, 9 do
    local scores = getScores(prefix .. i .. ".trace")
    if scores == nil then
      context:warning("Missing file: " .. prefix .. i .. ".trace")
    else
      stat:observe(scores[5]:getScore())
    end
  end
  return stat
end

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/Tasks/"

local tasks = {"NoTask", "ss3", "ss8", "sa", "dr", "stal", "AllTask"}

for k, task in ipairs(tasks) do
  local prefix = "x3_" .. task .. "_Win40_HugeK_1000T_NMIN1_Fold"
  local stat = getAveragedScore(dir .. prefix)
  context:enter("Task: " .. task)
  context:result("Task", task)
  context:result("Index", k)
  context:result("Mean", stat:getMean())
  context:result("StdDev", stat:getStandardDeviation())
  context:leave(stat:getMean())
end