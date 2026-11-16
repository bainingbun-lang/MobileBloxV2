package com.mobileblox.v2.luau

/**
 * Luau prelude that provides a small, sandboxed simulation of the Roblox API.
 *
 * It runs inside the app's own Luau VM. It never attaches to Roblox and does not
 * touch another process: it only gives scripts a familiar API to experiment with.
 */
object RobloxApiPrelude {

    val SOURCE: String = """
        -- MobileBlox simulated Roblox API (sandbox, in-app only)
        function warn(...)
            local count = select("#", ...)
            local parts = {}
            for i = 1, count do
                parts[i] = tostring(select(i, ...))
            end
            print("[WARN] " .. table.concat(parts, " "))
        end

        local function newSignal()
            local signal = { _handlers = {} }
            function signal:Connect(fn)
                table.insert(self._handlers, fn)
                return { Disconnect = function() end }
            end
            function signal:Wait()
                return nil
            end
            function signal:Fire(...)
                for _, fn in ipairs(self._handlers) do
                    local ok, err = pcall(fn, ...)
                    if not ok then warn(err) end
                end
            end
            return signal
        end

        local Vector3Methods = {}
        local Vector3 = {}
        Vector3.__index = function(self, key)
            if key == "Magnitude" then
                return math.sqrt(self.X * self.X + self.Y * self.Y + self.Z * self.Z)
            end
            if key == "Unit" then
                local magnitude = self.Magnitude
                if magnitude == 0 then return Vector3.new() end
                return Vector3.new(self.X / magnitude, self.Y / magnitude, self.Z / magnitude)
            end
            local method = Vector3Methods[key]
            if method ~= nil then return method end
            return rawget(self, key)
        end
        function Vector3.new(x, y, z)
            return setmetatable({ X = x or 0, Y = y or 0, Z = z or 0, __type = "Vector3" }, Vector3)
        end
        function Vector3Methods:Dot(other)
            return self.X * other.X + self.Y * other.Y + self.Z * other.Z
        end
        function Vector3Methods:Cross(other)
            return Vector3.new(
                self.Y * other.Z - self.Z * other.Y,
                self.Z * other.X - self.X * other.Z,
                self.X * other.Y - self.Y * other.X
            )
        end
        function Vector3Methods:Lerp(other, alpha)
            return Vector3.new(
                self.X + (other.X - self.X) * alpha,
                self.Y + (other.Y - self.Y) * alpha,
                self.Z + (other.Z - self.Z) * alpha
            )
        end
        Vector3.__add = function(a, b) return Vector3.new(a.X + b.X, a.Y + b.Y, a.Z + b.Z) end
        Vector3.__sub = function(a, b) return Vector3.new(a.X - b.X, a.Y - b.Y, a.Z - b.Z) end
        Vector3.__mul = function(a, b)
            if type(a) == "number" then return Vector3.new(a * b.X, a * b.Y, a * b.Z) end
            if type(b) == "number" then return Vector3.new(a.X * b, a.Y * b, a.Z * b) end
            return Vector3.new(a.X * b.X, a.Y * b.Y, a.Z * b.Z)
        end
        Vector3.__div = function(a, b)
            if type(b) == "number" then return Vector3.new(a.X / b, a.Y / b, a.Z / b) end
            return Vector3.new(a.X / b.X, a.Y / b.Y, a.Z / b.Z)
        end
        Vector3.__unm = function(a) return Vector3.new(-a.X, -a.Y, -a.Z) end
        Vector3.__eq = function(a, b) return a.X == b.X and a.Y == b.Y and a.Z == b.Z end
        Vector3.__tostring = function(a)
            return string.format("Vector3(%g, %g, %g)", a.X, a.Y, a.Z)
        end

        local CFrameMethods = {}
        local CFrame = {}
        CFrame.__index = function(self, key)
            if key == "Position" then return Vector3.new(self.X, self.Y, self.Z) end
            if key == "LookVector" then return Vector3.new(0, 0, -1) end
            if key == "RightVector" then return Vector3.new(1, 0, 0) end
            if key == "UpVector" then return Vector3.new(0, 1, 0) end
            local method = CFrameMethods[key]
            if method ~= nil then return method end
            return rawget(self, key)
        end
        function CFrame.new(a, b, c)
            if type(a) == "table" and a.X ~= nil then
                return setmetatable({ X = a.X, Y = a.Y, Z = a.Z, __type = "CFrame" }, CFrame)
            end
            return setmetatable({ X = a or 0, Y = b or 0, Z = c or 0, __type = "CFrame" }, CFrame)
        end
        function CFrame.Angles()
            return CFrame.new()
        end
        function CFrame.fromEulerAnglesXYZ()
            return CFrame.new()
        end
        function CFrame.lookAt(position)
            return CFrame.new(position)
        end
        function CFrameMethods:Lerp(other, alpha)
            return CFrame.new(
                self.X + (other.X - self.X) * alpha,
                self.Y + (other.Y - self.Y) * alpha,
                self.Z + (other.Z - self.Z) * alpha
            )
        end
        function CFrameMethods:Inverse()
            return CFrame.new(-self.X, -self.Y, -self.Z)
        end
        function CFrameMethods:ToWorldSpace(other)
            if getmetatable(other) == Vector3 then
                return Vector3.new(self.X + other.X, self.Y + other.Y, self.Z + other.Z)
            end
            return CFrame.new(self.X + other.X, self.Y + other.Y, self.Z + other.Z)
        end
        CFrame.__mul = function(a, b)
            if type(b) == "number" then return a end
            if getmetatable(b) == Vector3 then
                return Vector3.new(a.X + b.X, a.Y + b.Y, a.Z + b.Z)
            end
            if getmetatable(b) == CFrame then
                return CFrame.new(a.X + b.X, a.Y + b.Y, a.Z + b.Z)
            end
            return a
        end
        CFrame.__tostring = function(a)
            return string.format("CFrame(%g, %g, %g)", a.X, a.Y, a.Z)
        end

        local Color3 = {}
        Color3.__index = Color3
        function Color3.new(r, g, b)
            return setmetatable({ R = r or 0, G = g or 0, B = b or 0, __type = "Color3" }, Color3)
        end
        function Color3.fromRGB(r, g, b)
            return Color3.new((r or 0) / 255, (g or 0) / 255, (b or 0) / 255)
        end
        Color3.__tostring = function(color)
            return string.format("Color3(%g, %g, %g)", color.R, color.G, color.B)
        end

        local UDim = {}
        function UDim.new(scale, offset)
            return { Scale = scale or 0, Offset = offset or 0 }
        end
        local UDim2 = {}
        function UDim2.new(xScale, xOffset, yScale, yOffset)
            return {
                X = UDim.new(xScale, xOffset),
                Y = UDim.new(yScale, yOffset),
                __type = "UDim2",
            }
        end
        function UDim2.fromScale(x, y)
            return UDim2.new(x, 0, y, 0)
        end
        function UDim2.fromOffset(x, y)
            return UDim2.new(0, x, 0, y)
        end

        local function enumItem(name, value)
            return { Name = name, Value = value, EnumType = name }
        end
        local Enum = {
            PartType = {
                Ball = enumItem("Ball", 0),
                Block = enumItem("Block", 1),
                Cylinder = enumItem("Cylinder", 2),
            },
            Material = {
                Plastic = enumItem("Plastic", 256),
                SmoothPlastic = enumItem("SmoothPlastic", 272),
                Neon = enumItem("Neon", 288),
                Metal = enumItem("Metal", 1088),
            },
            KeyCode = {
                E = enumItem("E", 101),
                F = enumItem("F", 102),
            },
            HumanoidStateType = {
                Running = enumItem("Running", 0),
                Jumping = enumItem("Jumping", 1),
                Swimming = enumItem("Swimming", 2),
            },
        }

        local newInstance
        local InstanceMethods = {}
        local Instance = {}
        Instance.__index = function(self, key)
            if key == "Parent" then return rawget(self, "_parent") end
            local method = InstanceMethods[key]
            if method ~= nil then return method end
            return rawget(self, key)
        end
        Instance.__newindex = function(self, key, value)
            if key == "Parent" then
                local oldParent = rawget(self, "_parent")
                if oldParent ~= nil then
                    local siblings = rawget(oldParent, "_children")
                    if siblings ~= nil then
                        for index, child in ipairs(siblings) do
                            if child == self then
                                table.remove(siblings, index)
                                break
                            end
                        end
                    end
                end
                rawset(self, "_parent", value)
                if value ~= nil then
                    local children = rawget(value, "_children")
                    if children ~= nil then
                        table.insert(children, self)
                    end
                end
            else
                rawset(self, key, value)
            end
        end
        function InstanceMethods:GetChildren()
            local result = {}
            for index, child in ipairs(rawget(self, "_children") or {}) do
                result[index] = child
            end
            return result
        end
        function InstanceMethods:GetDescendants()
            local result = {}
            local function walk(node)
                for _, child in ipairs(rawget(node, "_children") or {}) do
                    table.insert(result, child)
                    walk(child)
                end
            end
            walk(self)
            return result
        end
        function InstanceMethods:FindFirstChild(name, recursive)
            for _, child in ipairs(rawget(self, "_children") or {}) do
                if child.Name == name then return child end
            end
            if recursive then
                for _, child in ipairs(rawget(self, "_children") or {}) do
                    local found = child:FindFirstChild(name, true)
                    if found ~= nil then return found end
                end
            end
            return nil
        end
        function InstanceMethods:FindFirstChildOfClass(className)
            for _, child in ipairs(rawget(self, "_children") or {}) do
                if child.ClassName == className then return child end
            end
            return nil
        end
        function InstanceMethods:WaitForChild(name)
            return self:FindFirstChild(name)
        end
        function InstanceMethods:IsA(className)
            local current = rawget(self, "ClassName")
            if current == className then return true end
            local hierarchy = {
                Part = { "BasePart", "PVInstance", "Instance" },
                MeshPart = { "BasePart", "PVInstance", "Instance" },
                WedgePart = { "BasePart", "PVInstance", "Instance" },
                SpawnLocation = { "BasePart", "PVInstance", "Instance" },
                Model = { "PVInstance", "Instance" },
                Workspace = { "Model", "PVInstance", "Instance" },
                Humanoid = { "Instance" },
                Player = { "Instance" },
                Folder = { "Instance" },
                Script = { "LuaSourceContainer", "Instance" },
                LocalScript = { "LuaSourceContainer", "Instance" },
                ModuleScript = { "LuaSourceContainer", "Instance" },
            }
            for _, parentClass in ipairs(hierarchy[current] or {}) do
                if parentClass == className then return true end
            end
            return className == "Instance"
        end
        function InstanceMethods:Destroy()
            self.Parent = nil
            rawset(self, "_children", {})
        end
        function InstanceMethods:Clone()
            local copy = newInstance(rawget(self, "ClassName"))
            for key, value in pairs(self) do
                if type(key) == "string" and key:sub(1, 1) ~= "_" then
                    rawset(copy, key, value)
                end
            end
            return copy
        end
        function InstanceMethods:SetAttribute(name, value)
            local attributes = rawget(self, "_attributes")
            if attributes == nil then
                attributes = {}
                rawset(self, "_attributes", attributes)
            end
            attributes[name] = value
        end
        function InstanceMethods:GetAttribute(name)
            local attributes = rawget(self, "_attributes")
            if attributes == nil then return nil end
            return attributes[name]
        end
        function InstanceMethods:GetPropertyChangedSignal()
            return newSignal()
        end
        function InstanceMethods:TakeDamage(amount)
            if self.Health ~= nil then
                self.Health = math.max(0, self.Health - (amount or 0))
            end
        end
        function InstanceMethods:MoveTo(position)
            local root = self:FindFirstChild("HumanoidRootPart")
            if root ~= nil then
                root.Position = position
            end
        end

        function newInstance(className, parent)
            local object = setmetatable({
                ClassName = className,
                Name = className,
                _children = {},
                _attributes = {},
                _parent = nil,
            }, Instance)
            if parent ~= nil then
                object.Parent = parent
            end
            return object
        end
        Instance.new = function(className)
            return newInstance(className or "Instance")
        end

        local game = newInstance("DataModel")
        game.Name = "Game"
        game.PlaceId = 0
        game.GameId = 0
        game.JobId = "mobileblox-sandbox"

        local workspace = newInstance("Workspace", game)
        workspace.Name = "Workspace"
        workspace.Gravity = 196.2

        local players = newInstance("Players", game)
        players.Name = "Players"
        players.PlayerAdded = newSignal()
        players.PlayerRemoving = newSignal()

        local camera = newInstance("Camera", workspace)
        camera.Name = "Camera"
        workspace.CurrentCamera = camera

        local baseplate = newInstance("Part", workspace)
        baseplate.Name = "Baseplate"
        baseplate.Anchored = true
        baseplate.Size = Vector3.new(512, 20, 512)
        baseplate.Position = Vector3.new(0, -10, 0)

        local spawnLocation = newInstance("SpawnLocation", workspace)
        spawnLocation.Name = "SpawnLocation"
        spawnLocation.Position = Vector3.new(0, 0.5, 0)

        local localPlayer = newInstance("Player", players)
        localPlayer.Name = "Player"
        localPlayer.DisplayName = "Player"
        localPlayer.UserId = 1

        local character = newInstance("Model", workspace)
        character.Name = "Player"
        local humanoid = newInstance("Humanoid", character)
        humanoid.Name = "Humanoid"
        humanoid.Health = 100
        humanoid.MaxHealth = 100
        humanoid.WalkSpeed = 16
        humanoid.JumpPower = 50
        humanoid.JumpHeight = 7.2
        humanoid.Died = newSignal()
        local rootPart = newInstance("Part", character)
        rootPart.Name = "HumanoidRootPart"
        rootPart.Size = Vector3.new(2, 2, 1)
        rootPart.Position = Vector3.new(0, 3, 0)
        localPlayer.Character = character
        localPlayer.Humanoid = humanoid

        players.LocalPlayer = localPlayer
        players.GetPlayers = function()
            return { localPlayer }
        end

        game.Workspace = workspace
        game.Players = players
        game.GetService = function(_, name)
            if name == "Workspace" then return workspace end
            if name == "Players" then return players end
            if name == "RunService" then
                return {
                    IsStudio = function() return false end,
                    Heartbeat = newSignal(),
                    RenderStepped = newSignal(),
                    Stepped = newSignal(),
                }
            end
            return newInstance(name, game)
        end

        local Random = {}
        Random.__index = Random
        function Random.new(seed)
            return setmetatable({ _seed = seed or os.time() }, Random)
        end
        function Random:NextNumber(min, max)
            min = min or 0
            max = max or 1
            return min + math.random() * (max - min)
        end
        function Random:NextInteger(min, max)
            return math.random(min, max)
        end
        function Random:NextUnitVector()
            return Vector3.new(
                math.random() * 2 - 1,
                math.random() * 2 - 1,
                math.random() * 2 - 1
            ).Unit
        end
        function Random:Shuffle(list)
            for index = #list, 2, -1 do
                local swap = math.random(index)
                list[index], list[swap] = list[swap], list[index]
            end
        end

        local task = {}
        function task.wait(seconds) return seconds or 0 end
        function task.spawn(fn, ...)
            local ok, err = pcall(fn, ...)
            if not ok then warn(err) end
        end
        function task.delay(_, fn, ...)
            local ok, err = pcall(fn, ...)
            if not ok then warn(err) end
        end
        function task.defer(fn, ...)
            local ok, err = pcall(fn, ...)
            if not ok then warn(err) end
        end
        function task.cancel() end

        wait = task.wait
        spawn = task.spawn
        delay = task.delay

        function getgenv() return _G end
        function getrenv() return _G end
        function getreg() return _G end

        local rawType = type
        function typeof(value)
            if rawType(value) == "table" then
                local tagged = rawget(value, "__type")
                if tagged ~= nil then return tagged end
                if getmetatable(value) == Instance then return "Instance" end
            end
            return rawType(value)
        end

        game = game
        workspace = workspace
        Workspace = workspace
        Players = players
        Game = game
        script = newInstance("Script", game)
        script.Name = "Script"
    """.trimIndent()
}
