local assert, ipairs, pairs, rawget, require, table, type, setmetatable
    = assert, ipairs, pairs, rawget, require, table, type, setmetatable

local M = {}
local pbox = setmetatable({}, {__mode = "v"})
local node_meta, evt_meta = {}, {}
local node = require 'c-node.rawnode' (pbox, node_meta)
local event = require 'c-node.event'  (pbox, evt_meta)

local isnode           = node.isnode
local utable           = node.utable
local node_rawnode     = node.rawnode
local node_init        = node.init
local node_setut       = node.setut
local node_delete      = node.delete
local node_append      = node.append
local node_insert      = node.insert
local node_setchildren = node.setchildren
local node_removeself  = node.removeself

local event_new                = event.new
local event_eventnode          = event.eventnode
local event_addeventhandler    = event.addeventhandler
local event_removeeventhandler = event.removeeventhandler

local function idx(self)
    local parent = self.parent
    if parent then return assert(utable(parent)[self]) end
end

local query_funcs = {
    idx         = idx,
    type        = node.type,
    parent      = node.parent,
    prevsibling = node.prevsibling,
    nextsibling = node.nextsibling,
    firstchild  = node.firstchild,
    lastchild   = node.lastchild,
    firstleaf   = node.firstleaf,
    lastleaf    = node.lastleaf,
    prevleaf    = node.prevleaf,
    nextleaf    = node.nextleaf,
}

for k, v in pairs(query_funcs) do
    M[k] = v
end

node_meta.__tostring = node.tostring
node_meta.__gc = node.delete

function node_meta:__len()
    return utable(self).n or 0
end

function node_meta:__newindex(k, v)
    if type(k) ~= 'number' then
        if not query_funcs[k] then node_setut(ut, k, v) end
        return error("attempt set read-only field "..k)
    end
    local ut = utable(self)
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

function node_meta:__index(k)
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

function node_meta.__ipairs(node)
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

function M.new(func, tbl)
    if not tbl then
        func, tbl = M.rawnode, func
    end
    local nobj = node_init(func(), tbl)
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

function M.rawnode()
    local nobj = node_rawnode()
    utable(nobj).type = "raw"
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

return setmetatable(M, {__call = function(t, ...) return M.new(...) end})
