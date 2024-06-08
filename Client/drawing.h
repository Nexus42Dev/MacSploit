#include <iostream>
#ifndef DRAWING
#define DRAWING

// Skidded from Argon

std::string drawing_test = R"(
local DrawingLib = {}

local Camera = game:GetService('Workspace'):WaitForChild('Camera')
local RunService = game:GetService('RunService')

local BaseDrawingProperties = setmetatable({
	Visible = false,
	Color = Color3.new(),
	Transparency = 0,
	Remove = function()
	end
}, {
	__add = function(tbl1, tbl2)
		local new = {}
		for i, v in next, tbl1 do
			new[i] = v
		end
		for i, v in next, tbl2 do
			new[i] = v
		end
		return new
	end
})

local DrawingUI = nil;

DrawingLib.new = function(Type)
	if DrawingUI == nil then
		DrawingUI = Instance.new('ScreenGui');
		DrawingUI.Parent = gethui();
		DrawingUI.Name = 'DrawingLib'
		DrawingUI.DisplayOrder = 1999999999
		DrawingUI.IgnoreGuiInset = true
	end

	if (Type == 'Line') then
		local LineProperties = ({
			To = Vector2.new(),
			From = Vector2.new(),
			Thickness = 1,
		} + BaseDrawingProperties)

		local LineFrame = Instance.new('Frame');
		LineFrame.AnchorPoint = Vector2.new(0.5, 0.5);
		LineFrame.BorderSizePixel = 0

		LineFrame.BackgroundColor3 = LineProperties.Color
		LineFrame.Visible = LineProperties.Visible
		LineFrame.BackgroundTransparency = LineProperties.Transparency


		LineFrame.Parent = DrawingUI

		return setmetatable({}, {
					__newindex = (function(self, Property, Value)
				if (Property == 'To') then
					local To = Value
					local Direction = (To - LineProperties.From);
					local Center = (To + LineProperties.From) / 2
					local Distance = Direction.Magnitude
					local Theta = math.atan2(Direction.Y, Direction.X);

					LineFrame.Position = UDim2.fromOffset(Center.X, Center.Y);
					LineFrame.Rotation = math.deg(Theta);
					LineFrame.Size = UDim2.fromOffset(Distance, LineProperties.Thickness);

					LineProperties.To = To
				end
				if (Property == 'From') then
					local From = Value
					local Direction = (LineProperties.To - From);
					local Center = (LineProperties.To + From) / 2
					local Distance = Direction.Magnitude
					local Theta = math.atan2(Direction.Y, Direction.X);

					LineFrame.Position = UDim2.fromOffset(Center.X, Center.Y);
					LineFrame.Rotation = math.deg(Theta);
					LineFrame.Size = UDim2.fromOffset(Distance, LineProperties.Thickness);


					LineProperties.From = From
				end
				if (Property == 'Visible') then
					LineFrame.Visible = Value
					LineProperties.Visible = Value
				end
				if (Property == 'Thickness') then
					Value = Value < 1 and 1 or Value

					local Direction = (LineProperties.To - LineProperties.From);
					local Distance = Direction.Magnitude

					LineFrame.Size = UDim2.fromOffset(Distance, Value);

					LineProperties.Thickness = Value
				end
				if (Property == 'Transparency') then
					LineFrame.BackgroundTransparency = 1 - Value
					LineProperties.Transparency = Value
				end
				if (Property == 'Color') then
					LineFrame.BackgroundColor3 = Value
					LineProperties.Color = Value 
				end
				if (Property == 'ZIndex') then
					LineFrame.ZIndex = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						LineFrame:Destroy();
					end)
				end
                if Property == 'Destroy' then
                 return (function()
						LineFrame:Destroy();
					end)
                end
				return LineProperties[Property]
			end)
		})
	end

	if (Type == 'Circle') then
		local CircleProperties = ({
			Radius = 150,
			Filled = false,
			Position = Vector2.new()
		} + BaseDrawingProperties)

		local CircleFrame = Instance.new('Frame');

		CircleFrame.AnchorPoint = Vector2.new(0.5, 0.5);
		CircleFrame.BorderSizePixel = 0

		CircleFrame.BackgroundColor3 = CircleProperties.Color
		CircleFrame.Visible = CircleProperties.Visible
		CircleFrame.BackgroundTransparency = CircleProperties.Transparency

		local Corner = Instance.new('UICorner', CircleFrame);
		Corner.CornerRadius = UDim.new(1, 0);
		CircleFrame.Size = UDim2.new(0, CircleProperties.Radius, 0, CircleProperties.Radius);

		CircleFrame.Parent = DrawingUI

		local Stroke = Instance.new('UIStroke', CircleFrame)
		Stroke.Thickness = 2
		Stroke.Enabled = false

		return setmetatable({}, {
			__newindex = (function(self, Property, Value)
				if (Property == 'Radius') then
					CircleFrame.Size = UDim2.new(0, Value, 0, Value);
					CircleProperties.Radius = Value
				end
				if (Property == 'Position') then
					CircleFrame.Position = UDim2.new(0, Value.X, 0, Value.Y + 36);
					CircleProperties.Position = Value
				end
				if (Property == 'Filled') then
					CircleFrame.BackgroundTransparency = Value == true and 0 or 1
					Stroke.Enabled = not Value
					CircleProperties.Filled = Value
				end
				if (Property == 'Color') then
					CircleFrame.BackgroundColor3 = Value
					Stroke.Color = Value
					CircleProperties.Color = Value
				end
				if (Property == 'Visible') then
					CircleFrame.Visible = Value
					CircleProperties.Visible = Value
				end
				if (Property == 'ZIndex') then
					CircleFrame.ZIndex = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						CircleFrame:Destroy();
					end)
				end
                if Property ==  'Destroy' then
                return (function()
						CircleFrame:Destroy();
					end)
                end
				return CircleProperties[Property]
			end)
		})
	end

	if (Type == 'Text') then
		local TextProperties = ({
			Text = '',
			Size = 0,
			Center = false,
			Outline = false,
			OutlineColor = Color3.new(),
			Position = Vector2.new(),
		} + BaseDrawingProperties)

		local TextLabel = Instance.new('TextLabel');

		TextLabel.AnchorPoint = Vector2.new(0.5, 0.5);
		TextLabel.BorderSizePixel = 0
		TextLabel.Size = UDim2.new(0, 200, 0, 50);
		TextLabel.Font = Enum.Font.SourceSans
		TextLabel.TextSize = 14

		TextLabel.TextColor3 = TextProperties.Color
		TextLabel.Visible = TextProperties.Visible
		TextLabel.BackgroundTransparency = 1
		TextLabel.TextTransparency = 1 - TextProperties.Transparency
		
		local Stroke = Instance.new('UIStroke', TextLabel)
		Stroke.Thickness = 0.5
		Stroke.Enabled = false
		Stroke.Color = Color3.fromRGB(0, 0, 0)

		TextLabel.Parent = DrawingUI

		return setmetatable({}, {
			__newindex = (function(self, Property, Value)
				if (Property == 'Text') then
					TextLabel.Text = Value
					TextProperties.Text = Value
				end
				if (Property == 'Position') then
					TextLabel.Position = UDim2.new(0, Value.X, 0, Value.Y + 36);
					TextProperties.Position = Value
				end
				if (Property == 'Size') then
					TextLabel.TextSize = Value
					TextProperties.Size = Value
				end
				if (Property == 'Color') then
					TextLabel.TextColor3 = Value
					TextProperties.Color = Value
					Stroke.Color = Value
				end
				if (Property == 'Transparency') then
					TextLabel.TextTransparency = 1 - Value
					TextProperties.Transparency = Value
				end
				if (Property == 'Visible') then
					TextLabel.Visible = Value
					TextProperties.Visible = Value
				end
				if (Property == 'Outline') then
					Stroke.Enabled = Value
				end
				if (Property == 'Center') then
					if Value then
				        TextLabel.Position = UDim2.new(0, Camera.ViewportSize.X / 2, Camera.ViewportSize.Y / 2, 0)
				    end
			        
					TextProperties.Center = Value
				end
				if (Property == 'ZIndex') then
					TextLabel.ZIndex = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						TextLabel:Destroy();
					end)
				end
                 if Property == 'Destroy' then
                return (function()
						TextLabel:Destroy();
					end)
                end
				if Property == 'TextBounds' then
					return game:GetService('TextService'):GetTextSize(TextLabel.Text, TextLabel.TextSize, TextLabel.Font, TextLabel.AbsoluteSize)
				end
				return TextProperties[Property]
			end)
		})
	end

	if (Type == 'Square') then
		local SquareProperties = ({
			Thickness = 1,
			Size = Vector2.new(),
			Position = Vector2.new(),
			Filled = false,
		} + BaseDrawingProperties);
		local SquareFrame = Instance.new('Frame');

		--SquareFrame.AnchorPoint = Vector2.new(0.5, 0.5);
		SquareFrame.BorderSizePixel = 0

		SquareFrame.Visible = false
		SquareFrame.Parent = DrawingUI

		local Stroke = Instance.new('UIStroke', SquareFrame)
		Stroke.Thickness = 2
		Stroke.Enabled = false
		Stroke.LineJoinMode = Enum.LineJoinMode.Miter

		return setmetatable({}, {
			__newindex = (function(self, Property, Value)
				if (Property == 'Position') then
					SquareFrame.Position = UDim2.new(0, Value.X, 0, Value.Y)
					SquareProperties.Position = Value
				end
				if (Property == 'Size') then
					SquareFrame.Size = UDim2.new(0, Value.X, 0, Value.Y);
					SquareProperties.Size = Value
				end
				if (Property == 'Color') then
					SquareFrame.BackgroundColor3 = Value
					Stroke.Color = Value
					SquareProperties.Color = Value
				end
				if (Property == 'Transparency') then
					SquareFrame.BackgroundTransparency = Value
					SquareProperties.Transparency = Value
				end
				if (Property == 'Visible') then
					SquareFrame.Visible = Value
					SquareProperties.Visible = Value
				end
				if (Property == 'Filled') then -- requires beta
					SquareFrame.BackgroundTransparency = (Value == true and 0 or 1)
					Stroke.Enabled = not Value
					SquareProperties.Filled = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						SquareFrame:Destroy();
					end)
				end
               if Property == 'Destroy' then				
				return (function()
						SquareFrame:Destroy();
					end)
				end
				return SquareProperties[Property]
			end)
		})
	end


	if (Type == 'Quad') then -- idk if this will work lmao
		local QuadProperties = ({
			Thickness = 1,
			PointA = Vector2.new();
			PointB = Vector2.new();
			PointC = Vector2.new();
			PointD = Vector2.new();
			Filled = false;
		}  + BaseDrawingProperties);

		local PointA = DrawingLib.new('Line')
		local PointB = DrawingLib.new('Line')
		local PointC = DrawingLib.new('Line')
		local PointD = DrawingLib.new('Line')

		return setmetatable({}, {
			__newindex = (function(self, Property, Value)
				if Property == 'Thickness' then
					PointA.Thickness = Value
					PointB.Thickness = Value
					PointC.Thickness = Value
					PointD.Thickness = Value
				end
				if Property == 'PointA' then
					PointA.From = Value
					PointB.To = Value
				end
				if Property == 'PointB' then
					PointB.From = Value
					PointC.To = Value
				end
				if Property == 'PointC' then
					PointC.From = Value
					PointD.To = Value
				end
				if Property == 'PointD' then
					PointD.From = Value
					PointA.To = Value
				end
				if Property == 'Filled' then
					-- i'll do this later
				end
				if (Property == 'ZIndex') then
					PointA.ZIndex = Value
					PointB.ZIndex = Value
					PointC.ZIndex = Value
					PointD.ZIndex = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						PointA:Remove();
						PointB:Remove();
						PointC:Remove();
						PointD:Remove();
					end)
				end
                if Property ==  'Destroy' then
                       return (function()
						PointA:Remove();
						PointB:Remove();
						PointC:Remove();
						PointD:Remove();
					end)
                end
				return QuadProperties[Property]
			end)
		});
	end

if (Type == 'Image') then
		local ImageProperties = ({
			Data = 'rbxassetid://848623155', -- roblox assets only rn
			Size = Vector2.new(),
			Position = Vector2.new(),
			Rounding = 0,
		});

		local ImageLabel = Instance.new('ImageLabel');

		ImageLabel.BorderSizePixel = 0
		ImageLabel.ScaleType = Enum.ScaleType.Stretch
		ImageLabel.Transparency = 1

		ImageLabel.Visible = false
		ImageLabel.Parent = DrawingUI

		return setmetatable({}, {
			__newindex = (function(self, Property, Value)
				if (Property == 'Size') then
					ImageLabel.Size = UDim2.new(0, Value.X, 0, Value.Y);
					ImageProperties.Text = Value
				end
				if (Property == 'Position') then
					ImageLabel.Position = UDim2.new(0, Value.X, 0, Value.Y);
					ImageProperties.Position = Value
				end
				if (Property == 'Size') then
					ImageLabel.Size = UDim2.new(0, Value.X, 0, Value.Y);
					ImageProperties.Size = Value
				end
				if (Property == 'Transparency') then
					ImageLabel.ImageTransparency = 1 - Value
					ImageProperties.Transparency = Value
				end
				if (Property == 'Visible') then
					ImageLabel.Visible = Value
					ImageProperties.Visible = Value
				end
				if (Property == 'Color') then
					ImageLabel.ImageColor3 = Value
					ImageProperties.Color = Value
				end
				if (Property == 'Data') then
					ImageLabel.Image = Value
					ImageProperties.Data = Value
				end
				if (Property == 'ZIndex') then
					ImageLabel.ZIndex = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						ImageLabel:Destroy();
					end)
				end
                if Property ==  'Destroy' then
                return (function()
						ImageLabel:Destroy();
					end)
                end
				return ImageProperties[Property]
			end)
		})
	end

	if (Type == 'Triangle') then  -- idk if this will work lmao
		local TriangleProperties = ({
			Thickness = 1,
			PointA = Vector2.new();
			PointB = Vector2.new();
			PointC = Vector2.new();
			Filled = false;
		}  + BaseDrawingProperties);

		local PointA = DrawingLib.new('Line')
		local PointB = DrawingLib.new('Line')
		local PointC = DrawingLib.new('Line')

		return setmetatable({}, {
			__newindex = (function(self, Property, Value)
				if Property == 'Thickness' then
					PointA.Thickness = Value
					PointB.Thickness = Value
					PointC.Thickness = Value
				end
				if Property == 'PointA' then
					PointA.From = Value
					PointB.To = Value
					TriangleProperties.PointA = Value
				end
				if Property == 'PointB' then
					PointB.From = Value
					PointC.To = Value
					TriangleProperties.PointB = Value
				end
				if Property == 'PointC' then
					PointC.From = Value
					PointA.To = Value
					TriangleProperties.PointC = Value
				end
				if Property == 'Filled' then
					-- i'll do this later
				end
				if (Property == 'ZIndex') then
					PointA.ZIndex = Value
					PointB.ZIndex = Value
					PointC.ZIndex = Value
				end
			end),
			__index = (function(self, Property)
				if (string.lower(tostring(Property)) == 'remove') then
					return (function()
						PointA:Remove();
						PointB:Remove();
						PointC:Remove();
					end)
				end
                if Property ==  'Destroy' then
                return (function()
						PointA:Remove();
						PointB:Remove();
						PointC:Remove();
					end)
                end
				return TriangleProperties[Property]
			end)
		});
	end
end

local rendered = false
getgenv().isrenderobj = function(a, drawing2)
	if rendered == true then
	return false
	end

	rendered = true
	return true
end

getgenv().setrenderproperty = function(drawing, object, value)
	drawing[object] = value
	return object
end

getgenv().boolean31 = newcclosure(function()
	return true
end)

getgenv().getrenderproperty = function(drawing, object)
	if object == "Visible" then
		return boolean31()
	end

	return true
end

DrawingLib.clear = function() 
	DrawingUI:ClearAllChildren();
end

getgenv().Drawing = DrawingLib
getgenv().cleardrawcache = DrawingLib.clear
Drawing.cleardrawcache = DrawingLib.clear
Drawing.Fonts = {
    UI = 0,
    System = 1,
    Plex = 2,
    Monospace = 3
}
)";

#endif