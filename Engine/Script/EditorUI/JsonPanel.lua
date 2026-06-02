-- 测试 JSON 数据（用 Lua 表模拟）
local json_data = {
    name = "GameActor",
    visible = true,
    position = {1.0, 2.0, 3.0},
    rotation = {0.0, 45.0, 0.0},
    scale = {1.0, 1.0, 1.0},
    color = {1.0, 0.5, 0.0, 1.0},
    speed = 10.0,
    type = 2,                     -- 0: Static, 1: Dynamic, 2: Player
    mesh = "default.mesh",
    tags = "player,hero,important!",  -- 字符串模拟数组
    description = "This is a sample actor."
}
-- 全局变量
index = 0

-- ----------------------------------------------------
-- 绘制 Transform 折叠块
-- ----------------------------------------------------
local function DrawTransform(data)
    if GUI:TreeNode("Transform") then
        -- 向量控件直接操作以数字索引的表 {1=x, 2=y, 3=z}
        GUI:Vector3Control("Position", data.position, 1.0, 80.0)
        GUI:Vector3Control("Rotation", data.rotation, 1.0, 80.0)
        GUI:Vector3Control("Scale",    data.scale,    1.0, 80.0)
        GUI:TreePop()
    end
end

-- ----------------------------------------------------
-- 绘制 Rendering 折叠块
-- ----------------------------------------------------
local function DrawRendering(data)
    if GUI:TreeNode("Rendering") then
        GUI:Checkbox("Visible", data, "visible")
        GUI:ColorEdit("Color", data.color)
        GUI:TextInput("Mesh", data, "mesh", 256)
        GUI:TreePop()
    end
end

-- ----------------------------------------------------
-- 绘制 Configuration 折叠块
-- ----------------------------------------------------
local function DrawConfiguration(data)
    if GUI:TreeNode("Configuration") then
        GUI:DragFloat("Speed", data, "speed", 0.1, 0.0, 100.0, "%.2f", 1)

        -- 下拉框选择类型
        local type_items = { "Static", "Dynamic", "Player" }
        GUI:Combo("Type", data, "type", type_items, 3)

        -- 标签字符串
        GUI:TextInput("Tags", data, "tags", 128)
        GUI:TextArea("Description", data, "description", 1024)
        GUI:TreePop()
    end
end

-- ----------------------------------------------------
-- 绘制输入区域
-- ----------------------------------------------------

local function DrawInputArea()
    if GUI:TreeNode("InputArea") then
    
        GUI:Label("Create New Lua Member")
        GUI:Separator()
        GUI:Spacing()
    
        -- 输入 member_name
        GUI:Label("Member Name:")
        GUI:SameLine(0.0, -1.0)

        local input_name = "LUATEST"
    
        GUI:Spacing()
    
        -- 输入 lua_type（下拉框）
        GUI:Label("Lua Type:")
        GUI:SameLine(0.0, -1.0)
        input_type = "number"
    
        GUI:Spacing()
        GUI:Separator()
    
        -- 创建按钮
        GUI:Button("Update", function()
            if input_name ~= "" then
                -- 创建新的 LuaMemberClass 对象
                local new_member = CreateNewMember(input_name, input_type)
            
                -- 调用 Print 函数
                new_member:Print(index)
                index = index + 1
            
                -- 可选：清空输入框
                input_name = ""
                input_type = "string"
            end
        end, 100, 30)

        GUI:SameLine(0.0, -1.0)

        GUI:Button("UpdateInfo", function()
            -- 调用 ShowToast 函数
            ShowToast(1)
        end, 100, 30)

        GUI:SameLine(0.0, -1.0)

        GUI:Button("UpdateWarning", function()
            -- 调用 ShowToast 函数
            ShowToast(2)
        end, 100, 30)

        GUI:Button("UpdateError", function()
            -- 调用 ShowToast 函数
            ShowToast(3)
        end, 100, 30)

        GUI:SameLine(0.0, -1.0)

        GUI:Button("UpdateNone", function()
            -- 调用 ShowToast 函数
            ShowToast(4)
        end, 100, 30)

        GUI:TreePop()
    end
end


-- ----------------------------------------------------
-- 主绘制函数
-- ----------------------------------------------------
local function DrawPropertyPanel(data)

    if GUI:BeginChild("Select Control Target Windows")then
        -- 基础文本
        GUI:Label("JSON Property Editor")
        GUI:Separator()

        -- 名称
        GUI:TextInput("Name", data, "name", 128)
        GUI:Separator()

        -- 各分类折叠块
        DrawTransform(data)
        GUI:Spacing()
        DrawRendering(data)
        GUI:Spacing()
        DrawConfiguration(data)
        GUI:Spacing()
        DrawInputArea()
    end
    GUI:EndChild()
end

function Draw() -- 绘制函数
	DrawPropertyPanel(json_data)
end
