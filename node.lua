local assert       = assert
local ipairs       = ipairs
local pairs        = pairs
local rawget       = rawget
local require      = require
local setmetatable = setmetatable
local type         = type

local M = {}
local pbox, meta = {}, {}
local node = require 'c-node' (pbox, meta)

local isnode           = node.isnode
local utable           = node.utable
local node_new         = node.new
local node_delete      = node.delete
local node_append      = node.append
local node_insert      = node.insert
local node_setchildren = node.setchildren
local node_removeself  = node.removeself
local node_parent      = node.parent
local node_prevsibling = node.prevsibling
local node_nextsibling = node.nextsibling
local node_firstchild  = node.firstchild
local node_lastchild   = node.lastchild
local node_firstleaf   = node.firstleaf
local node_lastleaf    = node.lastleaf
local node_prevleaf    = node.prevleaf
local node_nextleaf    = node.nextleaf

local query_funcs = {
    parent      = node_parent,
    prevsibling = node_prevsibling,
    nextsibling = node_nextsibling,
    firstchild  = node_firstchild,
    lastchild   = node_lastchild,
    firstleaf   = node_firstleaf,
    lastleaf    = node_lastleaf,
    prevleaf    = node_prevleaf,
    nextleaf    = node_nextleaf,
}

for k, v in pairs(query_funcs) do
    M[k] = v
end

setmetatable(pbox, {__mode = "v"})

meta.__tostring = node.tostring
meta.__gc = node.delete

function meta:__len()
    return utable(self).n or 0
end

function meta:__newindex(k, v)
    local ut = utable(self)
    if type(k) ~= 'number' then
        if not query_funcs[k] then 
            return rawset(ut, k, v)
        end
    end
    if k == ut.n and not v then
        return M.delete(ut[k])
    end
    if k < 0 then k = k + ut.n + 1 end
    if (k >= 1 and k <= ut.n) and ut[k] ~= v then
        local vut = utable(v)
        local dk = vut.parent == self and ut[v] < k and 1 or 0
        M.delete(ut[k])
        M.insertchild(self, v, k-dk)
    end
end

function meta:__index(k)
    local ut = utable(self)
    local v = ut[k]
    if v then return v end
    local f = query_funcs[k]
    if f then return f(self) end
    if type(k) == 'number' and k < 0 then
        local v = ut[k + ut.n + 1]
        if v then return v end
    end
    return rawget(M, k)
end

function meta.__ipairs(node)
    return ipairs(utable(node))
end

local function remove_from_parent(node)
    local ut = utable(node)
    local parent = ut.parent
    ut.parent = nil
    if parent then
        local ut = utable(parent)
        local idx = assert(ut[node])
        ut[node] = nil
        table.remove(ut, idx)
        local n = ut.n-1
        for i = idx, n do
            ut[ut[i]] = i
        end
        ut.n = n
    end
end

function M.new(tbl)
    local nobj = node_new(tbl)
    local children
    local last = 0
    for i, v in ipairs(tbl) do
        if not isnode(v) then break end
        children = node_append(children, v)
        utable(v).parent = nobj
        tbl[v] = i
        last = i
    end
    node_setchildren(nobj, children)
    tbl.n = last
    -- clear all other number value
    for k, v in pairs(tbl) do
        if type(k) == 'number' and (k > last or k < 1) then
            tbl[k] = nil
        end
    end
    return nobj
end

function M:delete()
    remove_from_parent(self)
    node_removeself(self)
end

function M:purge()
    for i, v in ipairs(self) do
        purge(v)
    end
    self:delete()
end

function M:appendchild(newnode)
    local ut = utable(self)
    local nut = utable(assert(newnode))
    remove_from_parent(newnode)
    node_append(ut[1], newnode)
    nut.parent = self
    local n = ut.n+1
    ut[n] = newnode
    ut[newnode] = n
    ut.n = n
end

function M:insertchild(newnode, idx)
    local ut = utable(self)
    local n = ut.n+1
    local idx = idx or 1
    if idx < 0 then idx = idx + n end
    if idx < 0 or idx > n then return end
    local begin = idx
    if ut[assert(newnode)] then
        local ridx = ut[newnode]
        table.remove(ut, ridx)
        if ridx < idx then
            idx = idx - 1
            begin = ridx
        end
        n = n - 1
    else
        remove_from_parent(newnode)
    end
    node_insert(ut[idx], newnode)
    table.insert(ut, idx, newnode)
    for i = begin, n do
        ut[ut[i]] = i
    end
    ut.n = n
    local nut = utable(newnode)
    nut.parent = self
end

function M:idx()
    local parent = self.parent
    if parent then return assert(utable(parent)[self]) end
end

return setmetatable(M, {__call = function(t, ...) return M.new(...) end})
