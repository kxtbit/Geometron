// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppTooWideScopeInitStatement
// ReSharper disable CppParameterMayBeConst
#include "ResizingCircularBuffer.hpp"

#include <Geode/Geode.hpp>

#include "../engine/LuaEngine.hpp"
using namespace geode::prelude;

namespace _ResizingCircularBuffer_base {
    static void debugLogChars(char* array, size_t len, size_t start, size_t end) {
        std::stringstream out;
        out << "ResizingCircularBuffer array is now: [";
        for (size_t i = 0; i < len; i++) {
            if (i == end) out << ">";
            if (i != 0) out << ", ";
            if (i == start) out << "<";
            out << "'" << escapeString(std::string {array[i]}) << "'";
        }
        out << "]";
        log::info("{}", out.view());
        if (start < 0 || start >= len)
            log::info("ResizingCircularBuffer OUT OF BOUNDS start = {}", start);
        if (end < 0 || end > len)
            log::info("ResizingCircularBuffer OUT OF BOUNDS start = {}", end);
    }

    template<class T>
    ResizingCircularBuffer<T>::ResizingCircularBuffer() : ResizingCircularBuffer(2) {}

    template<class T>
    ResizingCircularBuffer<T>::ResizingCircularBuffer(size_t initialLen) {
        if (initialLen < 2) initialLen = 2;
        array = static_cast<T*>(std::malloc(sizeof(T) * initialLen));
        //debugging purposes
        //std::memset(array, 0, sizeof(T) * initialLen);

        length = initialLen;
        start = 0;
        end = 0;

        //debugLogChars(array, length, start, end);
    }

    template<class T>
    void ResizingCircularBuffer<T>::copyIn(size_t dst, const T* src, size_t count) {
        //log::info("ResizingCircularBuffer copying ptr -> {}:{} in array of {}", dst, dst + count - 1, length);
        std::memcpy(array + dst, src, count * sizeof(T));
        //if constexpr (std::is_same_v<T, char>) debugLogChars(array, length, start, end);
    }
    template<class T>
    void ResizingCircularBuffer<T>::copyOut(T* dst, size_t src, size_t count) {
        //log::info("ResizingCircularBuffer copying {}:{} -> ptr in array of {}", src, src + count - 1, length);
        std::memcpy(dst, array + src, count * sizeof(T));
        //if constexpr (std::is_same_v<T, char>) debugLogChars(array, length, start, end);
    }
    template<class T>
    void ResizingCircularBuffer<T>::move(size_t dst, size_t src, size_t count) {
        //log::info("ResizingCircularBuffer moving {}:{} -> {}:{} in array of {}", src, src + count - 1, dst, dst + count - 1, length);
        std::memmove(array + dst, array + src, count * sizeof(T));
    }

    template<class T>
    void ResizingCircularBuffer<T>::expand(size_t newLength) {
        size_t newStart = (start > end) ? (start - length + newLength) : (start);
        //log::info("ResizingCircularBuffer resizing {} -> {}", length, newLength);
        array = static_cast<T*>(std::realloc(array, newLength * sizeof(T)));
        //debugging purposes
        //std::memset(array + length, 0, sizeof(T) * (newLength - length));
        if (start > end) //data is split across the end and start of the array, so the part at the end must be moved
            move(newStart, start, length - start);
        length = newLength;
        end = newStart;
        //if constexpr (std::is_same_v<T, char>) debugLogChars(array, length, start, end);
    }
    template<class T>
    void ResizingCircularBuffer<T>::expandIfNeeded() {
        if (start != end) return;
        size_t newLength = std::bit_ceil(length + 1);
        expand(newLength);
    }

    template<class T>
    void ResizingCircularBuffer<T>::write(const T& value) {
        std::memcpy(array + (end++), &value, sizeof(T));
        if (end == length) end = 0;
        expandIfNeeded();
    }
    template<class T>
    bool ResizingCircularBuffer<T>::read(T& out) {
        if (start == end) return false;
        copyOut(&out, start++, 1);
        return true;
    }
    template<class T>
    std::optional<T> ResizingCircularBuffer<T>::read() {
        T out;
        bool success = read(out);
        return success ? out : std::optional<T>();
    }

    template<class T>
    void ResizingCircularBuffer<T>::peek(T& out, size_t pos) {
        size_t arrayPos = start + pos;
        if (arrayPos >= length) arrayPos -= length;
        copyOut(&out, arrayPos, 1);
    }
    template<class T>
    T ResizingCircularBuffer<T>::peek(size_t pos) {
        T out;
        peek(out, pos);
        return out;
    }
    template<class T>
    bool ResizingCircularBuffer<T>::peekSafe(T& out, size_t pos) {
        if (pos > readable()) return false;
        peek(out, pos);
        return true;
    }
    template<class T>
    std::optional<T> ResizingCircularBuffer<T>::peekSafe(size_t pos) {
        if (pos > readable()) return {};
        return peek(pos);
    }

    template<class T>
    void ResizingCircularBuffer<T>::write(const T* values, size_t len) {
        size_t space = (end >= start) ? (length - end + start - 1) : (start - end - 1);
        if (len > space)
            expand(std::bit_ceil(length + (len - space)));
        size_t firstLen = std::min(len, length - end);
        size_t secondLen = len - firstLen;
        copyIn(end, values, firstLen);
        end += firstLen;
        if (end == length) end = 0;
        copyIn(end, values + firstLen, secondLen);
        end += secondLen;
    }
    template<class T>
    void ResizingCircularBuffer<T>::write(const std::vector<T>& values) {
        return write(values.data(), values.size());
    }
    template<class T>
    template<size_t N>
    void ResizingCircularBuffer<T>::write(const std::array<T, N>& values) {
        return write(values.data(), N);
    }
    template<class T>
    size_t ResizingCircularBuffer<T>::read(T* out, size_t count) {
        size_t available = readable();
        if (count > available)
            count = available;
        size_t firstCount = std::min(count, length - start);
        size_t secondCount = count - firstCount;
        copyOut(out, start, firstCount);
        start += firstCount;
        if (start == length) start = 0;
        copyOut(out + firstCount, start, secondCount);
        start += secondCount;
        return count;
    }
    template<class T>
    size_t ResizingCircularBuffer<T>::read(std::vector<T>& out) {
        return read(out.data(), out.size());
    }
    template<class T>
    template<size_t N>
    size_t ResizingCircularBuffer<T>::read(std::array<T, N>& out) {
        return read(out.data(), N);
    }

    template<class T>
    size_t ResizingCircularBuffer<T>::readable() const {
        return (end >= start) ? (end - start) : (length - start + end);
    }

    template class ResizingCircularBuffer<char>;
}

void ResizingCircularBuffer<char>::write(const std::string_view& str) {
    //log::info("ResizingCircularBuffer writing string of length {} \"{}\"", str.size(), escapeString(std::string(str)));
    return Super::write(str.data(), str.size());
}
void ResizingCircularBuffer<char>::read(std::string& out, size_t len) {
    out.resize_and_overwrite(len, [this](char* buf, size_t size) {
        return Super::read(buf, size);
    });
    //log::info("ResizingCircularBuffer read string of length {} \"{}\"", out.size(), escapeString(out));
}
std::string ResizingCircularBuffer<char>::read(size_t len) {
    std::string out;
    read(out, len);
    return out;
}