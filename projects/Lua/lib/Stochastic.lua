-- Francis Maes, 03/08/2011
-- Stochastic extension

--[[
Interface:

  A stochastic funtion is a table implementing:
 
    - sample(...)           the sampling function

  For number and vector samplers:

    - expectation(...)      expectation function (==> deterministic version of the function)

]]

Stochastic = {}

Stochastic.MT = {} -- Metatable
 
function Stochastic.MT.__call(tbl, ...)
  return tbl:sample(...)
end

function Stochastic.new(table)
  return setmetatable(table, Stochastic.MT)
end

--
-- Standard Uniform ([0,1])
--
Stochastic.standardUniform = Stochastic.new({})

function Stochastic.standardUniform:sample()
  return context:random()
end

function Stochastic.standardUniform:expectation()
  return 0.5
end

--
-- Uniform integer ([a,b])
--
Stochastic.uniformInteger = Stochastic.new({})

function Stochastic.uniformInteger:sample(a, b)
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

function Stochastic.uniformInteger:expectation(a, b)
  return a + (b - a) / 2
end


--
-- Standard Gaussian 
--
Stochastic.standardGaussian = Stochastic.new({})
Stochastic.standardGaussian.readySample = nil

-- sampling function,
-- from http://www.taygeta.com/random/gaussian.html
function Stochastic.standardGaussian:sample()
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

function Stochastic.standardGaussian:expectation()
  return 0
end


--
-- Bernoulli
--
Stochastic.bernoulli = Stochastic.new({})

function Stochastic.bernoulli:sample(p)
  return context:random() < p and 1 or 0
end

function Stochastic.bernoulli:expectation(p)
  return p
end

--
-- UniformOrder
--
Stochastic.uniformOrder = Stochastic.new({})

function Stochastic.uniformOrder:sample(n)
  local function shuffle(t)
    local n = #t
  
    while n >= 2 do
      -- n is now the last pertinent index
      local k = Stochastic.uniformInteger(1,n)
      -- Quick swap
      t[n], t[k] = t[k], t[n]
      n = n - 1
    end
  
    return t
  end
  local res = {}
  for i=1,n do table.insert(res, i) end
  return shuffle(res)
end

--
--
--
return Stochastic