require 'Statistics'
require 'IterationFunction'
require 'DiscreteBandit'
require 'Random'
require 'Stochastic'

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

---

subspecified function klUcb(rk, sk, tk, t)
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


-------------- main -----------------

context.randomGenerator = Random.new(1664)

numProblems = 10000
numEstimations = 1
numTimeSteps = 10

-- create training problems
trainingProblems = {}
for i = 1,numProblems do
  local p1 = Stochastic.standardUniform()
  local p2 = Stochastic.standardUniform()
  table.insert(trainingProblems, {Sampler.bernoulli{p = p1}(), Sampler.bernoulli{p = p2}()})
end

-- create policies
local policies = {}
local function addPolicy(name, policy)
  table.insert(policies, {name, policy})
end


addPolicy("KL-ucb c=0", DiscreteBandit.indexBasedPolicy{indexFunction = klUcb{c=0}}.__get)
addPolicy("KL-ucb c=3", DiscreteBandit.indexBasedPolicy{indexFunction = klUcb{c=3}}.__get)

addPolicy("ucb1Tuned", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}.__get)

--[[
addPolicy("ucb1", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1}.__get)
addPolicy("ucb1Tuned", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Tuned}.__get)
addPolicy("ucb1Normal", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1Normal}.__get)
addPolicy("ucb1V", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c = 1, zeta = 1}}.__get)
addPolicy("e-greedy", DiscreteBandit.epsilonGreedy{c = 1, d = 1}.__get)

addPolicy("ucb1_10", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.170}}.__get)
addPolicy("ucb1_100", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.173}}.__get)
addPolicy("ucb1_1000", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucb1C{C=0.187}}.__get)

addPolicy("ucbV_10", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.542, zeta=0.0631}}.__get)
addPolicy("ucbV_100", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.681, zeta=0.0347}}.__get)
addPolicy("ucbV_1000", DiscreteBandit.indexBasedPolicy{indexFunction = DiscreteBandit.ucbV{c=1.304, zeta=0.0852}}.__get)

addPolicy("e-greedy_10", DiscreteBandit.epsilonGreedy{c=0.0499, d=1.505}.__get)
addPolicy("e-greedy_100", DiscreteBandit.epsilonGreedy{c=1.096, d=1.349}.__get)
addPolicy("e-greedy_1000", DiscreteBandit.epsilonGreedy{c=0.845, d=0.738}.__get)
]]

-- evaluate policies
function evaluatePolicy(description, policy, problems)
  context:call(description, DiscreteBandit.estimatePolicyRegretOnProblems, policy, problems, numEstimations, numTimeSteps)
end

for i,t in ipairs(policies) do
  evaluatePolicy(t[1], t[2], trainingProblems)
end