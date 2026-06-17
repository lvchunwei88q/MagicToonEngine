#pragma once
#include <vector>
#include <cstring>
#include <cstdint>

class BinaryBuffer {
private:
    std::vector<uint8_t> m_Data;
    size_t m_WriteOffset;      // 当前写入位置
    size_t m_CurrentSize;      // 当前分配的大小

public:
    explicit BinaryBuffer(size_t initialSize = 0)
        : m_WriteOffset(0)
        , m_CurrentSize(initialSize) {
        if (initialSize > 0) {
            m_Data.resize(initialSize);
        }
    }

    BinaryBuffer(const BinaryBuffer& other)
        : m_Data(other.m_Data)      
        , m_WriteOffset(other.m_WriteOffset)
        , m_CurrentSize(other.m_CurrentSize) {
    }

    BinaryBuffer& operator=(const BinaryBuffer& other) {
        if (this != &other) {  // 防止自赋值
            m_Data = other.m_Data;
            m_WriteOffset = other.m_WriteOffset;
            m_CurrentSize = other.m_CurrentSize;
        }
        return *this;
    }

    // 分配空间并返回写入指针
    uint8_t* Allocate(size_t size) {
        // 检查是否需要扩展
        size_t newSize = m_WriteOffset + size;
        if (newSize > m_CurrentSize) {
            // 扩展空间
            size_t reserved = static_cast<size_t>(newSize * 1.5 + 1024);
            m_Data.resize(reserved);
            m_CurrentSize = reserved;
        }

        // 获取当前写入位置的指针
        uint8_t* ptr = m_Data.data() + m_WriteOffset;
        m_WriteOffset += size;

        return ptr;
    }

    // 写入数据
    template<typename T>
    void Write(const T& value) {
        uint8_t* ptr = Allocate(sizeof(T));
        memcpy(ptr, &value, sizeof(T));
    }

    void Write(const void* data, size_t size) {
        uint8_t* ptr = Allocate(size);
        memcpy(ptr, data, size);
    }

    void Write(const std::vector<uint8_t>& data) {
        Write(data.data(), data.size());
    }

    // 获取当前写入指针
    const uint8_t* GetCurrentPtr() const {
        return m_Data.data() + m_WriteOffset;
    }

    // 获取数据指针
    uint8_t* Data() {
        return m_Data.data();
    }

    const char* Data() const {
        return reinterpret_cast<const char*>(m_Data.data());
    }

    // 获取当前已写入的大小
    size_t GetWrittenSize() const {
        return m_WriteOffset;
    }

    // 获取当前分配的总大小
    size_t GetTotalSize() const {
        return m_CurrentSize;
    }

    // 获取实际数据大小
    size_t GetUsedSize() const {
        return m_WriteOffset;
    }

    // 收缩到实际使用大小
    void ShrinkToFit() {
        if (m_Data.size() > m_WriteOffset) {
            m_Data.resize(m_WriteOffset);
            m_CurrentSize = m_WriteOffset;
        }
    }

    // 获取底层 vector（移动语义）
    std::vector<uint8_t> MoveData() {
        ShrinkToFit();
        return std::move(m_Data);
    }

    // 重置（清空数据）
    void Reset() {
        m_Data.clear();
        m_WriteOffset = 0;
        m_CurrentSize = 0;
    }

    // 重置并重新分配
    void Reset(size_t newSize) {
        m_Data.clear();
        m_Data.resize(newSize);
        m_WriteOffset = 0;
        m_CurrentSize = newSize;
    }

    // 手动设置写入位置
    void Seek(size_t position) {
        if (position <= m_CurrentSize) {
            m_WriteOffset = position;
        }
    }

    // 获取当前偏移
    size_t Tell() const {
        return m_WriteOffset;
    }
};