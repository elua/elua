package.path = package.path .. ";../utils/?.lua"
local utils = require "utils.utils"
local sf = string.format

-- List of platforms to build
local ci_boards = {"ek-lm3s8962", "elua-puc", "mbed", "arm2368", "stm32f4discovery"}

-- Variants to build for each platform
-- TODO: is "lualong" also needed here?
local build_variants = {"lua", "lualonglong"}

-- Run a single build command
local function build_single(header, cmd)
    if header then print(utils.col_magenta(sf(">>>>>>>> %s", header))) end
    print(utils.col_magenta(sf(">>>>>>>> Command: %s", cmd)))
    local res = os.execute(cmd)
    if res ~= 0 then os.exit(res) end
end

-- Build the cross-compiler first
build_single("Building cross-compiler", "lua cross-lua.lua")

-- Then the actual targets
for _, board in pairs(ci_boards) do
    for _, variant in pairs(build_variants) do
        local header = sf("Building eLua for %s (%s)", board, variant)
        local cmd = sf("lua build_elua.lua board=%s target=%s", board, variant)
        build_single(header, cmd)
    end
end

