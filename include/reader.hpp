#pragma once

#include <vector>
#include <cstring>

#include <unistd.h>

namespace itch
{

template <size_t BufSize = 1024 * 1024>
class Reader
{
    static const size_t buf_size = BufSize;
public:
    Reader(int fd) 
    :m_fd(fd), m_beg(0), m_end(0), m_buf(buf_size) {};

    bool available(const size_t n)
    {
        return m_beg + n <= m_end;
    }

    void read(const size_t n)
    {
        if (m_beg + n >= buf_size)
        {
            std::memcpy(&m_buf[0], &m_buf[m_beg], m_end - m_beg);
            m_end = m_end - m_beg;
            m_beg = 0;
        }

        while (!available(n))
        {
            int bytes = ::read(m_fd, &m_buf[m_end], buf_size - m_end);
            if (bytes <= 0)
                break;
            m_end += bytes;
        }
    }

    bool ensure(const size_t n)
    {
        if (available(n))
            return true;
        
        read(n);

        return available(n);
    }

    void advance(const int n)
    {
        m_beg += n;
    }

    char* get(const int pos)
    {
        return &m_buf[m_beg + pos];
    }

    char* get()
    {
        return &m_buf[m_beg];
    }

private:
    int m_fd;
    int m_beg;
    int m_end;
    std::vector<char> m_buf;
};


}