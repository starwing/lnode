local event = require 'event'
local lbind = require 'event.c-lbind'

local f = function(...) print("f", ...) end


local signal1 = event.new()
local signal2 = event.new()
print "connect"
signal1:connect(f)
signal1:connect("abc", f)
signal1:connect(function()error"abc"end)
signal1:connect(function()error"def"end)
signal2:connect(f)
signal2:connect(f)
print "signal"
print(pcall(signal1, 1))
signal1("abc", "abc")
print(pcall(signal1, "def", "abc"))
signal2(2)
signal1:reset()
signal1:connect(f, f, f, f)
print "signal"
signal1:disconnect(f, f)
signal1(1)
print "signal"
signal1:disconnect(f, f)
signal1(1)
print "reset"
signal2:reset()
signal2(2)
print "end"
