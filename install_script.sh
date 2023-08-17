#!/bin/bash

echo -e "Downloading MacSploit"
curl -O "https://cdn.discordapp.com/attachments/1100029005819813928/1141593315205201981/MacSploit.zip"

echo -e "Installing MacSploit"
unzip ./MacSploit.zip
sleep 0.5

echo -e "Patching Roblox"
mv ./macsploit.dylib "/Applications/Roblox.app/Contents/MacOS/macsploit.dylib"
./insert_dylib "/Applications/Roblox.app/Contents/MacOS/macsploit.dylib" "/Applications/Roblox.app/Contents/MacOS/RobloxPlayer" --strip-codesig --all-yes
mv "/Applications/Roblox.app/Contents/MacOS/RobloxPlayer_patched" "/Applications/Roblox.app/Contents/MacOS/RobloxPlayer"
rm ./insert_dylib

echo -e "Installing MacSploit App"
[ -d "/Applications/MacSploit.app" ] && rm -rf "/Applications/MacSploit.app"
mv ./MacSploit.app /Applications/MacSploit.app
rm ./MacSploit.zip

echo -e "Install Complete! Developed by Nexus42!"
