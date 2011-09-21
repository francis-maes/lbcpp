require 'Statistics'
require 'IterationFunction'
require 'DiscreteBandit'
require 'Random'
require 'Stochastic'
require 'Sampler'

context.randomGenerator = Random.new(1664)

local minK = 2
local maxK = 10
local maxRewardExpectation = 1
local numBanditProblems = 1000
local horizon = 1000

function makeBanditObjective(minArms, maxArms, numProblems, numEstimationsPerProblem, numTimeSteps)

  local banditProblems = {}
  for i=1,numProblems do
    local K = Stochastic.uniformInteger(minArms, maxArms)
    local problem = {}
    for j=1,K do
      table.insert(problem, Sampler.Bernoulli{p=Stochastic.standardUniform() * maxRewardExpectation})
    end
    table.insert(banditProblems, problem)  
  end

  return function (policy)
    local prevRandom = context.randomGenerator
    context.randomGenerator = Random.new()
    local res = DiscreteBandit.estimatePolicyRegretOnProblems(policy, banditProblems, numEstimationsPerProblem, numTimeSteps)
    context.randomGenerator = prevRandom
    return res
  end
end

local policyObjective = makeBanditObjective(minK, maxK, numBanditProblems, 1, horizon)
local objective = |f| policyObjective(DiscreteBandit.indexBasedPolicy{indexFunction = f}.__get)


-- create policies
local policies = {}
local function addPolicy(name, policy)
  table.insert(policies, {name, policy})
end
local function addIndex(name, indexFunction)
  addPolicy(name, DiscreteBandit.indexBasedPolicy{indexFunction = indexFunction}.__get)
end


addIndex("ucb1", DiscreteBandit.ucb1)
addIndex("ucb1Tuned", DiscreteBandit.ucb1Tuned)
addIndex("ucb1Normal", DiscreteBandit.ucb1Normal)
addIndex("ucb1V", DiscreteBandit.ucbV{c = 1, zeta = 1})
addPolicy("e-greedy", DiscreteBandit.epsilonGreedy{c = 1, d = 1}.__get)
addIndex("KL-ucb c=0", DiscreteBandit.klUcb{c=0})
addIndex("KL-ucb c=3", DiscreteBandit.klUcb{c=3})

addIndex("greedy", |rk,sk,tk,t| rk)
addIndex("rk+1/tk", |rk,sk,tk,t| rk + 1/tk)
addIndex("rk+1.5/tk", |rk,sk,tk,t| rk + 1.5/tk)
addIndex("rk+2/tk", |rk,sk,tk,t| rk + 2/tk)
addIndex("max(8rk.sqrt(tk)+7rk,t)", |rk,sk,tk,t| math.max(8*rk*math.sqrt(tk)+7*rk,t))
addIndex("max(-2,-sk/rk)", |rk,sk,tk,t| math.max(-2, -sk/rk))
addIndex("max(0.2,rk)", |rk,sk,tk,t| math.max(0.2, rk))
addIndex("min(-1/t,log(rk+1/tk))", |rk,sk,tk,t| math.min(-1/t,math.log(rk+1/tk)))
addIndex("log(1.0986-min(rk,tk,log(rk)/3))", |rk,sk,tk,t| math.log(1.0986 - math.min(rk, math.min(tk, math.log(rk)/3))))
addIndex("rk.sqrt(tk)", |rk,sk,tk,t| rk * math.sqrt(tk))
addIndex("rk.(1+sqrt(tk))", |rk,sk,tk,t| rk*(1+math.sqrt(tk)))
addIndex("max(8rk.sqrt(tk)+7rk,t)", |rk,sk,tk,t| math.max(8*rk*math.sqrt(tk)+7*rk,t))


-- evaluate policies
function evaluatePolicy(description, policy)
  context:call(description, policyObjective, policy)
  --context:call(description + " b-test", DiscreteBandit.estimatePolicyRegretOnProblems, policy, problems, numEstimations, numTimeSteps)
  --context:call(description .. " g-test", DiscreteBandit.estimatePolicyRegretOnProblems, policy, gaussianTestProblems, numEstimations, numTimeSteps)
end

for i,t in ipairs(policies) do
  evaluatePolicy(t[1], t[2], testingProblems)
end