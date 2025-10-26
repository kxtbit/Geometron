#pragma once

#ifndef GEOMETRON_RESIZINGCIRCULARBUFFER_HPP
#define GEOMETRON_RESIZINGCIRCULARBUFFER_HPP

namespace _ResizingCircularBuffer_base {
    template<class T>
    class ResizingCircularBuffer {
    protected:
        static_assert(std::is_trivially_copyable_v<T>, "type is not trivially copyable");

        T* array;
        size_t length;
        size_t start;
        size_t end;

        void copyIn(size_t dst, const T* src, size_t count);
        void copyOut(T* dst, size_t src, size_t count);
        void move(size_t dst, size_t src, size_t count);

        void expand(size_t newLength);
        void expandIfNeeded();

    public:
        ResizingCircularBuffer();
        explicit ResizingCircularBuffer(size_t initialLen);

        void write(const T& value);
        bool read(T& out);
        std::optional<T> read();

        void peek(T& out, size_t pos);
        T peek(size_t pos);
        bool peekSafe(T& out, size_t pos);
        std::optional<T> peekSafe(size_t pos);

        void write(const T* values, size_t len);
        void write(const std::vector<T>& values);
        template<size_t N>
        void write(const std::array<T, N>& values);
        size_t read(T* out, size_t count);
        size_t read(std::vector<T>& out);
        template<size_t N>
        size_t read(std::array<T, N>& out);

        size_t readable() const;
    };
}

template<class T>
class ResizingCircularBuffer : public _ResizingCircularBuffer_base::ResizingCircularBuffer<T> {
    using Super = _ResizingCircularBuffer_base::ResizingCircularBuffer<T>;
public:
    ResizingCircularBuffer() : Super(1) {}
    ResizingCircularBuffer(size_t s) : Super(s) {}
};
template<>
class ResizingCircularBuffer<char> : public _ResizingCircularBuffer_base::ResizingCircularBuffer<char> {
    using Super = _ResizingCircularBuffer_base::ResizingCircularBuffer<char>;
public:
    ResizingCircularBuffer() : Super(1) {}
    ResizingCircularBuffer(size_t s) : Super(s) {}

    void write(const std::string_view& str);
    void read(std::string& out, size_t len);
    std::string read(size_t len);
};

#endif //GEOMETRON_RESIZINGCIRCULARBUFFER_HPP