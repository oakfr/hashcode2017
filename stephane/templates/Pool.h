template <typename T, int chuncksize>
class Pool
{
public:
    ~Pool();
    T* get();
    void drop(T* ptr);
private:
    std::vector<T*> objs;
    std::vector<T*> chuncks;
};

template <typename T, int chuncksize>
Pool<T, chuncksize>::~Pool()
{
    for (std::size_t i = 0; i < chuncks.size(); ++i)
        free(chuncks[i]);
}

template <typename T, int chuncksize>
T* Pool<T, chuncksize>::get()
{
    if (objs.size() == 0)
    {
        T* chunck = static_cast<T*>(malloc(sizeof(T) * chuncksize));
        for (std::size_t i = 0; i < chuncksize; ++i)
            objs.push_back(chunck+i);
        chuncks.push_back(chunck);
    }

    T* obj = objs.back();
    objs.pop_back();
    return obj;
}

template <typename T, int chuncksize>
void Pool<T, chuncksize>::drop(T* ptr)
{
    objs.push_back(ptr);
}

template <typename T, int chuncksize>
class Pooled
{
public:
    virtual ~Pooled(){};

    void* operator new(std::size_t /*size*/)
    {
        return getPool().get();
    }

    void operator delete(void* ptr)
    {
        getPool().drop((T*)ptr);
    }
private:
    inline static Pool<T, chuncksize>& getPool()
    {
        static Pool<T, chuncksize> pool;
        return pool;
    }
};
