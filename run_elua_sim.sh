#!/bin/bash

# Setup terminal
stty -echo raw -igncr

# Run simulator
./elua_lua$1_linux.elf

# Restore terminal to default settings
stty echo cooked

