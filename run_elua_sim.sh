#!/bin/bash

# Setup terminal
stty -echo raw -igncr

# Run simulator
./elua_lua$1_sim.elf

# Restore terminal to default settings
stty echo cooked

