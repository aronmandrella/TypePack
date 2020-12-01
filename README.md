# TypePack
### Klasa do operacji na typach w czasie kompilacji

TypePack pozwala przechowywać wiele dowolnych typów a następnie wykonywać na nich operacje w czasie kompilacji (metaprograming). Poszczególne funkcje pozwalają np. zliczyć ile typów danego typu jest w paczce typów. Lista funkcji:

```C++
template<typename... Us>
struct TypePack final
{
  //--------------------- STALE WARIADCZNE STATYCZNE OPISUJACE PACZKE ---------------------//

  using size = std::integral_constant<size_t, sizeof...(Us)>;
  using sum_sizeof = std::integral_constant<size_t, tpack::sum_sizeof<Us...>()>;
  using max_alignof = std::integral_constant<size_t, tpack::max_alignof<Us...>()>;
  using sum_sizeof_aligned = std::integral_constant<size_t, tpack::sum_sizeof_aligned<Us...>()>;
  using is_empty = std::integral_constant<bool, sizeof...(Us) == 0>;

  template<typename T> using index_of = tpack::index_of<T, Us...>;

  //Przyklad uzycia 1 : pack::size() // ilosc elementow w paczce
  //Przyklad uzycia 2 : pack::size::value // ilosc elementow w paczce
  //Przyklad uzycia 5 : pack::index_of<float>() // index wystepowania typu float w paczce


  //------- FUNKTORY SPRAWDZAJACE WYSTEPOWANIE DANEGO TYPU - MOZNA UZYC JAKO FILTR ------//

  template<typename T> using contains = std::integral_constant<bool,   tpack::contains<T, Us...>()>;
  template<typename T> using contains_once = std::integral_constant<bool,   tpack::contains_once<T, Us...>()>;
  template<typename T> using contains_count = std::integral_constant<size_t, tpack::contains_count<T, Us...>()>;
                       using contains_unique = std::integral_constant<bool,   tpack::contains_unique<Us...>()>;

  template <template<typename> typename T>
  using check_all = std::integral_constant<bool, tpack::check_all<T, Us...>()>;
  template <template<typename> typename T>
  using check_any = std::integral_constant<bool, tpack::check_any<T, Us...>()>;
  template <template<typename> typename T>
  using check_count = std::integral_constant<size_t, tpack::check_count<T, Us...>()>;

  // Przyklad uzycia 1 : pack::contains<int>()             // 1 jesli int jest przynajmniej raz w paczce, 0 jesi nie
  // Przyklad uzycia 2 : pack2::filter<pack1::contains>()  // wynik to paczka elementow wspolnych
  // Przyklad uzycia 3 : pack3::check_all<std::is_empty>() // 1 jesli chociaz jeden typ to typ pusty


  //------- TYPY POZWALAJACE POBRAC TYP NA DANYM INDEKSIE LUB OD/WYPAKOWAC PACZKE -------//

  template<template<class...>class T>  using unpack_into = T<Us...>;
	template<typename T> using unpack_from = typename tpack::unpack_from<T>::type;
  template<size_t Index> using get = typename tpack::get<Index, Us...>::type;

	// Przyklad uzycia 1 : TypePack<int,char>::get<1>	// zwroci typ int
	// Przyklad uzycia 2 : TypePack<int,char>::unpack_into<std::tuple>	// zwroci typ std::tuple<int,char>
	// Przyklad uzycia 3 : TypePack<>::unpack_from<std::tuple<int,char>>	// zwroci typ TypePack<int,char>


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

	// Przyklad uzycia 1 : TypePack<int,char>
	// for_each_type([]{auto i, auto t}{ std::cout<<typeid(typename decltype(t)::type).name();})


  //------------------ STALE I TYPY PRZYDATNE PRZY ALOKOWANIU PAMIECI -------------------//

  // Odpowiednia zaalokowana pamiec na typy.
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
```
