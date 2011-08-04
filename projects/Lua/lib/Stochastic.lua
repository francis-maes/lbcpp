-- Francis Maes, 03/08/2011
-- Stochastic extension

--[[
Interface:

  A stochastic funtion is a table implementing:
 
    - sample(...)           the sampling function

  For number and vector samplers:

    - expectation(...)      expectation function (==> deterministic version of the function)

]]

module("Stochastic", package.seeall)

MT = {} -- Metatable
 
function MT.__call(tbl, ...)
  return tbl:sample(...)
end

function new()
  return setmetatable({}, MT)
end

--
-- Standard Uniform ([0,1])
--
standardUniform = new()

function standardUniform:sample()
  return context:random()
end

function standardUniform:expectation()
  return 0.5
end

--
-- Uniform integer ([a,b])
--
uniformInteger = new()

function uniformInteger:sample(a, b)
  if type(a) ~= "number" or type(b) ~= "number" then
    error("Invalid arguments in uniformInteger:sample()")
  elseif a > b then
    error("min (" .. a .. ") should be lower than max (" .. b .. ")")
  end
    
  if a == b then
    return a
  else
    local res = a + math.floor(context:random() * (b - a + 1))
    assert (res >= a and res <= b)
    return res
  end
end

function uniformInteger:expectation(a, b)
  return a + (b - a) / 2
end


--
-- Standard Gaussian 
--
standardGaussian = new()
standardGaussian.readySample = nil

-- sampling function,
-- from http://www.taygeta.com/random/gaussian.html
function standardGaussian:sample()
  if self.readySample then
    local res = self.readySample
    self.readySample = nil
    return res
  else
    local w = 0
    local x1 = 0
    local x2 = 0
    repeat
      x1 = 2 * context:random() - 1
      x2 = 2 * context:random() - 1
      w = x1 * x1 + x2 * x2
    until w < 1
    w = math.sqrt((-2 * math.log(w)) / w);
    self.readySample = x2 * w
    return x1 * w
  end
end

function standardGaussian:expectation()
  return 0
end


--
-- Bernoulli
--
bernoulli = new()

function bernoulli:sample(p)
  return context:random() < p and 1 or 0
end

function bernoulli:expectation(p)
  return p
end