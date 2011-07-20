-- checker.lua
--
-- Static analysis of Lua code.
--
-- (c) 2008 David Manura.  Licensed under the same terms as Lua (MIT license).

--! local t_G = typeimport 'luaanalyze.library.standard'
--! local t_ast = require 'luaanalyze.type.ast'
--! t_ast.add_field("nonconst")
--! t_ast.add_field("type")
--! typematch('ast$', t_ast)
--! t_G.allow_write 'typeimport'
--! t_G.allow_write 'typematch'
--! t_G.allow_write 'globaltypematch'
--! t_G.allow_write 'checkglobals'
--! t_G.allow_write 'checktypes'
--! t_G.add_field("current_ast")
--! t_G.add_field("current_istart")
--! checkglobals()
--! checktypes()

local M = {}

local W = require "luaanalyze.walk"

-- Whether types propagates in current type propagation iteration.
-- Used to detect convergence.
local is_changing

-- Sets types of AST node <ast> to <atype>.
-- Updates is_changing flag.
local function set_type(ast, atype)
  if atype ~= nil and ast.type == nil then
    is_changing = true
  end
  ast.type = atype
end


local function typeimport(name)
  local mod = require(name)
  mod.import()
  return mod
end
M.typeimport = typeimport
_G.typeimport = typeimport


local function typematch(pattern, atype)
  local ast    = _G.current_ast
  local istart = _G.current_istart

  if type(atype) == 'string' then
    atype = require(atype)
  end
  W.find_vars_scoped(ast, istart, {getvar=function(ast, def_ast)
    local name = ast[1]
    --print('DEBUG:try', pattern, name, name:match(pattern), def_ast)
    if def_ast and name:match(pattern) then
      set_type(ast, atype)
    end
  end})
end
M.typematch = typematch
_G.typematch = typematch


local function globaltypematch(pattern, atype)
  local ast    = _G.current_ast
  local istart = _G.current_istart

  if type(atype) == 'string' then
    atype = require(atype)
  end
  W.find_vars_scoped(ast, istart, {getvar=function(ast, def_ast)
    local name = ast[1]
    --print('DEBUG:try', pattern, name, name:match(pattern))
    if not def_ast and name:match(pattern) then
      set_type(ast, atype)
    end
  end})
end
M.globaltypematch = globaltypematch
_G.globaltypematch = globaltypematch


local function checkglobals()
  local ast    = _G.current_ast
  local istart = _G.current_istart

  W.find_vars_scoped(ast, istart, {getvar=function(ast, def_ast)
    if not def_ast then
      if ast.type == nil then
        local name = ast[1]
        error{'undefined global \"' .. name .. '\"', ast}
      end
    end
  end})
end
M.checkglobals = checkglobals
_G.checkglobals = checkglobals


-- Walks AST, marking each definition of a lexical variable that is
-- non-constant (i.e. modified in another statement).
local function mark_nonconst(ast)
  local callback = {}
  function callback.setvar(ast, val_ast, def_ast)
    --print('setv', ast[1], def_ast)
    if def_ast then
      def_ast.nonconst = true
    end
  end
  W.find_vars_scoped(ast, 0, callback)
end


-- Walks AST, propagating types inside expressions.
local function propagate_expressions(ast)

  local callback = {}

  function callback.onnode(ast)
    if ast.tag == 'Call' then
      local type1 = ast[1].type
      if type1 then
        if type1.__call then
          local tresult = type1.__call(unpack(ast, 2))
          set_type(ast, tresult)
        end
      end
    elseif ast.tag == 'Index' then
      local type1 = ast[1].type
      if type1 then
        if type1.__index then
          local tresult = type1.__index(ast[1], ast[2])
          set_type(ast, tresult)
        end
      end
    --:FIX:add more
    end
  end

  W.find_vars(ast, 0, callback)
end


-- Walks AST, performing type checks.
local function checktypes(ast)
  ast = ast or _G.current_ast

  mark_nonconst(ast)

  repeat
    is_changing = false

    -- propagate types from values to constant variables they are assigned to.
    W.find_vars_scoped(ast, 0, {
      newvar = function(ast, val_ast)
        --print('V',ast, ast.nonconst)
        if not ast.nonconst and val_ast and val_ast.type then
          --print('DEBUG:prop', ast[1], val_ast.type.__basetype)
          set_type(ast, val_ast.type)
        end
      end
    })

    -- propagate types from definition to uses
    W.find_vars_scoped(ast, 0, {
      getvar = function(ast, def_ast)
        if def_ast and def_ast.type then
          set_type(ast, def_ast.type)
        end
      end
    })

    -- propagate types inside expressions
    propagate_expressions(ast)

    --print('DEBUG:changing?', is_changing)

  until not is_changing

end
M.checktypes = checktypes
_G.checktypes = checktypes

return M

