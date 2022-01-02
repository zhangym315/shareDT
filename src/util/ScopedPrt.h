#ifndef _SCOPEDPRT_H_
#define _SCOPEDPRT_H_
template<typename T> class ScopedPtr {
  public:
    explicit ScopedPtr(T *p = nullptr) : _p(p)
    {
    }
    ~ScopedPtr()
    {
        if (_p) delete _p;
    }
    bool valid() const
    {
        return _p != nullptr;
    }

    T *get()
    {
        return _p;
    }
    const T *get() const
    {
        return _p;
    }
    T& operator*()
    {
        return *_p;
    }
    const T& operator*() const
    {
        return *_p;
    }

    T *operator->()
    {
        return _p;
    }

    const T *operator->() const
    {
        return _p;
    }

    T* release()
    {
        T *result = _p;
        _p = nullptr;
        return result;
    }

    void reset(T *p)
    {
        if (p != _p) {
            if (_p) delete _p;
            _p = p;
        }
    }

    void reset()
    {
        delete _p;
        _p = nullptr;
    }

    ScopedPtr& operator=(T *p)
    {
        reset(p);
        return *this;
    }

    T **pointerToPointer()
    {
        return &_p;
    }

    void swap(ScopedPtr& other)
    {
        T *t = other._p;
        other._p = _p;
        _p = t;
    }

  private:
    ScopedPtr(const ScopedPtr&) = delete;
	ScopedPtr& operator=(const ScopedPtr&) = delete;

    T *_p;
};

#endif //_SCOPEDPRT_H_
