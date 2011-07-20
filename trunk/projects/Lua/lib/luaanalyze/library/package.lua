local set_from_string = require 'luaanalyze.util' . set_from_string

local tpackage = {}

local is_package_field = set_from_string[[
  cpath
  loaded
  loaders
  loadlib
  path
  preload
  seeall
]]


local tpackage = {}
function tpackage:__index(k_ast)
  local name = k_ast[1]
  if is_package_field[name] then -- ok
  else
    error{name .. ' not field of package', k_ast}
  end
end
tpackage.__basetype = 'table'

function tpackage.import()
  globaltypematch('^package$', tpackage)
end
return tpackage


