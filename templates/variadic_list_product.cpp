#include <type_traits>

using namespace std;

// "_" is pronounced "typelist"

template <typename...> struct _
{
};

// concat

template <typename, typename> struct concat;

template <typename... Ts, typename... Us> struct concat<_<Ts...>, _<Us...>>
{
    typedef _<Ts..., Us...> type;
};

// foldr

template <template <typename, typename> class, typename, typename> struct foldr;

template <template <typename, typename> class F, typename Z> struct foldr<F, Z, _<>>
{
    typedef Z type;
};

template <template <typename, typename> class F, typename Z, typename T, typename... Ts> struct foldr<F, Z, _<T, Ts...>> : F<T, typename foldr<F, Z, _<Ts...>>::type>
{
};

// foldl

template <template <typename, typename> class, typename, typename> struct foldl;

template <template <typename, typename> class F, typename Z> struct foldl<F, Z, _<>>
{
    typedef Z type;
};

template <template <typename, typename> class F, typename Z, typename T, typename... Ts> struct foldl<F, Z, _<T, Ts...>> : foldl<F, typename F<Z, T>::type, _<Ts...>>
{
};

// join

template <typename> struct join;

template <typename... Ts> struct join<_<Ts...>> : foldl<concat, _<>, _<Ts...>>
{
};

// pairmap: [t0,t1,t2,t3,t4,...] -> [[t0,t1],[t0,t2],[t0,t3],[t0,t4],...] 

template <typename...> struct pairmap;

template <typename T, typename... Ts> struct pairmap<T, Ts...>
{
    typedef _<_<T, Ts>...> type;
};

// product

template <typename, typename> struct product;

template <typename... Ts, typename... Us> struct product<_<Ts...>, _<Us...>>
{
    typedef typename join<_<typename pairmap<Ts, Us...>::type...>>::type type;
};

int main()
{
    static_assert(is_same<
        product<_<bool, char, int>, _<float, double>>::type,
        _<_<bool, float>, _<bool, double>, _<char, float>, _<char, double>, _<int, float>, _<int, double>>
    >::value, "");

    static_assert(is_same<
        product<_<>, _<float, double>>::type,
        _<>
    >::value, "");

    static_assert(is_same<
        product<_<bool, char, int>, _<>>::type,
        _<>
    >::value, "");
}
