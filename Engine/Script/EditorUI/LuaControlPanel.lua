require("Common/LuaUIEnum") 

-- 全局状态
PersistentData = PersistentData or {}
PersistentData.listboxState = PersistentData.listboxState or { selected = 0 }
PersistentData.selectedName = PersistentData.selectedName or "Please select a control target"
PersistentData.selectedType = PersistentData.selectedType or "..."

-- ----------------------------------------------------
-- 从 LuaMembers 构建选项列表
-- ----------------------------------------------------
local function BuildMemberItems()
    local items = {}
    if LuaMembers and LuaMembersCount then
        local count = LuaMembersCount
        local i = 0
        while i < count do
            local member = LuaMembers[i]
            local name = "???"
            local ltype = "???"
            
            if member then
                if member.member_name ~= nil then name = member.member_name end
                if member.lua_type ~= nil then ltype = member.lua_type end
            end
            
            items[i + 1] = name .. " [" .. ltype .. "]"
            i = i + 1
        end
    end
    return items
end
-- ----------------------------------------------------
-- 主绘制函数
-- ----------------------------------------------------
function Draw()
    local listboxState = PersistentData.listboxState -- 引用
    local selectedName = PersistentData.selectedName -- 引用
    local selectedType = PersistentData.selectedType -- 引用

    local items = BuildMemberItems()
    local count = #items

    if GUI:BeginChild("Select Control Target Windows")then

        GUI:Dummy(2.0, 0)
        GUI:SameLine(0.0, -1.0)

        GUI:Label("Select Control Target:")

        if GUI:TreeNode("Control List") then
            if count > 0 then
                -- 确保选中索引在有效范围
                if listboxState.selected < 0 or listboxState.selected >= count then
                    listboxState.selected = 0
                end

                -- 限制显示高度
                local height = count
                if height > 10 then height = 10 end

                local availWidth = GUI:GetContentRegionAvailWidth()
                GUI:SetNextItemWidth(availWidth)
                -- 列表选择控件
                GUI:ListBox("##control_list", listboxState, "selected", items, height)

                -- 根据选中的索引（0-based）读取对应的 LuaMember
                local idx = listboxState.selected
                local member = LuaMembers[idx]
                if member then
                    selectedName = member.member_name or "???"
                    selectedType = member.lua_type or "???"
                end
            else
                GUI:Dummy(2.0, 0)
                GUI:SameLine(0.0, -1.0)
                GUI:Label("No controls registered")
                selectedName = "Please select a control target"
                selectedType = "..."
            end
            GUI:TreePop()
        end
    end
    GUI:EndChild()

    GUI:PushStyleColor(ImGuiCol.ChildBg, 1.0, 0.0, 0.0, 1.0)
    if GUI:BeginChild("Select Control Target Windows")then
        GUI:Dummy(0.0, 0.0)

        GUI:Separator()

        GUI:Dummy(2.0, 0)
        GUI:SameLine(0.0, -1.0)
        -- 显示选中项信息
        GUI:Label("Selected Target:")
        GUI:Dummy(10.0, 0)
        GUI:SameLine(0.0, -1.0)
        GUI:Label("  Name: " .. selectedName)
        GUI:Dummy(10.0, 0)
        GUI:SameLine(0.0, -1.0)
        GUI:Label("  Type: " .. selectedType)

        GUI:Separator()

        GUI:Dummy(10.0, 0)
        GUI:SameLine(0.0, -1.0)
        -- 按钮操作
        GUI:Button("Update", function()
            if selectedName ~= "" then
                Update(selectedType,selectedName)
            end
        end, 100, 30)

        --GUI:SameLine(0.0, -1.0) 
    end
    GUI:EndChild()
    GUI:PopStyleColor()
    
end