#!/bin/bash

echo -e "Downloading Latest Roblox"
local version = $(curl -s http://setup.roblox.com/mac/version)
curl "http://setup.rbxcdn.com/mac/$version-RobloxPlayer.zip" -o "./RobloxPlayer.zip"
