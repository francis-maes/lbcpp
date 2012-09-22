hop = subspecified 51

print (hop)


subspecified function toto(x)
  parameter constant1 = {default = 51}
  parameter constant2 = {default = 1}
  return constant1 + constant2 * x
end

print (toto)

totoInstance = toto{constant2 = 8.6}

print (totoInstance)

print (totoInstance(2))