--
-- Copyright (C) 2024 Kirill Sharov <sheriffkorov@altlinux.org>
--
-- This file is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
--

if not transaction_success and transaction_success ~= nil then
	return
end

local filename = nil
if script_slot == "Scripts::AptGet::DistUpgrade" then
	filename = confget("Scripts::AlteratorLogger::UpgradesLogFile/f", "/var/log/alterator/apt/dist-upgrades.log")
end

if filename == nil then
	return
end

local file = io.open(filename, "a+")
if not file then
	print("error: can't open log file at " .. filename)
	return
end
file:write(os.date("!%Y-%m-%d %H:%M:%S"), " UTC\n")
file:close()
