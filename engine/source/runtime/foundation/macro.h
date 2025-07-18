
namespace jgw
{
    #define CLASS_DEF(T)        \
        T();                    \
        ~T();                   \
        T(const T&);            \
        T(T&&);                 \
        T& operator=(const T&); \
        T& operator=(T&&);
}