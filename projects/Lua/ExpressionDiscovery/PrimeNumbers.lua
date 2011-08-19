local cachedPrimeNumbers = {}

function math.isPrime(x)
  if x < 2 then
        return false
  end
  if x == 2 then
        return true
  end
  if x % 2 == 0 then
    return false
  end
  if x < 9 then
    return true
  end
  if (x + 1) % 6 ~= 0 then
    if (x - 1) % 6 ~= 0 then
      return false
    end
  end

  function impl(x)
    local lim = math.sqrt(x) + 1.0
    for y=3,lim,2 do
      if x % y == 0 then
        return false
      end
    end
    return true
  end

  res = cachedPrimeNumbers[x]
  if res == nil then
    res = impl(x)
    cachedPrimeNumbers[x] = res
  end
  return res
end


local function f(x)
  return x * x - 3 * x + 43
end


local res = 0
local lastPrime = -math.huge
for i=0,49 do
  local fi = f(i)
  if math.isPrime(fi) then
    if fi > lastPrime then
      res = res + 1
    else
      res = 1
    end
    lastPrime = fi
  end
  print (i,f(i), math.isPrime(f(i)), res)
end
print(res)

