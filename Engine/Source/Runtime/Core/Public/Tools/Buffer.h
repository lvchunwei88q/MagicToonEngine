#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <stdexcept>

/*
* In future development, we need to handle a lot of byte data (which are often the smallest units of data),
* so to make it easier and clearer to work with this data, we use these classes to manage it efficiently.
*/

class BinaryWrite {
private:
    std::vector<uint8_t> m_Data;
    size_t m_WriteOffset;      // Current Write Postion
    size_t m_CurrentSize;      // Currently allocated size

public:
    explicit BinaryWrite(size_t initialSize = 0)
        : m_WriteOffset(0)
        , m_CurrentSize(initialSize) {
        if (initialSize > 0) {
            m_Data.resize(initialSize);
        }
    }

    explicit BinaryWrite(const std::vector<uint8_t>& data)
        : m_WriteOffset(0)
        , m_CurrentSize(data.size()) {
        if (data.size() > 0) {
            m_Data = data;
        }
    }

    // copy construction
    BinaryWrite(const BinaryWrite& other)
        : m_Data(other.m_Data)      
        , m_WriteOffset(other.m_WriteOffset)
        , m_CurrentSize(other.m_CurrentSize) {
    }

    // Note that 'other' can no longer be used afterwards
    BinaryWrite(BinaryWrite&& other) noexcept
        : m_Data(std::move(other.m_Data))
        , m_WriteOffset(other.m_WriteOffset)
        , m_CurrentSize(other.m_CurrentSize) {}

    BinaryWrite& operator=(const BinaryWrite& other) {
        if (this != &other) {  // Prevent self-assignment
            m_Data = other.m_Data;
            m_WriteOffset = other.m_WriteOffset;
            m_CurrentSize = other.m_CurrentSize;
        }
        return *this;
    }

    // Allocate space and return the write pointer
    uint8_t* Allocate(size_t size) {
        // Check if an extension is needed
        size_t newSize = m_WriteOffset + size;
        if (newSize > m_CurrentSize) {
            // Expandable space
            size_t reserved = static_cast<size_t>(newSize * 1.5 + 1024);
            m_Data.resize(reserved);
            m_CurrentSize = m_Data.size();
        }

        // Get the pointer to the current write position
        uint8_t* ptr = m_Data.data() + m_WriteOffset;
        m_WriteOffset += size;

        return ptr;
    }

    // Write data
    template<typename T>
    void Write(const T& value) {
        uint8_t* ptr = Allocate(sizeof(T));
        memcpy(ptr, &value, sizeof(T));
    }

    void Write(const void* data, size_t size) {
        uint8_t* ptr = Allocate(size);
        memcpy(ptr, data, size);
    }

    void Write(const std::vector<uint8_t>& data) {Write(data.data(), data.size());}
    // Get current write pointer
    const uint8_t* GetCurrentPtr()  const {return m_Data.data() + m_WriteOffset;}
    // Get data pointer
    const uint8_t* DataU8()         const {return m_Data.data();}
    const char* DataChar()          const {return reinterpret_cast<const char*>(m_Data.data());}
    // Get the size written so far
    size_t GetWrittenSize()         const {return m_WriteOffset;}
    // Get the total size currently allocated
    size_t GetTotalSize()           const {return m_CurrentSize;}
    // Get the actual data size
    size_t GetUsedSize()            const {return m_WriteOffset;}

    // Manually set write position
    void Seek(size_t position)      { if (position <= m_CurrentSize)m_WriteOffset = position; }
    // Get Current Offset
    size_t Tell()                   const { return m_WriteOffset; }

    // Shrink to actual size
    void ShrinkToFit() {
        if (m_Data.size() > m_WriteOffset) {
            m_Data.resize(m_WriteOffset);
            m_CurrentSize = m_WriteOffset;
        }
    }

    // Get vector
    std::vector<uint8_t> MoveData() {
        ShrinkToFit();
        return std::move(m_Data);
    }

    // Reset
    void Reset() {
        m_Data.clear();
        m_WriteOffset = 0;
        m_CurrentSize = 0;
    }

    // Reset and reassign
    void Reset(size_t newSize) {
        m_Data.clear();
        m_Data.resize(newSize);
        m_WriteOffset = 0;
        m_CurrentSize = newSize;
    }
};

class BinaryReader {
private:
    std::vector<uint8_t> m_Data;                // Point to external data
    size_t m_ReadOffset;                        // Current read postion
    size_t m_DataSize;                          // data size

public:
    // in vector construction
    explicit BinaryReader(const std::vector<uint8_t>& data)
        : m_Data(data)
        , m_ReadOffset(0)
        , m_DataSize(data.size()) {}

    // in BinaryBuffer read
    explicit BinaryReader(const BinaryWrite& buffer)
        : m_ReadOffset(0)
        , m_DataSize(buffer.GetUsedSize()) {
        // copy data to this
        const uint8_t* src = buffer.DataU8();
        m_Data = std::vector<uint8_t>(src, src + m_DataSize);
    }

    // copy construction
    BinaryReader(const BinaryReader& other)
        : m_Data(other.m_Data)  // Shared data
        , m_ReadOffset(other.m_ReadOffset)
        , m_DataSize(other.m_DataSize) {}

    // Note that 'other' can no longer be used afterwards
    BinaryReader(BinaryReader&& other) noexcept
        : m_Data(std::move(other.m_Data))
        , m_ReadOffset(other.m_ReadOffset)
        , m_DataSize(other.m_DataSize){}

    ~BinaryReader() {}

    // read base type
    template<typename T>
    T Read() {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");

        if (m_ReadOffset + sizeof(T) > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        T value;
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        memcpy(&value, src, sizeof(T));
        m_ReadOffset += sizeof(T);
        return value;
    }

    // read to vector input vector count
    std::vector<uint8_t> ReadToVector(size_t count) {
        if (m_ReadOffset + count > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        const uint8_t* src = m_Data.data() + m_ReadOffset;
        std::vector<uint8_t> result(src, src + count);
        m_ReadOffset += count;
        return result;
    }

    // read to vector add
    void ReadToVector(std::vector<uint8_t>& out, size_t count) {
        if (m_ReadOffset + count > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        const uint8_t* src = m_Data.data() + m_ReadOffset;
        out.insert(out.end(), src, src + count);
        m_ReadOffset += count;
    }

    // Read a specified number of bytes into the buffer
    void ReadBytes(void* buffer, size_t count) {
        if (m_ReadOffset + count > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        const uint8_t* src = m_Data.data() + m_ReadOffset;
        memcpy(buffer, src, count);
        m_ReadOffset += count;
    }

    // Read string
    std::string ReadString(size_t length) {
        if (m_ReadOffset + length > m_DataSize) {
            throw std::out_of_range("String data truncated");
        }

        const char* src = reinterpret_cast<const char*>(m_Data.data() + m_ReadOffset);
        std::string result(src, length);
        m_ReadOffset += length;
        return result;
    }

    // Peek data
    template<typename T>
    T Peek() const {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");

        if (m_ReadOffset + sizeof(T) > m_DataSize)
            throw std::out_of_range("Not enough data to peek");

        T value;
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        memcpy(&value, src, sizeof(T));
        return value;
    }

    // Skip to specified byte
    void Skip(size_t count) {
        if (m_ReadOffset + count > m_DataSize)
            throw std::out_of_range("Skip beyond data end");
        m_ReadOffset += count;
    }

    // get current read postion
    size_t Tell() const {return m_ReadOffset;}

    // set read postion
    void Seek(size_t position) {
        if (position > m_DataSize)
            throw std::out_of_range("Seek position out of range");
        m_ReadOffset = position;
    }

    // Get the remaining readable bytes
    size_t Remaining()          const {return m_DataSize - m_ReadOffset;}
    // data size
    size_t Size()               const {return m_DataSize;}
    // Check if it's finished reading
    bool IsEOF()                const {return m_ReadOffset >= m_DataSize;}
    // Get data pointer
    const uint8_t* Data()       const {return m_Data.data();}
    // Get the current pointer position
    const uint8_t* CurrentPtr() const {return m_Data.data() + m_ReadOffset;}
    // Reset read position
    void Reset()                {m_ReadOffset = 0;}

    // Reset and set new data
    void Reset(const std::vector<uint8_t>& data) {
        m_Data = data;
        m_ReadOffset = 0;
        m_DataSize = data.size();
    }
};