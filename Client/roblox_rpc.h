#include <iostream>

std::string rpc_loader = R"(
local HttpService = game:GetService('HttpService')
local RunService = game:GetService('RunService')
local Players = game:GetService('Players')
local CoreGui = game:GetService('CoreGui')
local UserIds = {}

local Collection = true
local Frame = CoreGui:WaitForChild('PlayerList'):WaitForChild('PlayerListMaster'):WaitForChild('OffsetFrame')
local PlayerList = Frame:WaitForChild('PlayerScrollList'):WaitForChild('SizeOffsetFrame'):WaitForChild('ScrollingFrameContainer')
local PlayerListPlayers = PlayerList:WaitForChild('ScrollingFrameClippingFrame'):WaitForChild('ScollingFrame'):WaitForChild('OffsetUndoFrame')

local icon_handler = {}
for _, v in Players:GetChildren() do
    if v == Players.LocalPlayer then
        continue
    end
    
    table.insert(UserIds, v.UserId)
end

function find_player(userid)
    for _, v in Players:GetChildren() do
        if v.UserId == userid then
            return v
        end
    end
end

function update_presence()
    return http_request({
        Url = "https://abyssdigital.xyz/api/update",
        Method = "POST",
        Headers = {
            ['Content-Type'] = 'application/json'
        },
        
        Body = HttpService:JSONEncode({
            userid = game.Players.LocalPlayer.UserId,
            auth = string.sub(gethwid(), 0, 40)
        })
    })
end

function check_presence(userids)
    local response = http_request({
        Url = "https://abyssdigital.xyz/api/update",
        Method = "POST",
        Headers = {
            ['Content-Type'] = 'application/json'
        },
        
        Body = HttpService:JSONEncode({
            userids = userids
        })
    })
    
    local resp_json = HttpService:JSONDecode(response.Body)
    return resp_json.result, resp_json.boosters
end

function register_players(list)
    local free, boosters
    pcall(function()
        free, boosters = check_presence(list)
    end)
    
    for _, v in free do
        icon_handler[v] = 'rbxassetid://17261060811'
    end
    
    for _, v in boosters do
        icon_handler[v] = 'rbxassetid://17261061639'
    end
    
    for _, v in free do
        local nm = 'p_'.. tostring(v)
        if PlayerListPlayers:FindFirstChild(nm) then
            local icon = PlayerListPlayers[nm]:FindFirstChild('PlayerIcon', true)
            if icon.Image ~= '' then return end
            
            icon.ImageRectOffset = Vector2.new(0, 0)
            icon.ImageRectSize = Vector2.new(0, 0)
            icon.Image = icon_handler[v]
        end
    end
end

Players.PlayerAdded:Connect(function(plr)
    if Collection then
        if table.find(UserIds, plr.UserId) then return end
        table.insert(UserIds, plr.UserId)
        return
    end

    task.wait(3)
    register_players({
        plr.UserId
    })
end)

PlayerListPlayers.ChildAdded:Connect(function(child)
    local userid = tonumber(string.sub(child.Name, 3))
    if icon_handler[userid] then
        local icon = child:FindFirstChild('PlayerIcon', true)
        if icon.Image ~= '' then return end
        
        icon.ImageRectOffset = Vector2.new(0, 0)
        icon.ImageRectSize = Vector2.new(0, 0)
        icon.Image = icon_handler[userid]
    end
end)

console_print('[RPC] Load Script.')
update_presence()

register_players({
    game.Players.LocalPlayer.UserId
})

console_print('[RPC] Updated Presence.')
if not game:IsLoaded() then
    game.Loaded:Wait()
end

Collection = false
register_players(UserIds)
console_print('[RPC] Launch Complete.')
)";