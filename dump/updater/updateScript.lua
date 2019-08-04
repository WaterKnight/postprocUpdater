local md5lib = require 'libmd5'

dofile(io.local_dir()..'makeChecksums.lua')

local files = {}
local filesC = 0

local function defFile(path)
	path = path:gsub('/', '\\')

	if (files[path] ~= nil) then
		return files[path]
	end

	local file = {}

	files[path] = file
	filesC = filesC + 1

	file.path = path

	return file
end

--local files
local f = io.open('checksums.txt', 'r')

local s = f:read('*a')

f:close()

local t = s:split('\n')

local checksums = {}

for _, line in pairs(t) do
	local checksum, path = line:match('([^%s]+)%s+(.+)')

	if ((path ~= nil) and (checksum ~= nil)) then
		local file = defFile(path)

		file.localChecksum = checksum
	end
end

require 'configParser'

local serverConfig = configParser.create()

serverConfig:readFromFile(io.local_dir()..'server.txt')

local server = serverConfig.assignments['server']

--remote files
local http = require 'socket.http'

local t = {}

local req = {
	host = server,
	path = '/postproc/updater/checksums.txt',
	type = 'i',
	sink = ltn12.sink.table(t)
}

local response, status, header = http.request(req)

assert((status == 200), string.format('could not read remote checksums.txt (%s)', status))

assert((header.location == nil), 'could not read remote checksums.txt (file does not exist)')

--[[local ftp = require 'socket.ftp'

local t = {}

local req = {
	scheme = 'ftp',
	authority = server,
	user = 'anonymous',
	password = 'anonymous',
	host = server,
	path = '/postproc/updater/checksums.txt',
	type = 'i',
	sink = ltn12.sink.table(t)
}

print(ftp.get(req))]]

local t = table.concat(t):split('\n')

for _, line in pairs(t) do
	local checksum, path = line:match('([^%s]+)%s+(.+)')

	if ((path ~= nil) and (checksum ~= nil)) then
		local file = defFile(path)

		file.remoteChecksum = checksum
	end
end

local postprocDir = orient.reduceFolder(io.local_dir())

require 'orient'

orient.addPackagePath(postprocDir..'?')

require 'wx'

local frame = wx.wxFrame(wx.NULL, wx.wxID_ANY, 'postproc updater', wx.wxDefaultPosition, wx.wxSize(500, 100))

local sizer = wx.wxBoxSizer(wx.wxVERTICAL)

local gaugeLabel = wx.wxStaticText(frame, wx.wxID_ANY, '0%')

local gauge = wx.wxGauge(frame, wx.wxID_ANY, filesC)

local curFileLabel = wx.wxStaticText(frame, wx.wxID_ANY, '')

sizer:Add(gaugeLabel, 1, wx.wxEXPAND)
sizer:Add(gauge, 2, wx.wxEXPAND)
sizer:Add(curFileLabel, 1, wx.wxEXPAND)

frame:SetSizer(sizer)
frame:Centre()
frame:Show(true)

local c = 0

for _, file in pairs(files) do
	local pullFile = false

	if (file.remoteChecksum == nil) then
		print('remove', file.path)
		curFileLabel:SetLabel(string.format('remove %s', file.path))
	elseif (file.localChecksum == nil) then
		print('add', file.path)
		pullFile = true
		curFileLabel:SetLabel(string.format('add %s', file.path))
	elseif (file.remoteChecksum ~= file.localChecksum) then
		print('update', file.path)
		pullFile = true
		curFileLabel:SetLabel(string.format('update %s', file.path))
	end

	if pullFile then
		local http = require 'socket.http'

		local t = {}

		local req = {
			host = server,
			path = string.format('/postproc/%s', file.path),
			type = 'i',
			sink = ltn12.sink.table(t)
		}

		local response, status, header = http.request(req)

		if (status == 200) then
			if (header.location == nil) then
				print(string.format('downloaded file %s', file.path))

				local targetPath = postprocDir..file.path

				io.createFile(targetPath, true)

				local f = io.open(targetPath, 'w')

				table.write(f, t)

				f:close()
			else
				print(string.format('failed to downloadfile %s (file does not exist)', file.path))
			end
		else
			print(string.format('failed to download file %s (%s)', file.path, status))
		end
	end

	c = c + 1

	gauge:SetValue(c)
	gaugeLabel:SetLabel(string.format('%.0f%%', c / filesC * 100))
end

curFileLabel:SetLabel('done')

wx.wxGetApp():MainLoop()