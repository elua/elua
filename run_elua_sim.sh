#!/bin/bash

stty -echo raw -igncr
./elua_lua_linux.elf
stty echo cooked
