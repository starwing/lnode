local event = require 'event'
local lbind = require 'event.c-lbind'

lbind.info()
--local f = function(...) print("f", ...) end


collectgarbage()
print(collectgarbage"count")
collectgarbage()
print(collectgarbage"count")

local signal = event.new()

collectgarbage()
print(collectgarbage"count")
event.new()
collectgarbage()
print(collectgarbage"count")



print "begin"
collectgarbage()
print(collectgarbage"count")
local signal1 = event.new()
local signal2 = event.new()
collectgarbage()
print(collectgarbage"count")
print "connect"
--signal1:connect(f)
--signal1:connect(f)
--signal2:connect(f)
--signal2:connect(f)
--print "signal"
--signal1(1)
--signal2(2)
--print "signal"
--signal1:disconnect(f)
--signal1(1)
--print "signal"
--signal1:disconnect(f)
--signal1(1)
--print "reset"
--signal2:reset()
--signal2(2)
--print "end"
signal1, signal2, f = nil
collectgarbage()
print(collectgarbage"count")
collectgarbage()
print(collectgarbage"count")
collectgarbage()
print(collectgarbage"count")
collectgarbage()
print(collectgarbage"count")
