require 'Statistics'
require 'IterationFunction'
require 'DiscreteBandit'

---------- evaluation -----------------

-- returns the sum of reward expectations
function playBanditGame(policy, bandits, numTimeSteps)
  local res = 0.0
  local co = coroutine.create(policy)
  local info = #bandits
  for i = 1,numTimeSteps do
    local ok, bi = coroutine.resume(co, info)
    assert(ok)
    res = res + bandits[bi]:expectation()
    --print ("Bi = ", bi)
    local ri = bandits[bi]:sample()  -- sample from bandit distribution
    info = ri
    --print ("RI = ", info)
  end
  return res  
end

function evaluatePolicyOnProblem(policy, bandits, numEstimations, numTimeSteps)

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
    stats:observe(playBanditGame(policy, bandits, numTimeSteps))
  end
  -- return regret  
  return bestExpectedReward * numTimeSteps - stats:getMean()
  
end

function evaluatePolicyOnProblems(policy, banditProblems, numEstimationsPerProblem, numTimeSteps)

  local regret = Statistics.mean()
  for i,problem in ipairs(banditProblems) do
    regret:observe(evaluatePolicyOnProblem(policy, problem, numEstimationsPerProblem, numTimeSteps))
  end
  return regret:getMean()
end


------------- Sampler ---------------

Sampler = {}

--stochastic subspecified function Sampler.bernoulli()
--  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}
--  return Stochastic.bernoulli(p)
--end


-- FIXME: Problem: we cannot directly declare a subspecified table, because there would be no place
--  do put the parameter statements so that they can be used in different fields of the table
-- apriori, we need to extend the grammar of tables to allow parameter statements inside ....
Sampler.bernoulli = subspecified function ()
  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}

  return setmetatable({
    sample = function () return Stochastic.bernoulli:sample(p) end,
    expectation = function () return p end,
  }, Stochastic.MT)
end

--subspecified function Sampler.bernoulli()
--  parameter p = {default = 0.5, min = 0, max = 1, sampler = Stochastic.standardUniform}
--  return Stochastic.bernoulli:sample(p)
--end  


-------------- main -----------------

numProblems = 100
numEstimations = 10
numTimeSteps = 10

policy1 = DiscreteBandit.randomPolicy
policy2 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.greedy}
policy3 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1}
policy4 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}
policy5 = DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=2,zeta=5}}

bandits = {Sampler.bernoulli{p = 0.9}(), Sampler.bernoulli{p = 0.6}()}

trainingProblems = {}
for i = 1,numProblems do
  local p1 = Stochastic.standardUniform()
  local p2 = Stochastic.standardUniform()
  table.insert(trainingProblems, {Sampler.bernoulli{p = p1}(), Sampler.bernoulli{p = p2}()})
end

function evaluatePolicy(description, policy, problems)
   context:call(description, evaluatePolicyOnProblems, policy, problems, numEstimations, numTimeSteps)
end

evaluatePolicy("Evaluate policy random", policy1, trainingProblems)
evaluatePolicy("greedy", policy2, trainingProblems)
evaluatePolicy("ucb1(2)", policy3, trainingProblems)
evaluatePolicy("ucb1Tuned", policy4, trainingProblems)
evaluatePolicy("ucbv(2,5)", policy5, trainingProblems)