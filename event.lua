local M = {}
local pbox = setmetatable({}, {__mode = "v"})
local sign_meta, slot_meta = {}, {}
local event = require 'c-node.event' (pbox, sign_meta, slot_meta)

local event_newsignal    = event.newsignal
local event_newslot      = event.newslot
local event_initsignal   = event.initsignal
local event_initslot     = event.initslot
local event_deletesignal = event.deletesignal
local event_deleteslot   = event.deleteslot
local event_emit         = event.emit
local event_callback     = event.callback
local event_conenct      = event.conenct
local event_disconnect   = event.disconnect

local sign_libs = {}
local slot_libs = {}

sign_meta.__tostring = event.tostring
sign_meta.__gc = event_deletesignal
sign_meta.__index = sign_libs
slot_meta.__tostring = event.tostring
slot_meta.__gc = event_deleteslot
slot_meta.__index = slot_libs

function M:signal()
end

function M:slot(f)
end

function sign_libs:connect()
end

function sign_libs:emit()
end

function slot_libs:call(...)
    return event_callback(self)(..., event_getsignal(self), self, select(2, ...))
end

function slot_libs:disconnect()
end

setmetatable(sign_libs, {__call = sign_libs.emit }) 
setmetatable(slot_libs, {__call = sign_libs.call }) 

return M
