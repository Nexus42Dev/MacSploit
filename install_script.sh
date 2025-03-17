#!/bin/bash

echo -e "Downloading MacSploit"
curl -O "https://raw.githubusercontent.com/miloDev/MacSploit/main/MacSploit.zip"

echo -e "Installing MacSploit"
unzip -o -q "./MacSploit.zip"

echo -e "Patching Roblox"
mv ./macsploit.dylib "/Applications/Roblox.app/Contents/MacOS/macsploit.dylib"
./insert_dylib "/Applications/Roblox.app/Contents/MacOS/macsploit.dylib" "/Applications/Roblox.app/Contents/MacOS/RobloxPlayer" --strip-codesig --all-yes
mv "/Applications/Roblox.app/Contents/MacOS/RobloxPlayer_patched" "/Applications/Roblox.app/Contents/MacOS/RobloxPlayer"
rm ./insert_dylib

echo -e "Installing MacSploit App"
[ -d "/Applications/MacSploit.app" ] && rm -rf "/Applications/MacSploit.app"
mv ./MacSploit.app /Applications/MacSploit.app
rm ./MacSploit.zip

echo -e "Install Complete! Developed by milo!"
