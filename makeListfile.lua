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

local waterluaPath = orient.toAbsPath('..\\waterlua', orient.local_dir())

assert(waterluaPath, 'no waterlua path found')

orient.addPackagePath(waterluaPath)

orient.requireDir(waterluaPath)

local updaterDir = orient.local_dir()..'Release\\'
local uploadDir = orient.local_dir()..'upload\\'

io.removeDir(uploadDir)
io.copyDir(updaterDir, uploadDir)

io.removeDir(uploadDir..'src\\')

local listfilePath = uploadDir..'listfile.txt'

local f = io.open(listfilePath, 'w+')

f:write('path\tmd5\n')
f:write('--------------------------------\n')

local files = io.getFiles(uploadDir, '*')

for _, path in pairs(files) do
	local shortPath = path:sub(uploadDir:len() + 1, path:len())

	if ((path ~= listfilePath) and (shortPath:sub(1, 1) ~= '.')) then
		require 'libmd5'

		local status, checksum = md5.digest(path)

		if (status == 0) then
			print(string.format('write %s %s', shortPath, checksum))
			f:write(shortPath, '\t', checksum, '\n')
		else
			print(string.format('could not digest %s (%s)', shortPath, checksum))
		end
	end
end

f:close()

os.execute('pause')