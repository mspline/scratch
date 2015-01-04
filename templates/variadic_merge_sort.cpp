#include <cstdio>
#include <type_traits>
#include <vector>

using namespace std;

// "_" is pronounced "list"

template <int...> struct _
{
    static vector<int> to_vector() { return vector<int>(); }
};

template <int X, int... Xs> struct _<X, Xs...>
{
    static vector<int> to_vector() { auto v = _<Xs...>::to_vector(); v.insert(v.begin(), X); return v; }
};

// cons

template <int, typename> struct cons;

template <int X, int... Xs> struct cons<X, _<Xs...>>
{
    using type = _<X, Xs...>;
};

// cond

template <bool, typename TT, typename> struct cond
{
    using type = TT;
};

template <typename TF, typename TT> struct cond<false, TT, TF>
{
    using type = TF;
};

// skip

template <size_t, typename> struct skip
{
};

template <int... Xs> struct skip<0, _<Xs...>>
{
    using type = _<Xs...>;
};

template <int X, int... Xs> struct skip<0, _<X, Xs...>>
{
    using type = _<X, Xs...>;
};

template <size_t N, int X, int... Xs> struct skip<N, _<X, Xs...>> : skip<N-1, _<Xs...>>
{
};

// take

template <size_t, typename> struct take;

template <int... Xs> struct take<0, _<Xs...>>
{
    using type = _<>;
};

template <int X, int... Xs> struct take<0, _<X, Xs...>>
{
    using type = _<>;
};

template <size_t N, int X, int... Xs> struct take<N, _<X, Xs...>> : cons<X, typename take<N-1, _<Xs...>>::type>
{
};

// slice

template <size_t I1, size_t I2, typename L> struct slice : take<I2-I1, typename skip<I1, L>::type>
{
};

// merge

template <typename, typename> struct merge;

template <> struct merge<_<>, _<>>
{
    using type = _<>;
};

template <int X, int... Xs> struct merge<_<X, Xs...>, _<>>
{
    using type = _<X, Xs...>;
};

template <int Y, int... Ys> struct merge<_<>, _<Y, Ys...>>
{
    using type = _<Y, Ys...>;
};

template <int X, int... Xs, int Y, int... Ys> struct merge<_<X, Xs...>, _<Y, Ys...>> :
    cond<
        X <= Y,
        typename cons<X, typename merge<_<Xs...>, _<Y, Ys...>>::type>,
        typename cons<Y, typename merge<_<X, Xs...>, _<Ys...>>::type>
    >::type
{
};

// mergesort

template <typename> struct mergesort;

template <> struct mergesort<_<>>
{
    using type = _<>;
};

template <int X> struct mergesort<_<X>>
{
    using type = _<X>;
};

template <int... Xs> struct mergesort<_<Xs...>> :
    merge<
        typename mergesort<typename slice<0,               sizeof...(Xs)/2, _<Xs...>>::type>::type,
        typename mergesort<typename slice<sizeof...(Xs)/2, sizeof...(Xs),   _<Xs...>>::type>::type
    >
{
};

int main()
{
    static_assert(is_same<
        mergesort<
            _<>>::type,
            _<>
    >::value, "");

    static_assert(is_same<
        mergesort<
            _<42>>::type,
            _<42>
    >::value, "");

    static_assert(is_same<
        mergesort<
            _<12, 11, 15, 13>>::type,
            _<11, 12, 13, 15>
    >::value, "");

    static_assert(is_same<
        mergesort<
            _<49, 11, -8, 345, 4, -18, 29, 11, 42>>::type,
            _<-18, -8, 4, 11, 11, 29, 42, 49, 345>
    >::value, "");
}
