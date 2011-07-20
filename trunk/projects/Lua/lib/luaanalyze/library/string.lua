local set_from_string = require 'luaanalyze.util' . set_from_string

local is_string_field = set_from_string[[
  byte
  char
  dump
  find
  format
  gmatch
  gsub
  len
  lower
  match
  rep
  reverse
  sub
  upper
]]


local tformat = {}
function tformat.__call(...)
  local args = {...}
  if #args < 1 then error{#args .. ' args, expected at least 1'} end
  local fmt_ast = ...
  if fmt_ast.type and fmt_ast.type.__basictype and
     fmt_ast.type.__basictype ~= 'string'
  then
    error{"got " .. tostring(fmt_ast.type.__basictype) .. ", expected string", fmt_ast}
  end
end

local tstring = {}
function tstring:__index(k_ast)
  local name = k_ast[1]
  if name == 'format' then
    return tformat
  elseif is_string_field[name] then -- ok
  else
    error{name .. ' not field of string', k_ast}
  end
end
tstring.__basetype = 'table'

function tstring.import()
  globaltypematch('^string$', tstring)
end
return tstring

