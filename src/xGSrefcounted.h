#pragma once

template<typename T>
class xGSrefcountedImpl : public T
{
public:
    xGSrefcountedImpl() :
        p_count(1)
    {}

    virtual ~xGSrefcountedImpl()
    {}

    void AddRef() override
    {
        // TODO: interlocked?
        ++p_count;
    }

    void Release() override
    {
        // TODO: interlocked?
        --p_count;
        if (p_count == 0) {
            delete this;
        }
    }

private:
    int p_count;
};
