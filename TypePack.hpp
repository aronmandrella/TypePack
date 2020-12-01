// Autor: Aron Mandrella

#pragma once
#include <type_traits>
#include <iostream>
#include <typeinfo>
#include <tuple>
#include <utility>

namespace tpack
{


template<typename... Us>
struct TypePack;


// Prosta klasa pozwalajaca zapakowac dowlony typ w tej klasie by nastepnie
// moc przekazac go w postaci obiektu. Do szablonu mozna przekazac wiecej
// niz jeden typ ale zapisany zostanie tylko ten podany jako pierwszy.
// Przyklad: TypeWrapper<int> iw; --> (typename decltype(iw))::type i;
template<typename T, typename... Us>
struct TypeWrapper { using type = T; };


template<size_t...I>
using Sequence = std::integer_sequence<size_t, I...>;



/*
--- Informacje przydatne przy analizie kodu: ---

Zasada dzialania struktury rekurencyjnej:

	template <size_t N, typename T, typename U, typename... Us>
	struct SizeOffset : SizeOffset<N + sizeof(U), T, Us...> {};
	template <size_t N, typename T, typename... Us>
	struct SizeOffset<N, T, T, Us...> : std::integral_constant<size_t, N> {};

	Kompilator tak dlugo bd sie zaglebial rekurencyjnie w SizeOffset, wykonujac przy okazji dodawanie
	az zabraknie typow w Us (za kazdym razem tu jeden odpada) lub az parametry templatki beda odpowiadac
	jakiejs specializacji tej klasy. W tym wypadku az drugi podany typ T bedzie odpowiadal typowi U.


Rozwijanie operatora ... :

	By zrobic cos po kazdym podanym typie trzeba rozwinac podana paczke. Mozna to zrobic poprzez
	takie oszukanie kompilatora:

	using Swallow_t = int[];
	Swallow_t{ (( std::cout<<typeid(Ts).name() ), 0)... };


Funckje przydatne przy pisaniu generalnych bibliotek:

	std::forward<T>(t)
	new (address) T(args)				//Tworzy obiekt w juz zaalokowanej pamieci
	std::addressof(t)					//Zwraca adres nawet gdy przeladowano operator &
	std::is_same_v<T1,T2>				//Sprawdza czy typy sa takie same zwraca bool
	std::index_sequence_for<Ts...>{}	//<-- 0,1,3,4,..., sizeof...(Ts)
*/




// -------------- PROSTE OPERACJE NA PACZCE TYPOW -------------- //


// Sprawdza czy podany typ wystepuje posrod typow podanych w paczce.
// Przyklad uzycia: contains<char,int,float>(); // <--- false bo char to nie int ani float
template <typename T, typename... Us>
constexpr bool contains()
{
	size_t count = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( count += std::is_same<T,Us>::value ? 1u : 0u ), 0)... };
	return (count >= 1) ? true : false;
}



// Sprawdza ile razy podany typ wystepuje posrod typow podanych w paczce.
// Przyklad uzycia: contains_count<char,char,char,float>(); // <--- 2 bo wystepuja dwa chary
template <typename T, typename... Us>
constexpr size_t contains_count()
{
	size_t count = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( count += std::is_same<T,Us>::value ? 1u : 0u ), 0)... };
	return count;
}



// Sprawdza czy podany typ wystepuje dokladnie raz posrod typow podanych w paczce.
// Przyklad uzycia: contains_once<char,int,float>(); // <--- false bo char to nie int ani float
template <typename T, typename... Us>
constexpr bool contains_once()
{
	size_t count = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( count += std::is_same<T,Us>::value ? 1u : 0u ), 0)... };
	return (count == 1) ? true : false;
}



// Sprawdza kazdy typ w paczce jest unikalny tz. wystepuje w niej dokladnie raz.
// Przyklad uzycia: has_type<char,int,float>(); // <--- false bo char to nie int ani float
template <typename... Us>
constexpr bool contains_unique()
{
	bool unique = true;
	using Swallow = int[];
	(void)Swallow{0, (void( unique = (contains_once<Us, Us...>()) ? unique : false ), 0)... };
	return unique;
}



// Pozwala sprawdzic czy wszystkie typy w paczce spelniaja okreslony warunek. Warunek okresla sie
// za pomoca struktury przyjmujacej jeden typ w szablonie i majacej pole value typu bool.
// Przyklad uzycia: check_all<is_TypePack, TypePack<int>, int, char>() // <--- 0 bo dwa nie spelniaja
template <template<typename> typename T, typename... Us>
constexpr bool check_all()
{
	bool result = true;
	using Swallow = int[];
	(void)Swallow{0, (void( result = ( T<Us>::value ) ? result : false ), 0)... };
	return result;
}



// Pozwala sprawdzic czy jakikolwiek typ w paczce spelnia okreslony warunek. Warunek okresla sie
// za pomoca struktury przyjmujacej jeden typ w szablonie i majacej pole value typu bool.
// Przyklad uzycia: check_any<is_TypePack, TypePack<int>, int, char>() // <--- 1 bo jeden spelnia
template <template<typename> typename T, typename... Us>
constexpr bool check_any()
{
	bool result = false;
	using Swallow = int[];
	(void)Swallow{0, (void( result = ( T<Us>::value ) ? true : result ), 0)... };
	return result;
}



// Pozwala sprawdzic ile typow w paczce spelnia okreslony warunek. Warunek okresla sie
// za pomoca struktury przyjmujacej jeden typ w szablonie i majacej pole value typu bool.
// Przyklad uzycia: check_any<is_TypePack, TypePack<int>, int, char>() // <--- 1 bo jeden spelnia
template <template<typename> typename T, typename... Us>
constexpr size_t check_count()
{
	size_t result = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( result += ( T<Us>::value ) ? 1u : 0u ), 0)... };
	return result;
}



// Za pomoca tej rekurencyjnej struktury mozna sprawdzic na ktorym miejscu posrod wymienionych
// typow pojawia sie wskazany typ. Jesli nie wystepuje lub wystepuje wiecej niz raz wystapi blad.
// Przyklad uzycia: index_of<int ,char,int,float>::value var; // <-- 1
template <typename T, typename... Us>
struct index_of;

namespace Impl
{
    template <size_t N, typename T, typename U, typename... Us>
    struct index_of : index_of<N + 1u, T, Us...>{};

    template <size_t N, typename T, typename... Us>
    struct index_of<N, T, T, Us...> : std::integral_constant<size_t, N>{};
}
template <typename T, typename... Us>
struct index_of : Impl::index_of<0, T, Us...>
{ static_assert(contains_once<T, Us...>(), "Type T has to occur in Us exactly once."); };



// Za pomoca tej rekurencyjnej struktury mozna pobrac N-ty (od zera) typ z paczki.
// Przyklad uzycia: get<1, char,int,float>::type var; // <-- to samo co int var;
template<size_t index, typename...Ts>
struct get;

template<size_t index>
struct get<index> { static_assert(0 * index, "sizeof...(Ts) can't be == 0"); };

template<typename T, typename... Us>
struct get<0u, T, Us...> { using type = T; };

template<size_t index, typename T, typename... Us>
struct get<index, T, Us...> : get<index - 1u, Us...>
{ static_assert(index <= sizeof...(Us) , "index has to be <= sizeof...(Us)"); };



// -------------- ALOKACJA PAMIECI NA ROZNE TYPY DANYCH -------------- //



// Sprawdza wartosc alignof dla wszystkich podanych typow i zwraca tą najwieksza.
// Przyklad uzycia: StorageSizeFor<char,int,float>(); // <--- 4 bo 4 >= 4 > 1
template <typename... Us>
constexpr size_t max_alignof()
{
	size_t align = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( align = alignof(Us) > align ? alignof(Us) : align), 0)... };
	return align;
}



// Zwraca sume wartosci sizeof dla wszystkich podanych typow.
// Przyklad uzycia: StorageSizeFor<char,int,float>(); // <--- 9 bo 4 + 1 + 4
template <typename... Us>
constexpr size_t sum_sizeof()
{
	size_t size = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( size += sizeof(Us) ), 0)... };
	return size;
}



// Zwraca minimalna ilosc bajtow potrzebna by przechowac w pamieci wszystkie podane typy,
// jak najblizej siebie, w podanej kolejnosci. W obliczeniach jest uzwgledniany padding ktory
// trzeba dodac by kazdy typ mogl znalesc sie na adresie ktory bedzie spelnial wymagania align.
// Uwaga: poczatkowy adres musi spelniac warunek - adres % max_align<Ts...>() == 0.
// Przyklad uzycia: sum_sizeof_aligned<char,short,float>(); // <--- 8 bo 1 + 1p + 2 + 4
template <typename... Us>
constexpr size_t sum_sizeof_aligned()
{
	size_t size = 0u;
	using Swallow = int[];
	(void)Swallow{0, (void( size += (alignof(Us) - size) % alignof (Us) + sizeof(Us) ), 0)... };
	return size;
}



// Alokuje pamiec w ktorej mozna zapisac obiekty podanych klas, w podanej kolejnosci, z uwzglednieniem align.
// Przyklad uzycia: aligned_storage<char,short,float> storage; // <--- sizeof(storage) to 8 bo 1 + 1p + 2 + 4
template <typename... Us>
using aligned_storage = std::aligned_storage_t<sum_sizeof_aligned<Us...>(), max_alignof<Us...>()>;



// Za pomoca tej rekurencyjnej struktury mozna sprawdzic jaki bylby offset w bajtach pomiedzy poczatkiem
// pamieci a pierwszym wystapieniem obiektu o wskazanym typie jesli umiescimy w pamieci obiekty zawsze
// jeden obok drugiego (padding nie jest dodawany gdy align obiektu sie nie zgadza).
// Przyklad uzycia: byte_offset<0, int, char, int, float>::value // <--- 1 bo sizeof(char) to 1
template <typename T, typename... Us>
struct byte_offset;

namespace Impl
{
    template <size_t N, typename T, typename... Us>
    struct byte_offset;

    template <size_t N, typename T, typename U, typename... Us>
    struct byte_offset<N, T, U, Us...> : byte_offset<N + sizeof(U), T, Us...> {};

    template <size_t N, typename T, typename... Us>
    struct byte_offset<N, T, T, Us...> : std::integral_constant<size_t, N> {};

    template <size_t N, typename T>
    struct byte_offset<N, T> { static_assert(0 * N, "Type T wasn't found in Us..."); };
}
template <typename T, typename... Us>
struct byte_offset : Impl::byte_offset<0, T, Us...> {};



// Za pomoca tej rekurencyjnej struktury mozna sprawdzic jaki bylby offset w bajtach pomiedzy poczatkiem
// pamieci a pierwszym wystapieniem obiektu o wskazanym typie jesli umiescimy w pamieci obiekty tak by
// ich align sie zgadzal (czasem moze zostac dodany padding miedzy sasiednimi obiektami).
// Przyklad uzycia: SizeOffset2<0, int, char, int, float>::value // <--- 4 bo sizeof(char) to 1 + 3p (align to 4)
template <size_t N, size_t align, typename T, typename U, typename... Us>
struct _SizeOffset2 : _SizeOffset2<N + sizeof(U) + (align - sizeof(U)) % align, align, T, Us...> {};
template <size_t N, size_t align, typename T, typename... Us>
struct _SizeOffset2<N, align, T, T, Us...> : std::integral_constant<size_t, N> {};
template <size_t N, typename T, typename... Us>
struct SizeOffset2 : _SizeOffset2<N, max_alignof<Us...>() , T, Us...> {};



// -------------- ZLOZONE OPERACJE NA PACZCE TYPOW -------------- //


// Pozwala polaczyc dowolna ilosc typow oraz typow zawartych w paczkach typow w nowa paczke typow.
// Przyklad uzycia: Join<TypePack<long, float>, int, TypePack<char, double>>::type;
// Wynik:           TypePack<long, float, int, char, double>
template<typename... Vs>
struct join;

template<>
struct join<> { using type = TypePack<>; };

template<typename U, typename... Vs>
struct join<U, Vs...> : join<TypePack<U>, Vs...> {};

template<typename... Ts>
struct join<TypePack<Ts...>> { using type = TypePack<Ts...>; };

template<typename... Ts, typename U, typename... Vs>
struct join<TypePack<Ts...>, U, Vs...> : join<TypePack<Ts..., U>, Vs...> {};

template<typename... Ts, typename... Us, typename... Vs>
struct join<TypePack<Ts...>, TypePack<Us...>, Vs...> : join<TypePack<Ts... , Us...>, Vs...> {};



// Za pomoca tej rekurencyjnej struktury mozna usunac count pierwszych typow z paczki.
// Jesli jako typ zostanie podana paczka typow to jej zawartosc nie bedzie wycigana.
// Przyklad uzycia: remove_front<2, char,int,float>::type var; // <-- to samo co TypePack<float>;
template<size_t count, typename...Ts>
struct remove_front;

template<size_t count>
struct remove_front<count> { using type = TypePack<>; };

template<typename...Us>
struct remove_front<0u, Us...> { using type = TypePack<Us...>; };

template<size_t count, typename T, typename... Us>
struct remove_front<count, T, Us...> : remove_front<count - 1u, Us...>
{ static_assert(count <= sizeof...(Us) + 1, "Count is to big. There is nothing else to remove."); };



// Za pomoca tej rekurencyjnej struktury mozna usunac count ostatnich elementow typow z paczki.
// Jesli jako typ zostanie podana paczka typow to jej zawartosc nie bedzie wycigana.
// Przyklad uzycia: remove_front<2, char,int,float>::type var; // <-- to samo co TypePack<float>;
template<size_t count, typename...Ts>
struct remove_back;

namespace Impl
{
	template<typename TypePack, size_t count, size_t rest_size, typename...Ts>
	struct remove_back;

	template<typename... Ts, size_t count, typename...Us>
	struct remove_back<TypePack<Ts...>, count, count, Us...> { using type = TypePack<Ts...>; };

	template<typename... Ts, size_t count, size_t rest_size, typename U, typename...Vs>
	struct remove_back<TypePack<Ts...>, count, rest_size, U, Vs...> : remove_back<TypePack<Ts..., U>, count, sizeof...(Vs), Vs...> {};
}
template<size_t count>
struct remove_back<count> { using type = TypePack<>; };

template<typename...Us>
struct remove_back<0u, Us...> { using type = TypePack<Us...>; };

template<size_t count, typename... Vs>
struct remove_back<count, Vs...> : Impl::remove_back<TypePack<>, count, sizeof...(Vs), Vs...>
{
	static_assert(count <= sizeof...(Vs), "Count is to big. There is nothing else to remove.");
};



// Przyjmuje filtr, dowolna ilosc roznych typow i zwraca paczke typow w ktorej beda  tylko te z podanych typow
// ktore spelnialy podany filtr (w postaci klasy szablonowej z polem value). Jesli jako typ zostanie podana paczka
// typow to jej zawartosc nie bedzie wycigana. Typ paczki typow bedzie wartoscia wejsciowa dla filtru.
// Przyklad wywolania filtra: F<KolejnyPodanyTyp>::value
// Przyklad uzycia:           Filter<is_integral, int, long, float, char>
// Wynik:					  to samo co TypePack<int, long, char>
// BONUS: Wytłumaczenie składni:
// https://stackoverflow.com/questions/6970219/x-is-not-a-template-error
template<template<typename>typename F, typename... Vs>
struct filter;

namespace Impl
{
    template<typename TypePack, template<typename>typename F, typename T, bool value, typename... Us>
    struct filter;

	template<typename... Ts, template<typename>typename F, typename T>
	struct filter<TypePack<Ts...>, F, T, 1> { using type = TypePack<Ts..., T>; };
	template<typename... Ts, template<typename>typename F, typename T>
	struct filter<TypePack<Ts...>, F, T, 0> { using type = TypePack<Ts...>; };

    template<typename... Ts, template<typename>typename F, typename T, typename U, typename... Vs>
    struct filter<TypePack<Ts...>, F, T, 1, U, Vs...> : filter<TypePack<Ts..., T>,  F, U, F<U>::value, Vs...> {};
    template<typename... Ts, template<typename>typename F, typename T, typename U, typename... Vs>
    struct filter<TypePack<Ts...>, F, T, 0, U, Vs...> : filter<TypePack<Ts...>,     F, U, F<U>::value, Vs...> {};
}
template<template<typename>typename F>
struct filter<F> { using type = TypePack<>; };

template<template<typename>typename F, typename U, typename... Vs>
struct filter<F, U, Vs...> : Impl::filter<TypePack<>, F, U, F<U>::value, Vs...> {};



// Przyjmuje filtr, dowolna ilosc roznych typow i zwraca paczke typow w ktorej beda  tylko te z podanych typow
// ktore nie spelnialy podango filtru (w postaci klasy szablonowej z polem value). Jesli jako typ zostanie podana paczka
// typow to jej zawartosc nie bedzie wycigana. Typ paczki typow bedzie wartoscia wejsciowa dla filtru.
// Przyklad uzycia: filter_inv<std::is_integral, int, long, float, char> // <--- to samo co TypePack<float>
template<template<typename>typename F, typename... Vs>
struct filter_inv;

namespace Impl
{
    template<template<typename>typename F, typename T>
    struct invert_filter : std::integral_constant<bool, !F<T>::value> {};
}
template<template<typename>typename F>
struct filter_inv<F> { using type = TypePack<>; };

template<template<typename>typename F, typename U, typename... Vs>
struct filter_inv<F, U, Vs...>
{
    template<typename T> using FN = Impl::invert_filter<F, T>;
    using type = typename filter<FN, U, Vs...>::type;
};



// Przyjmuje modfikator, dowolna ilosc roznych typow i zwraca paczke typow w ktorej beda typy wejsciowe po zastosowaniu
// podanego modyfikatora (np dodajacego const lub refernecje typu L lub R) Jesli jako typ zostanie podana paczka
// typow to jej zawartosc nie bedzie wycigana. Typ paczki typow bedzie wartoscia wejsciowa dla filtru.
// Przyklad wywolania modyfikatora: F<KolejnyPodanyTyp>::type
// Przyklad uzycia:					modify<std::remove_const, int, const long, float, const char>
template<template<typename>typename F, typename... Us>
struct modify;

namespace Impl
{
	template<typename TypePack, template<typename>typename F, typename T, typename... Us>
	struct modify;

	template<typename... Ts, template<typename>typename F, typename T>
	struct modify<TypePack<Ts...>, F, T> { using type = TypePack<Ts..., typename F<T>::type>; };

	template<typename... Ts, template<typename>typename F, typename T, typename... Us>
	struct modify<TypePack<Ts...>, F, T, Us...> : modify<TypePack<Ts..., typename F<T>::type>, F, Us...> {};
}
template<template<typename>typename F>
struct modify<F> { using type = TypePack<>; };

template<template<typename>typename F, typename T, typename... Us>
struct modify<F, T, Us...> : Impl::modify<TypePack<>, F, T, Us...> {};



// Sprawdza czy w paczce typow podanej jako pierwszej znajduja sie typy wymienione po niej. Jesli tak to zostaje
// zapisany index elementu w paczce typow ktory wystepowal rowniesz wsrod wymienionych typow. Jesli jako typ zostanie
// podana paczka  typow to jej zawartosc nie bedzie wycigana. Typ paczki typow bedzie traktowany jako osobny typ.
// Przyklad uzycia: contains_indexes<TypePack<int,float,char>, int, char>::type // <--- to samo co Sequence<0,2>;
template<typename TypePack, typename...Ts>
struct contains_indexes;

namespace Impl2 /// by nie wychwycilo Impl::index_of tylko index_of
{
	template<typename TypePack, typename Sequence, typename T, bool value, typename... Us>
	struct contains_indexes {};

	template<typename... Ts, size_t...I, typename T>
	struct contains_indexes<TypePack<Ts...>, Sequence<I...>, T, 0> { using type = Sequence<I...>; };
	template<typename... Ts, size_t...I, typename T>
	struct contains_indexes<TypePack<Ts...>, Sequence<I...>, T, 1> { using type = Sequence<I..., index_of<T, Ts...>::value>; };

	template<typename... Ts, size_t...I, typename T, typename U, typename...Vs>
	struct contains_indexes<TypePack<Ts...>, Sequence<I...>, T, 0, U, Vs...> : contains_indexes<TypePack<Ts...>, Sequence<I...>, U, contains<U, Ts...>(), Vs...> {};
	template<typename... Ts, size_t...I, typename T, typename U, typename...Vs>
	struct contains_indexes<TypePack<Ts...>, Sequence<I...>, T, 1, U, Vs...> : contains_indexes<TypePack<Ts...>, Sequence<I..., index_of<T, Ts...>::value>, U, contains<U, Ts...>(), Vs...> {};
}
template<typename...Ts>
struct contains_indexes<TypePack<Ts...>> { using type = Sequence<>; };

template<typename...Ts, typename U, typename...Vs>
struct contains_indexes<TypePack<Ts...>, U, Vs...> : Impl2::contains_indexes<TypePack<Ts...>, Sequence<>, U, contains<U, Ts...>(), Vs...> {};



// Przyjmuje dowolna ilosc typow i generuje liste typow na ktorej kazdy typ wystepoje tylko raz nawet jesli powtarzal
// sie wsrod podanych typow. Jesli jako typ zostanie podana paczka  typow to jej zawartosc nie bedzie wycigana.
// Typ paczki typow bedzie traktowany jako osobny typ (rozny w zaleznosci od zawartosci paczki).
// Przyklad uzycia: remove_repetitions<int, int, long, char>::type // <--- to samo co TypePack<int, long, char>
template<typename... Vs>
struct remove_repetitions;

namespace Impl
{
    template<typename TypePack, typename T, bool value, typename... Us>
    struct remove_repetitions{};

	template<typename... Ts, typename T>
	struct remove_repetitions<TypePack<Ts...>, T, 1> { using type = TypePack<Ts...>; };
	template<typename... Ts, typename T>
	struct remove_repetitions<TypePack<Ts...>, T, 0> { using type = TypePack<Ts..., T>; };

    template<typename... Ts, typename T, typename U, typename... Vs>
    struct remove_repetitions<TypePack<Ts...>, T, 1, U, Vs...> : remove_repetitions<TypePack<Ts...>,    U, contains<U, Ts...>(),    Vs...> {};
	template<typename... Ts, typename T, typename U, typename... Vs>
	struct remove_repetitions<TypePack<Ts...>, T, 0, U, Vs...> : remove_repetitions<TypePack<Ts..., T>, U, contains<U, Ts..., T>(), Vs...> {};
}
template<>
struct remove_repetitions<> { using type = TypePack<>; };

template<typename U, typename... Vs>
struct remove_repetitions<U, Vs...> : Impl::remove_repetitions<TypePack<>, U, 0, Vs...> {};



// Pozwala wyciagnac z dowolnego typu szablonowego, typy jakie zostaly podane w jego szablonie.
// Przyklad uzycia: unpack_from<std::tuple<int, char>>::type // <--- to samo co TypePack<int, char>
template<typename T>
struct unpack_from { using type = TypePack<T>; };

template<template<typename...> typename U, typename... Ts>
struct unpack_from<U<Ts...>> { using type = TypePack<Ts...>; };


// Przyjmuje dowolna ilosc typow i dzieli je na mniejsze paczki typow o okreslonym rozmiarze. Jesli jako typ
// zostanie podana paczka  typow to jej zawartosc nie bedzie wycigana. Typ paczki typow bedzie traktowany jako osobny typ.
// Przyklad uzycia: divide<2, int, long, char, float, void>::type // <--- to samo co TypePack<TypePack<int, long>, TypePack<char, float>, TypePack<void>>
template<size_t count, typename...Ts>
struct divide {};

namespace Impl
{
	template<typename finalTypePack, typename tempTypePack, size_t count, size_t size, typename... Vs>
	struct divide;

	template<typename...Ts, typename...Us, size_t count, size_t size>
	struct divide<TypePack<Ts...>, TypePack<Us...>, count, size> { using type = TypePack<Ts..., TypePack<Us...>>; };

	template<typename...Ts, typename...Us, size_t count, typename U, typename... Vs>
	struct divide<TypePack<Ts...>, TypePack<Us...>, count, count, U, Vs...> : divide<TypePack<Ts..., TypePack<Us...>>, TypePack<U>, count, 1, Vs...> {};

	template<typename...Ts, typename...Us, size_t count, size_t size, typename U, typename... Vs>
	struct divide<TypePack<Ts...>, TypePack<Us...>, count, size, U, Vs...> : divide<TypePack<Ts...>, TypePack<Us...,U>, count, sizeof...(Us) + 1, Vs...> {};
}
template<size_t count>
struct divide<count> { using type = TypePack<>; };

template<typename...Us>
struct divide<0u, Us...> { static_assert(0* sizeof...(Us), "U can't divide by zero."); };

template<typename...Us>
struct divide<1u, Us...> { using type = TypePack<TypePack<Us>...>; };

template<size_t count, typename...Vs>
struct divide<count, Vs...> : Impl::divide<TypePack<>, TypePack<>, count, 0, Vs...> {};


// ---------- FUNKCJE TYPU FOR EACH POZWALAJACE COS ZROBIC NA KAZDYM TYPIE PACZKI ----------//


// Funckcja pozwalajaca wywolac funckje na kazdym z podanych typow. Typy podaje sie w szablonie funkcji
// a funkcje callback jako argument funkcji. Jesli jako typ zostanie podana paczka typow to jej zawartosc
// nie bedzie wycigana. Typ paczki typow bedzie traktowany jako osobny typ (rozny w zaleznosci od zawartosci paczki).
// Do funckji callback zostana wyslane dwie wartosci typu: size_t i Type<T> (auto). Pierwszym argumentem bedzie
// index typu w paczce (od zara) a drugim obiekt w ktory jest zapakowany typ. Obiekt na typ mozna zamieniec
// przy uzyciu takiej formuly: 'typename decltype(type_obj)::type'.
// Przykladowe uzycie:
//      for_each_type<int, char, float>([](auto index, auto type_obj){ std::cout<<typeid(typename decltype(type_obj)::type).name(); });
// Wynik w konsoli: 'icf'
template<typename... Ts, typename Function>
void for_each_type(Function&& f)
{
    size_t index = 0;
    using Swallow = int[];
	(void)Swallow{0, (void( f(index++, TypeWrapper<Ts>()) ), 0)... };
}



// Funckcja pozwalajaca wywolac funckje na kazdym z elemencie klasy wariadyczneh - funkcja std::get<Index>
// musi miec przeladowanie dla typu 'Container' by funkcja mogla zadzialac. Jako drugi argument funkcji nalezy
// podac funkcje/funktur przyjmujaca dwa argumenty: size_t i auto. Pierwszy to bedzie index podany do funkcji
// 'std::get<Index>(c)' a drugi to bedzie wartosc zwrocona przez wywolanie tej funkcji.
// Przykladowe uzycie:
//          for_each_value(std::make_tuple(1,'a',3.f), [](auto, auto v){ std::cout<<v; });
namespace Impl
{
    template<typename Container, typename Function, size_t... Is>
    void for_each_value(Container& c, Function&& f, std::index_sequence<Is...>)
    {
        using Swallow = int[];
        (void)Swallow{0, (void( f(Is, std::get<Is>(c)) ), 0)... };
    }
}
template<template<typename...>typename Container, typename... Ts, typename Function>
void for_each_value(Container<Ts...>& c, Function&& f)
{
    Impl::for_each_value(c, std::forward<Function>(f), std::index_sequence_for<Ts...>{});
}
template<template<typename...>typename Container, typename... Ts, typename Function>
void for_each_value(Container<Ts...>&& c, Function&& f)
{
	Impl::for_each_value(c, std::forward<Function>(f), std::index_sequence_for<Ts...>{});
}



// ---------- ROZNE FILTRY PRZYDATNE PRZY FILTRACJI ORAZ TESTACH TYPOW ----------//


// Sprawdza czy zadany typ jest klasa typu TypePack - zawrtos moze byc dowolna.
// Przyklad: is_TypePack<TypePack<int, float>>::value // <-- da wartosc 1
template<typename T>
struct is_TypePack : std::integral_constant<bool, false> {};
template<template<typename...>typename T, typename...Ts>
struct is_TypePack<T<Ts...>> : std::integral_constant<bool, std::is_same<T<Ts...>, TypePack<Ts...>>::value> {};



// --------- ROZNE PRZYDATNE FUNKCJE TYPU TYPE TRIATS UZYWAJACE TypePack --------//

// Pozwala wypakowac z typu funkcji, lambdy lub też z funktora ktorego operator () NIE jest
// wielokrotnie przeladowany typ wartosci zwracanej: type_result, oraz paczke typow opisujacych
// typy parametrow wejsciowych.
// Przyklad: functor_traits<std::function<int(char, float)>>::type_result --> int
// functor_traits<std::function<int(char, float)>>::type_pack_args --> TypePack<char,float>;
// Zasada dzialania: wypakowywany jest typ metody funktora (operatora ()) i pozniej normalnie.
// const dodane jest w typie metody w specializacji bo funktory sa z natury stale??.
// Uwaga: uzywajac std::bind trzeba kastowac do std::function bo w bind podane parametry
// nie sa pakowane na stale w obiekcie a sa traktowane jako domyslne (mozna dalej podac inne).
template <class Functor>
struct function_traits : public function_traits<decltype(&Functor::operator())> {};

template <typename Result, typename... Args>
struct function_traits<Result(Args...)>
{
    using type_result    = Result;
    using type_pack_args = TypePack<Args...>;
};

template <class Functor, typename Result, typename... Args>
struct function_traits<Result(Functor::*)(Args...)>
{
    using type_result    = Result;
    using type_pack_args = TypePack<Args...>;
};

template <class Functor, typename Result, typename... Args>
struct function_traits<Result(Functor::*)(Args...) const>
{
    using type_result    = Result;
    using type_pack_args = TypePack<Args...>;
};



// ----------- KLASA POZWLAJACA NA WYGODNE OPERACJE NA PACZKACH TYPOW -----------//

//Klasa paczek typów.
template<typename... Us>
struct TypePack final
{
    //--------------------- STALE WARIADCZNE STATYCZNE OPISUJACE PACZKE ---------------------//

	using size				 = std::integral_constant<size_t, sizeof...(Us)>;
	using sum_sizeof		 = std::integral_constant<size_t, tpack::sum_sizeof<Us...>()>;
	using max_alignof		 = std::integral_constant<size_t, tpack::max_alignof<Us...>()>;
	using sum_sizeof_aligned = std::integral_constant<size_t, tpack::sum_sizeof_aligned<Us...>()>;

	using is_empty			 = std::integral_constant<bool, sizeof...(Us) == 0>;

	template<typename T> using index_of = tpack::index_of<T, Us...>;

    //Przyklad uzycia 1 : pack::size()            // ilosc elementow w paczce
    //Przyklad uzycia 2 : pack::size::value       // ilosc elementow w paczce
    //Przyklad uzycia 5 : pack::index_of<float>() // index wystepowania typu float w paczce




    //------- FUNKTORY SPRAWDZAJACE WYSTEPOWANIE DANEGO TYPU - MOZNA UZYC JAKO FILTR ------//

    template<typename T> using contains         = std::integral_constant<bool,   tpack::contains<T, Us...>()>;
    template<typename T> using contains_once    = std::integral_constant<bool,   tpack::contains_once<T, Us...>()>;
    template<typename T> using contains_count   = std::integral_constant<size_t, tpack::contains_count<T, Us...>()>;
                         using contains_unique  = std::integral_constant<bool,   tpack::contains_unique<Us...>()>;

    template <template<typename> typename T>
    using check_all   = std::integral_constant<bool,   tpack::check_all<T, Us...>()>;
    template <template<typename> typename T>
    using check_any   = std::integral_constant<bool,   tpack::check_any<T, Us...>()>;
    template <template<typename> typename T>
    using check_count = std::integral_constant<size_t, tpack::check_count<T, Us...>()>;

    //Przyklad uzycia 1 : pack::contains<int>()             // 1 jesli int jest przynajmniej raz w paczce, 0 jesi nie
    //Przyklad uzycia 2 : pack2::filter<pack1::contains>()  // wynik to paczka elementow wspolnych
    //Przyklad uzycia 3 : pack3::check_all<std::is_empty>() // 1 jesli chociaz jeden typ to typ pusty




    //------- TYPY POZWALAJACE POBRAC TYP NA DANYM INDEKSIE LUB OD/WYPAKOWAC PACZKE -------//

    template<template<class...>class T>  using unpack_into = T<Us...>;
	template<typename T>				 using unpack_from = typename tpack::unpack_from<T>::type;
    template<size_t Index>               using get		   = typename tpack::get<Index, Us...>::type;

	//Przyklad uzycia 1 : TypePack<int,char>::get<1>					// zwroci typ int
	//Przyklad uzycia 2 : TypePack<int,char>::unpack_into<std::tuple>	// zwroci typ std::tuple<int,char>
	//Przyklad uzycia 3 : TypePack<>::unpack_from<std::tuple<int,char>>	// zwroci typ TypePack<int,char>




    //------------------------ ZLOZONE OPERACJE NA PACZKACH TYPOW ------------------------//

    template<typename... Ts>
    using append = typename tpack::join<TypePack<Us...>, Ts...>::type;

	template<size_t count>
	using remove_front = typename tpack::remove_front<count, Us...>::type;

	template<size_t count>
	using remove_back = typename tpack::remove_back<count, Us...>::type;

    template<template<typename>typename F>
    using filter = typename tpack::filter<F, Us...>::type;

    template<template<typename>typename F>
    using filter_inv = typename tpack::filter_inv<F, Us...>::type;

	template<template<typename>typename F>
	using modify = typename tpack::modify<F, Us...>::type;

	template<size_t count>	
	using divide = typename tpack::divide<count, Us...>::type;

	template<typename...Ts>
	using contains_indexes = typename tpack::contains_indexes<TypePack<Us...>, Ts...>::type;

    using remove_repetitions = typename tpack::remove_repetitions<Us...>::type;

	using underlying_types = typename
		modify<std::remove_reference>::template
		modify<std::remove_cv>::template
		modify<std::remove_pointer>::template
		modify<std::remove_all_extents>;

    //Przyklad uzycia 1  : TypePack<>::append<int, TypePack<char, float>>        // zwroci typ TypePack<int,char,float>
    //Przyklad uzycia 2  : TypePack<TypePack<int>>::append<TypePack<int>>        // zwroci typ TypePack<TypePack<int>, int>
	//Przyklad uzycia 3  : TypePack<int,char,float>::filter<std::is_integral>    // zwroci typ TypePack<int,char>
    //Przyklad uzycia 4  : TypePack<int,char,float>::filter_inv<std::is_integral>// zwroci typ TypePack<float>
	//Przyklad uzycia 5  : TypePack<int,char,float>::contains_indexes<int,float> // zwroci typ std::integer_sequence<size_t, 0, 2>;
	//Przyklad uzycia 6  : TypePack<int, const float>::modify<std::add_volatile> // zwroci typ TypePack<volatile int,volatile const float>
    //Przyklad uzycia 7  : TypePack<int,float,int,char>::remove_repetitions      // zwroci typ TypePack<int,float,char>
	//Przyklad uzycia 8  : TypePack<const int, int*, int&&>::underlying_types	 // zwroci typ TypePack<int,int,int>
	//Przyklad uzycia 9  : TypePack<int,char,float>::remove_back<2>		         // zwroci typ TypePack<int>
	//Przyklad uzycia 10 : TypePack<int, long, char, float, void>::divide<2>	 // zwroci typ TypePack<TypePack<int, long>, TypePack<char, float>, TypePack<void>>



	//----------------------- FUNKCJE TYPU FOR EACH (PO KAZDYM TYPIE) ----------------------//

	template<typename Function>
	static void for_each_type(Function&& f)
	{
		::for_each_type<Us...>(std::forward<Function>(f));
	}

	//Przyklad uzycia 1 : TypePack<int,char>
	//      for_each_type([]{auto i, auto t}{ std::cout<<typeid(typename decltype(t)::type).name();})



    //------------------ STALE I TYPY PRZYDATNE PRZY ALOKOWANIU PAMIECI -------------------//

    //Odpowiednia zaalokowana pamiec na typy.
    using aligned_storage = tpack::aligned_storage<Us...>;


   //------------------ FUNKCJA WYPISUJACA POZWALAJACA NA DEBUGOWANIE -------------------//

    static void _print()
    {
		std::cout << "List size:\t" << size() << std::endl << "Contents:\t";
        using Swallow = int[];
        (void)Swallow{0, (void( std::cout<<typeid(Us).name()<<", " ), 0)... };
		std::cout << "\n\n";
    }
};




/// TESTY WYKONYWANE W TRAKCIE KOMPILACJI (NIEKOMPLATNE)
namespace _compile_time_tests 
{
static_assert(std::is_same<get<2, int, float, double, char>::type, double>::value, "");

using pack  = TypePack<char, int, float, int, float, double>;
using upack = TypePack<char, int, float, double>;


//Testy sprawdzajace wystepowanie typow w paczce.
static_assert(pack::contains<double>() == true          ,"");
static_assert(pack::contains<float>() == true           ,"");
static_assert(pack::contains<long>() == false           ,"");
static_assert(pack::contains_once<char>() == true       ,"");
static_assert(pack::contains_once<float>() == false     ,"");
static_assert(pack::contains_once<long>() == false      ,"");
static_assert(pack::contains_unique() == false          ,"");
static_assert(upack::contains_unique() == true          ,"");


//Testy sałych okreslajacych wielkosc.
static_assert(pack::size() == 6                         ,"");
static_assert(pack::sum_sizeof() == 25                  ,"");
static_assert(pack::max_alignof() == 8                  ,"");
static_assert(pack::sum_sizeof_aligned() == 32          ,"");


//Sprawdzenie funkcionalnosci potrzebnej do ECS.
template<typename... Ts>
using System = TypePack<Ts...>;
template<typename... Ts>
using Systems = TypePack<Ts...>;
struct Tag1{};
struct Tag2{};
struct Tag3{};
struct Tag4{};
struct Component1{ int x; };
struct Component2{ int x; };
struct Component3{ int x; };
struct Component4{ int x; };
using System1 = System<Component1, Component2>;
using System2 = System<Component2, Component3, Component4, Component4, Component4>;
using System3 = System<Component2, Tag1>;
using System4 = System<Component2, Tag2>;
using System5 = System<Component2, Tag1, Tag2>;
using System6 = System<Component2, Component3, Component4, Tag3>;
using System7 = System<Component2, Component3, Component4, Tag4>;
using Signatures  = Systems<System1, System2, System3, System4, System5, System6, System7>;
using Structs     = System1::append<System2, System3, System4, System5, System6, System7>;
using UsedStructs = Structs::remove_repetitions;
using Components  = UsedStructs::filter_inv<std::is_empty>;
using Tags        = UsedStructs::filter<std::is_empty>;
static_assert(std::is_same<Components, TypePack<Component1, Component2, Component3, Component4>>::value, "FAIL");
static_assert(std::is_same<Tags, TypePack<Tag1, Tag2, Tag3, Tag4>>::value, "FAIL");
}
}