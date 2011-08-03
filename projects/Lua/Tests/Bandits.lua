
----------------- index functions --------------------

subspecified function ucb1C(rk, sk, tk, t)
  parameter C = {default = 2.0}
  return rk + math.sqrt(C * math.log(t) / tk)
end

subspecified function ucbV(rk, sk, tk, t)
  parameter c = {default = 1.0}
  parameter zeta = {default = 1.0}
  local epsilon = zeta * math.log(t)
  return rk + math.sqrt((2.0 * sk * sk * epsilon) / tk) + c * (3.0 * epsilon) / tk
end

subspecified function powerFunction(rk, sk, tk, t)
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


function greedy(rk, sk, tk, t)
  return rk
end

ucb1 = ucb1C{C = 2.0}

function ucb1Tuned(rk, sk, tk, t)
  local logT = math.log(t)
  local varianceUB = sk * sk + math.sqrt(2 * logT / tk)
  return rk + math.sqrt((logT / tk) * math.min(0.25, varianceUB)
end


---------- discrete bandit policies -----------------

function randomPolicy(K)
  while true do
    coroutine.yield(Sampler.uniformInteger(1, K))
  end
end

subspecified function indexBasedPolicy(k)
  parameter indexFunction = {default = greedy}  

  -- create per-bandit statistics and score
  local stats = {}
  local scores = Vector.newDense()
  for i = 1,K do
    table.insert(stats, Statistics.meanVarianceAndBounds())
    table.insert(scores, 0)
  end

  -- play function
  local t = 0
  local function play(i)
    local stat = stats[i]
    local reward = coroutine.yield(i)
    stat:observe(reward)
    t = t + 1
    scores[i] = indexFunction(stat.getMean(), stat.getStandardDeviation(), stat.getCount(), t)
  end

  -- initialize: play each bandit once
  for i = 1,K do
    play(i)
  end

  -- play best bandit
  while true do
    play(scores:argmax())
  end
end


---------- evaluation -----------------

function playBanditGame(bandits, policy, numTimeSteps)
  local res = 0.0
  local co = coroutine.create(policy)
  local info = K
  for i in 1,numTimeSteps do
    local bi = coroutine.resume(co, info)
    info = bandits[bi]()  -- sample from bandit distribution
  end
  return res  
end

function evaluatePolicyOnProblem(bandits, policy, numEstimations, numTimeSteps)

  local K = #bandits
  
  -- find optimal bandit and associated expected reward
  local bestExpectedReward = 0.0
  local bestBandit = 0
  for i in 1,K do
    local expectedReward = bandits[i].expectation()
    if expectedReward > bestExpectedReward then
      bestExpectedReward = expectedReward
      bestBandit = i
    end
  end

  -- play numEstimation games
  local stats = Statistics.mean()
  local res = 0.0
  for i in 1,numEstimations do
    stats:observe(playBanditGame(bandits, policy, numTimeSteps))
  
  -- return regret  
  return bestExpectedReward * numTimeSteps - stats:getMean()
  
end

function evaluatePolicyOnProblems(banditsSampler, policy, numProblems, numEstimationsPerProblem, numTimeSteps)

  local regret = Statistics.mean()
  for i in 1,numProblems do
    local bandits = banditsSampler()
    regret:observe(evaluatePolicyOnProblem(bandits, policy, numEstimationsPerProblem, numTimeSteps))
  end
  return regret:getMean()
end


---------- main -----------------

local function uniformBernoulliBanditsSampler(K)
  return function () 
    res = {}
    for i=1,K do
      table.append(res, Sampler.bernoulli(Sampler.uniform(0.0, 1.0)()))
    end
    return res
  end
end

local function main(numProblems, numEstimationsPerProblem, numTimeSteps)

  local numBandits = 2

  local function policyObjective(policy)
    return evaluatePolicyOnProblems(uniformBernoulliBanditsSampler(numBandits), policy, numProblems, numEstimationsPerProblem, numTimeSteps)
  end
  
  local function powerFunctionObjective(theta)
    return policyObjective(powerFunction(theta, 1))
  end

  bestTheta = Optimizer.eda(myObjectiveFunction, Sampler.independentVector(Sampler.normalGaussian()), numIterations, ... tout ca)
end