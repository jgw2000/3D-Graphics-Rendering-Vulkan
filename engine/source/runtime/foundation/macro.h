
namespace jgw
{
    // 五法则定义
    #define CLASS_DEF(T)            \
        ~##T();                     \
        ##T(const ##T&);            \
        ##T(##T&&);                 \
        ##T& operator=(const ##T&); \
        ##T& operator=(##T&&);
}