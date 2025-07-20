
namespace jgw
{
    #define CLASS_DEF(T)                 \
        T();                             \
        ~T();                            \
        T(const T&);                     \
        T(T&&);                          \
        T& operator=(const T&);          \
        T& operator=(T&&);

    #define CLASS_COPY_MOVE_DELETE(T)    \
        T(const T&) = delete;            \
        T(T&&) = delete;                 \
        T& operator=(const T&) = delete; \
        T& operator=(T&&) = delete;
}