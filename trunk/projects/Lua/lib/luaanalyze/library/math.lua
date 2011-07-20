local set_from_string = require 'luaanalyze.util' . set_from_string

local tsqrt = {}
function tsqrt.__call(...)
  assert(select('#', ...) == 1, 'wrong num args')
  local val_ast = ...
  if val_ast.type and val_ast.type.__basictype and
     val_ast.type.__basictype ~= 'number'
  then
    error("got " .. tostring(val_ast.type.__basictype) .. " expected number")
  end
end

local is_math_field = set_from_string[[
  abs
  acos
  asin
  atan
  atan2
  ceil
  cos
  cosh
  deg
  exp
  floor
  fmod
  frexp
  huge
  ldexp
  log
  log10
  max
  min
  modf
  pi
  pow
  rad
  random
  randomseed
  sin
  sinh
  sqrt
  tan
  tanh
]]

local tmath = {}
function tmath:__index(k_ast)
  local name = k_ast[1]
  if is_math_field[name] then -- ok
  else
    error{name .. ' not field of math', k_ast}
  end
end
tmath.__basetype = 'table'

function tmath.import()
  globaltypematch('^math$', tmath)
end
return tmath
