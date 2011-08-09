-- Francis Maes, 01/08/2011
-- Samplers (subspecified stochastic functions that have no arguments)

--[[
Interface:

]]

require 'Stochastic'

Sampler = {}

Sampler.Gaussian = subspecified Stochastic.new(
{
  parameter mu = {default = 0},
  parameter sigma = {default = 1, min = 0},

  sample = || Stochastic.standardGaussian:sample() * sigma + mu,
  learn = function (samples)
    local stats = Statistics.meanAndVariance()
    for i,sample in ipairs(samples) do
      stats:observe(sample)
    end
    mu = stats:getMean()
    sigma = stats:getStandardDeviation()
    --print ("mean", mean, "stddev", stddev)
  end,
  expectation = || mu,
})


Sampler.Bernoulli = subspecified Stochastic.new(
{
  parameter p = {default = 0.5, min = 0, max = 1},

  sample = || Stochastic.bernoulli(p),
  learn = function (self, samples)
    local stats = Statistics.mean()
    for i,sample in ipairs(samples) do
      stats:observe(sample)
    end
    p = stats:getMean()
  end,
  expectation = || p
})

Sampler.Truncated = subspecified Stochastic.new(
{
  parameter sampler = {},
  parameter min = {default = 0},
  parameter max = {default = 1},
  
  sample = function ()
    local res = 0
    repeat
      res = sampler()
    until res >= min and res <= max
    return res
  end,
  expectation = sampler.expectation -- FIXME: this is not correct, see http://en.wikipedia.org/wiki/Truncated_normal_distribution
})


function Sampler.TruncatedGaussian(params)
  return Sampler.Truncated{sampler=Sampler.Gaussian(params),min=0,max=1}
end


return Sampler