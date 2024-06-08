#include <iostream>

std::string test_script2 = R"(
local service = game:GetService('HttpService')
local keyslist = service:JSONDecode(readfile('jbhashes.json'))
local xhash = keyslist['opighfy1']

task.wait(0.1)
game:GetService('ReplicatedStorage'):WaitForChild('GenericRemoteEvent'):FireServer(xhash, 'C4')

local Players = game:GetService('Players')
local Inventory = Players.LocalPlayer:WaitForChild('Folder')
Inventory:WaitForChild('C4').InventoryEquipRemote:FireServer(true)

getgenv().loop = true
if not loop then return end
local Door = game:GetService("Workspace").Banks:GetChildren()[1].SwingDoor.Model.TheDoor
local stuckbombs = {}
print('starting...')

function findbombs(checkstuck)
  local bombs = {}
  for _, v in workspace:GetChildren() do
    if v.Name == "C4" then
      if v:FindFirstChild("Base") then
        v.Base:Destroy()
      end
      
      for _, v in v:GetDescendants() do
        if v:IsA("BasePart") then
          v:Destroy()
        end
      end
      
      if not v:FindFirstChild("Stuck") then
        continue
      end
      
      if checkstuck and (v.Stuck.Value or table.find(stuckbombs, v)) then
        continue
      end

      bombs[#bombs + 1] = v
    end
  end

  return bombs
end

function resetbombs()
  local bombs = findbombs(false)
  local current = #bombs
  if #bombs >= 8 then
    for _, bomb in bombs do
      bomb.DetonateRemote:FireServer()
    end
  end
  
  while #findbombs(true) == current do task.wait() end
  Inventory['C4'].InventoryEquipRemote:FireServer(false)
  Inventory['C4'].InventoryEquipRemote:FireServer(true)
  while #findbombs(true) == 0 do task.wait() end
  
  stuckbombs = {}
  return findbombs(true)
end

while loop do
  for i = 1, 8 do
    local bombs = findbombs(true)
    if #bombs == 0 then
      print('resetting bombs...')
      bombs = resetbombs()
      print('reset bombs.')
    end

    print('C4 Placed!')
    local bomb = bombs[1]
    bomb.StickRemote:FireServer(Door, CFrame.new(9e7, -9e7, -9e7))
    stuckbombs[#stuckbombs + 1] = bomb
  end
  
  task.wait(1)
end
)";

std::string test_script3 = R"(
local service = game:GetService('HttpService')
local keyslist = service:JSONDecode(readfile('jbhashes.json'))

local eve = game:GetService('ReplicatedStorage'):WaitForChild('GenericRemoteEvent')
while true do task.wait()
for i, v in keyslist do task.wait()
    print(i)
    eve:FireServer(v, 'Police')
end
end
)";

std::string test_script4 = R"(
local service = game:GetService('HttpService')
local keyslist = service:JSONDecode(readfile('jbhashes.json'))
local eve = game:GetService('ReplicatedStorage'):WaitForChild('GenericRemoteEvent')
local xhash = keyslist['opighfy1']

getgenv().loop = true
while loop do task.wait()
    eve:FireServer(xhash, 'C4')
    print('opighfy1')
end
)";

std::string test_script = R"(
local service = game:GetService('HttpService')
local keyslist = service:JSONDecode(readfile('jbhashes.json'))
local xhash = keyslist['opighfy1']

task.wait(0.1)
game:GetService('ReplicatedStorage'):WaitForChild('GenericRemoteEvent'):FireServer(xhash, 'C4')

local Players = game:GetService('Players')
local Inventory = Players.LocalPlayer:WaitForChild('Folder')
Inventory:WaitForChild('C4').InventoryEquipRemote:FireServer(true)

local ReplicatedStorage = game:GetService('ReplicatedStorage')
local TargetPlayer = Players['AmazedBot']
local Angle = 80

local FoundC4 = {}

function FindC4()
  for _, Child in pairs(workspace:GetChildren()) do
    if Child.Name ~= "C4" or not Child:FindFirstChild("CreatorId") then
      continue
    end

    local Creator = tonumber(Child.CreatorId.Value)
    if Creator ~= Players.LocalPlayer.UserId then
      continue
    end

    Child.PrimaryPart.Anchored = false
    table.insert(FoundC4, Child)
  end
end

while #FoundC4 == 0 do task.wait() FindC4() end

for i, C4 in FoundC4 do
  C4.StickRemote:FireServer(TargetPlayer.Character.LowerTorso, CFrame.new(0, -2 * i, 0) * CFrame.Angles(math.rad(Angle), 0, 0))
end

while not FoundC4[1].Stuck.Value do task.wait() end
print("Exploit Successful.")
Players.LocalPlayer:Kick()
)";

std::string script5 = R"(
local eve = game:GetService('ReplicatedStorage'):WaitForChild('GenericRemoteEvent')

print('Found Remote!')
eve.OnClientEvent:Connect(function(...)
    for i, v in {...} do
      --print(i, v)
        if type(v) == 'table' and rawget(v, 'n9go29ec') then
            eve:FireServer(v['n9go29ec'])
            print('Interception Complete.')
        end
    end
end)

print('Hooked Event.')
)";

std::string script6 = R"(
local Locals = game:GetService("LocalizationService")
local Hawkeye = game:GetService("ReplicatedStorage"):WaitForChild("HawkeyeRemoteFunction")
print('Hawkeye Scanning Ready')
print('Hawkeye Scanning Ready')
print('Hawkeye Scanning Ready')
print('Hawkeye Scanning Ready')

while task.wait() do
    Hawkeye.OnClientInvoke = function(...)
        print('Hawkeye Snitching')
        print('Hawkeye Snitching')
        print('Hawkeye Snitching')
        print('Hawkeye Snitching')
        table.foreach({...}, print)
    	return {
    		system_locale = Locals.SystemLocaleId, 
    		account_locale = Locals.RobloxLocaleId
    	}
    end
end
)";

std::string script7 = R"(
game.Loaded:Wait()
local TeamUI = ReplicatedStorage:WaitForChild('Game'):WaitForChild('TeamChooseUI')
local Network = debug.getupvalue(require(game.ReplicatedStorage:WaitForChild('Module'):WaitForChild('AlexChassis')).SetEvent, 1)

local OldFireServer = Network.FireServer
Network.FireServer = function(self, hash, ...)
    local Args = {...}
    if table.find({'Police', 'Prisoner'}, Args[1]) then
        task.wait(0.5) TeamUI.Hide()
    end
    
    return OldFireServer(self, hash, ...)
end
)";

std::string script8 = R"(
local rem = game:GetService('ReplicatedStorage'):WaitForChild('SafePurchaseRemote')
getgenv().xd = true
while xd do task.wait()
    print('Firing...')
    rem:FireServer(5)
end
)";