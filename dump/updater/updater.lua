local function script_path()
	local str = debug.getinfo(2, "S").source:sub(2)

	str = str:gsub('/', '\\')

	local dir = str:match("(.*\\)")

	if (dir == nil) then
		return ''
	end

	return dir
end

package.path = script_path()..'?.lua'..';'..package.path

require 'orient'

local configPath = orient.reduceFolder(orient.toAbsPath(script_path()))..'postproc_getconfigs.lua'

local config = dofile(configPath)

local waterluaPath = orient.toAbsPath(config.assignments['waterlua'], orient.getFolder(configPath))

assert(waterluaPath, 'no waterlua path found')

orient.addPackagePath(waterluaPath)

orient.requireDir(waterluaPath)

local f
local useRemoteScript = false

if useRemoteScript then
	local http = require 'socket.http'
	local ltn2 = require 'ltn12'

	local t = {}

	local req = {
		url = 'http://www.moonlightflower.net/index.html',
		sink = ltn12.sink.table(t)
	}

	local response, status, header = http.request(req)

	local s = table.concat(t)

	f = loadstring(s)
else
	f = loadfile(io.local_dir()..'updateScript.lua')

	if (f == nil) then
		print(loadfile(io.local_dir()..'updateScript.lua'))
	end
end

f()

os.execute("pause")