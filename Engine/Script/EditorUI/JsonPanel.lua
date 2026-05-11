local gui = GUI

-- 数据
local state = {
    player_name = "Hero",
    player_level = 42,
    player_hp = 100.0,
    enable_effect = true,
    selected_item = 1,
    color = {1.0, 0.5, 0.2, 1.0}
}

local items = {"Sword", "Shield", "Potion", "Scroll"}

function Draw() -- 绘制函数
	-- 标题
    gui:Label("=== Player Settings ===")
    gui:Spacing()

    -- 文本输入
    gui:TextInput("Name", state, "player_name", 256, 0)
    
    -- 拖拽数字
    gui:DragInt("Level", state, "player_level", 1, 1, 99, "%d")
    gui:DragFloat("HP", state, "player_hp", 0.5, 0, 999, "%.1f", 1.0)
    
    gui:Separator()

    -- 复选框
    gui:Checkbox("Enable Effect", state, "enable_effect")

    -- 下拉框
    gui:Combo("Item", state, "selected_item", items, 5)

    -- 颜色选择器
    gui:ColorEdit("Tint", state["color"])

    gui:Separator()

    -- 按钮
    gui:Button("Apply", function()
        print("Settings applied!")
    end, 120, 0)
end
