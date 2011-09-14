require 'DiscreteBandit'

filename = "/Users/jbecker/Documents/Workspace/Data/Proteins/dsbExperiments/SPXFromFile/kNN/bfs_dsb.bandit"

featureScores = {}

for line in io.lines(filename) do
  local featureName = string.match(line, "%a+%[.+%]")
  if featureName == nil then
    print(line)
  end  

  line = string.sub(line, #featureName + 1)
  local scores = {}
  for n in string.gmatch(line, "%d") do
    table.insert(scores, tonumber(n))
  end
  table.insert(featureScores, {featureName = featureName, scores = scores})
end

print ('Number of parameters: ' .. #featureScores)

function returnAllScores(scores)
  for i,v in ipairs(scores) do
    coroutine.yield(v)
  end
end

local banditProblem = {}
for i,v in ipairs(featureScores) do
  local f = coroutine.wrap(|| returnAllScores(v.scores))
  table.insert(banditProblem, f)
end

local policy = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}.__get


-- returns the number of times each bandit has been played
local function playEpisode(policy, bandits, numTimeSteps)
  local res = {}
  for i=1,#bandits do res[i] = 0 end 

  local co = coroutine.create(policy)
  local info = #bandits
  for i = 1,numTimeSteps do
    context:enter('Step ' .. i)
    context:result('Step', i)

    local ok, bi = coroutine.resume(co, info)
    if not ok then
      error("coroutine failed: " .. bi)
    end
    res[bi] = res[bi] + 1
    local ri = bandits[bi]()  -- sample from bandit distribution
    info = ri
    
    for j = 1,#bandits do
      context:result(featureScores[j].featureName, res[j])
    end

    context:leave()
  end
  return res  
end

-- return the parameter index with the highest long term rewards
local function getBestParameter(verbose)
  local bestReward = 0
  local bestParameterIndex = 0
  for i= 1,#featureScores do
    local reward = 0
    for j = 1,#featureScores[i].scores do
      reward = reward + featureScores[i].scores[j]
    end
    if verbose then
      context:result(featureScores[i].featureName, reward)
    end
    if reward > bestReward then
      bestReward = reward
      bestParameterIndex = i
    end
  end
  return bestParameterIndex
end

-- return index of table t with the highest value
local function getTableMaximum(t)
  local bestValue = 0
  local bestIndex = 0
  for i, v in ipairs(t) do
    if v > bestValue then
      bestValue = v
      bestIndex = i
    end
  end
  return bestIndex
end

context:enter('Long term reward')
bestParameter = getBestParameter(true)
context:leave(bestParameter)

print('Best Parameter: ' .. featureScores[bestParameter].featureName)
print('Best Bandit: ' .. bestParameter)

context:enter('Play episodes')
numPlayed = playEpisode(policy, banditProblem, 2500)
context:leave()

print('Best Found Parameter: ' .. featureScores[getTableMaximum(numPlayed)].featureName)
print('Best Found Bandits: ' .. getTableMaximum(numPlayed))