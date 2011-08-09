-- Francis Maes, 03/08/2011
-- Discrete Bandit problems and policies

require 'Vector'

DiscreteBandit = {}

----------------- index functions --------------------

subspecified function DiscreteBandit.ucb1C(rk, sk, tk, t)
  parameter C = {default = 2.0}
  return rk + math.sqrt(C * math.log(t) / tk)
end

subspecified function DiscreteBandit.ucbV(rk, sk, tk, t)
  parameter c = {default = 1.0}
  parameter zeta = {default = 1.0}
  local epsilon = zeta * math.log(t)
  return rk + math.sqrt((2.0 * sk * sk * epsilon) / tk) + c * (3.0 * epsilon) / tk
end

subspecified function DiscreteBandit.powerFunction(rk, sk, tk, t)
  parameter n = {default = 1}
  parameter theta = {default = Vector.newDense()}

  local function fastPow(value, power)
    if power == 0.0 then
      return 1.0
    elseif power == 1.0 then
      return value
    elseif power == 2.0 then
      return value * value
    else
      return math.pow(value, power)
  	end
  end
  
  local res = 0.0
  for i in 0,n do
    for j in 0,n do
      for k in 0,n do
        for l in 0,n do
          res = res + theta[i][j][k][l] * fastPow(rk, i) * fastPow(sk, j) * fastPow(tk, j) * fastPow(t, k);
        end
      end
    end
  end
  return res
end       


function DiscreteBandit.greedy(rk, sk, tk, t)
  return rk
end

DiscreteBandit.ucb1 = DiscreteBandit.ucb1C{C = 2.0}

function DiscreteBandit.ucb1Tuned(rk, sk, tk, t)
  local logT = math.log(t)
  local varianceUB = sk * sk + math.sqrt(2 * logT / tk)
  return rk + math.sqrt((logT / tk) * math.min(0.25, varianceUB))
end

function DiscreteBandit.ucb1Normal(rk, sk, tk, t)
  -- if there is a machine which has been played less then 8log(t) times, then play this machine
  if tk < math.ceil(8 * math.log(t)) then
    return math.huge
  else
    return rk + math.sqrt(16 * tk * sk * sk * math.log(t - 1) / (tk * (tk - 1)))
  end
end

subspecified function DiscreteBandit.klUcb(rk, sk, tk, t)
  parameter c = {default = 3.0}
  
  local function kl(p, q)
    local function alogaov(a, b)
      if a == 0 then
        return 0
      elseif b == 0 then
        return math.huge
      else
        return a * math.log(a / b)
      end
    end
    return alogaov(p, q) + alogaov(1 - p, 1 - q)
  end

  local limit = (math.log(t) + c * math.log(math.log(t))) / tk
  local function objective(opt)
    return kl(rk,opt) - limit
  end
  
  local epsilon = 1e-4
  function findZero(fn, min, max)
    --print ("findZero in [" .. min .. ", " .. max .. "]")
    if math.abs(max - min) < epsilon then
      return min
    else
      local middle = (min + max) / 2
      local value = fn(middle)
      if value > 0 then
        return findZero(fn, min, middle)
      else
        return findZero(fn, middle, max)
      end
    end
  end

  return findZero(|opt| kl(rk,opt) - limit, rk, 1)
end


---------- discrete bandit policies -----------------

function DiscreteBandit.randomPolicy(K)
  while true do
    local s = Stochastic.uniformInteger(1, K)
    coroutine.yield(s)
  end
end

subspecified function DiscreteBandit.indexBasedPolicy(K)
  parameter indexFunction = {default = DiscreteBandit.greedy}  

  -- create per-bandit statistics and score
  local stats = {}
  local t = 0

  for i = 1,K do
    table.insert(stats, Statistics.meanAndVariance())
  end

  local function computeBestBandits()
    local bestBandits = {}
    local bestScore = -math.huge
    for i = 1,K do 
      local stat = stats[i]
      local score = indexFunction(stat:getMean(), stat:getStandardDeviation(), stat:getCount(), t)
      if score > bestScore then
        bestBandits = {i}
        bestScore = score
      elseif score == bestScore then
        table.insert(bestBandits, i)
      end
    end
    
    local function arrayToString(a)
      local res = ""
      for i,v in ipairs(a) do
        res = res .. " " .. tostring(v)
      end
      return res
    end

    --print ("Best Bandits: " .. arrayToString(bestBandits))
    return bestBandits
  end

  -- argmax function
  local function sampleBestBandit()
    local bestBandits = computeBestBandits()
    local numBests = #bestBandits
    if numBests == 0 then
      return Stochastic.uniformInteger(1, K)
    else
      return bestBandits[Stochastic.uniformInteger(1, numBests)]
    end 
  end

  -- play function
  local function play(i)
    --print ("play " .. i)
    local reward = coroutine.yield(i)
    stats[i]:observe(reward)
    t = t + 1
   -- printCurrentState()
  end

  -- initialize: play each bandit once
  for i = 1,K do
    play(i)
  end

  -- play best bandit
  while true do
    play(sampleBestBandit())
  end
end

subspecified function DiscreteBandit.epsilonGreedy(K)
  parameter c = {default = 1}
  parameter d = {default = 1}

  -- create per-bandit statistics and score
  local stats = {}
  local t = 0

  for i = 1,K do
    table.insert(stats, Statistics.mean())
  end

  local function computeBestBandits()
    local bestBandits = {}
    local bestScore = -math.huge
    for i = 1,K do 
      local score = stats[i]:getMean()
      if score > bestScore then
        bestBandits = {i}
        bestScore = score
      elseif score == bestScore then
        table.insert(bestBandits, i)
      end
    end
    
    return bestBandits
  end

  -- argmax function
  local function sampleBestBandit()
    local bestBandits = computeBestBandits()
    local numBests = #bestBandits
    if numBests == 0 then
      return Stochastic.uniformInteger(1, K)
    else
      return bestBandits[Stochastic.uniformInteger(1, numBests)]
    end 
  end

  -- play function
  local function play(i)
    --print ("play " .. i)
    local reward = coroutine.yield(i)
    stats[i]:observe(reward)
    t = t + 1
   -- printCurrentState()
  end

  -- initialize: play each bandit once
  for i = 1,K do
    play(i)
  end

  -- play best bandit
  while true do
    local epsilon = math.min(1, c * K / (d * d * (t + 1)))
    if Stochastic.bernoulli(epsilon) == 1 then
      play(Stochastic.uniformInteger(1,K))
    else
      play(sampleBestBandit())
    end
  end
end

---------- play / evaluate ---------------


-- returns the number of times each bandit has been played
function DiscreteBandit.playEpisode(policy, bandits, numTimeSteps)
  local res = {}
  for i=1,#bandits do res[i] = 0 end 

  local co = coroutine.create(policy)
  local info = #bandits
  for i = 1,numTimeSteps do
    local ok, bi = coroutine.resume(co, info)
    if not ok then
      error("coroutine failed: " .. bi)
    end
    res[bi] = res[bi] + 1
    --print ("Bi = ", bi)
    local ri = bandits[bi]:sample()  -- sample from bandit distribution
    info = ri
    --print ("RI = ", info)
  end
  return res  
end

function DiscreteBandit.estimatePolicyRegret(policy, bandits, numEstimations, numTimeSteps)

  local K = #bandits
  
  -- find optimal bandit and associated expected reward
  local bestExpectedReward = -math.huge
  local bestBandit = 0
  for i = 1,K do
    local expectedReward = bandits[i].expectation()
    if expectedReward > bestExpectedReward then
      bestExpectedReward = expectedReward
      bestBandit = i
    end
  end
  --print ("Best bandit: " .. bestBandit .. " with expected reward " .. bestExpectedReward)

  -- play numEstimation games
  local stats = Statistics.mean()
  local res = 0.0
  for i = 1,numEstimations do --line 31
    local playCounts = DiscreteBandit.playEpisode(policy, bandits, numTimeSteps)
    local regret = 0
    local numIncorrects = 0
    for bi,ci in ipairs(playCounts) do
      local deltaReward = bestExpectedReward - bandits[bi].expectation()
      assert(deltaReward >= 0)
      if deltaReward > 0 then
        regret = regret + ci * deltaReward
        numIncorrects = numIncorrects + ci
      end
    end
    stats:observe(regret)
  end

  -- return regret  
  return stats:getMean()
  
end

function DiscreteBandit.estimatePolicyRegretOnProblems(policy, banditProblems, numEstimationsPerProblem, numTimeSteps)

  local regret = Statistics.mean()
  for i,problem in ipairs(banditProblems) do
    regret:observe(DiscreteBandit.estimatePolicyRegret(policy, problem, numEstimationsPerProblem, numTimeSteps))
    context:progress(i, #banditProblems, "Problems")
  end
  return regret:getMean()
end

return DiscreteBandit