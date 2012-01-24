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

require 'Statistics'
require 'Context'

local dir = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/FromESSANAndBoth/x3/WindowSizes/"
local scoresOfInterest = {1, 2, 5, 6}
local scoresNames = {}

local function averageOverFolds(prefixFileName)
  local stats = {}
  for i = 1, #scoresOfInterest do
    stats[i] = Statistics.meanAndVariance()
  end
  for i = 0, 9 do
    local fileName = prefixFileName .. i .. ".trace"
    local scores = getScores(dir .. fileName)
    if scores == nil then
      context:warning("Missing file: " .. fileName)
    else
      for j = 1, #scoresOfInterest do
        stats[j]:observe(scores[scoresOfInterest[j]]:getScore())
        scoresNames[j] = j .. "-" .. scores[scoresOfInterest[j]].name
        context:result(scoresNames[j] .. " - Fold " .. i, scores[scoresOfInterest[j]]:getScore() * 100)
      end
    end
  end

  for j = 1, #scoresOfInterest do
    context:result(scoresNames[j] .. " - Mean", stats[j]:getMean() * 100)
    context:result(scoresNames[j] .. " - StdDev", stats[j]:getStandardDeviation() * 100)
  end
  return stats
end

local f = assert(io.open("result_WindowSize_x3_HugeK_1000T_NMIN1", "w"))

windowSizes = {0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 29, 35}
for i = 1,#windowSizes do
  local prefix = "x3_Win" .. windowSizes[i] .. "_HugeK_1000T_NMIN1_Fold"
  context:enter("Window Size: " .. windowSizes[i])
  context:result("WindowSize", windowSizes[i])
  local stats = averageOverFolds(prefix)
  context:leave()
  local line = ""
  if i == 1 then
    line = "#"
    for j = 1, #scoresOfInterest do
      line = line .. "\t" .. scoresNames[j]
    end
    writeLine(f, line)
  end

  line = windowSizes[i]
  for j = 1, #scoresOfInterest do
    line = line .. "\t" .. stats[j]:getMean() .. "\t" .. stats[j]:getStandardDeviation()
  end
  writeLine(f, line)
end

f:close()