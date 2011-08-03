require 'DiscreteBandit'

---------- evaluation -----------------

function playBanditGame(policy, bandits, numTimeSteps)
  local res = 0.0
  local pol = coroutine.wrap(policy)
  local info = K
  for i in 1,numTimeSteps do
    local bi = pol(info)
    info = bandits[bi]:sample()  -- sample from bandit distribution
  end
  return res  
end

function evaluatePolicyOnProblem(policy, bandits, numEstimations, numTimeSteps)

  local K = #bandits
  
  -- find optimal bandit and associated expected reward
  local bestExpectedReward = -math.huge
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
    stats:observe(playBanditGame(policy, bandits, numTimeSteps))
  end
  
  -- return regret  
  return bestExpectedReward * numTimeSteps - stats:getMean()
  
end

function evaluatePolicyOnProblems(policy, banditProblems, numEstimationsPerProblem, numTimeSteps)

  local regret = Statistics.mean()
  for i in 1,#banditProblems do
    local bandits = banditsSampler()
    regret:observe(evaluatePolicyOnProblem(policy, banditProblems[i], numEstimationsPerProblem, numTimeSteps))
  end
  return regret:getMean()
end


------------- Sampler ---------------

Sampler = {}

--stochastic subspecified function Sampler.bernoulli()
--  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}
--  return Stochastic.bernoulli(p)
--end

Sampler.bernoulli = subspecified setmetatable({
       
     sample = function ()
       parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}
       return Stochastic.bernoulli:sample(p)
     end,

  }, Stochastic.MT)


-------------- main -----------------


policy1 = DiscreteBandit.randomPolicy
policy2 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.greedy}
policy3 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1}
policy4 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbTuned}
policy5 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=2,zeta=5}}

bandits = {Sampler.bernoulli{0.9}, Sampler.bernoulli{0.6}}

evaluatePolicyOnProblem(policy1, bandits, numEstimations, numTimeSteps)




local function uniformBernoulliBanditsSampler(K)
  return function () 
    res = {}
    for i=1,K do
      local p = Stochastic.standardUniform()

      local bandit = Stochastic.new()
      function bandit:sample()
        return 

      table.append(res, stochastic | | Stochastic.bernoulli(p))
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