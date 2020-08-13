#ifndef _COMPILER_H_
#define _COMPILER_H_

/*
 * Helper macros for later in the file to detect what GCC extensions,
 * builtins, etc this version of clang supports:
 */
#if defined(__clang__) && defined(__has_attribute)
#  define CLANG_SUPPORTS_GNU_ATTRIBUTE(x) __has_attribute(x)
#else
#  define CLANG_SUPPORTS_GNU_ATTRIBUTE(x) 0
#endif

#if defined(__clang__) && defined(__has_cpp_attribute)
#  define CLANG_SUPPORTS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#  define CLANG_SUPPORTS_CPP_ATTRIBUTE(x) 0
#endif

/*
 * We only defined these macros for real gcc -- not other compilers which
 * set the __GNUC__ macro because the support some gcc extensions:
 */
#if defined(__GNUC__) && !(defined(__clang__) || defined(__INTEL_COMPILER))
#  define GCC_AT_LEAST_VERSION(maj, min) ((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))
#  define GCC_BELOW_VERSION(maj, min) ((__GNUC__ < (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ < (min)))
#else
#  define GCC_AT_LEAST_VERSION(maj, min) 0
#  define GCC_BELOW_VERSION(maj, min) 0
#endif

/*
 * Attributes for declaring functions or methods
 * as never throwing.   Use like:
 *
 * extern int foo() DECLARE_NEVER_THROWS;
 * [...]
 * int foo() DEFINE_NEVER_THROWS
 * {
 * 	return 1;
 * }
 *
 * We use C++11's "noexcept" keyword if it's available, otherwise we use
 * the older empty-throw()-specifier.  (According to the spec, empty-throw()
 * can still generate "bad_exception" although not every compiler supports that)
 *
 * Traditionally on g++/clang++ we also add "__attribute__((nothrow))" to
 * the declaration to make sure we get the strictest semantics available.
 * However, we don't do this now for C++11 compilers with final/override
 * macros available.  This is because to compile everywhere without warnings
 * the specifiers after a method declaration have to appear in a very
 * specific order:
 *   void meth() const noexcept override __attribute__((other_gcc_attrs));
 * Therefore we can't put "noexcept" and GNU attributes next to each other
 * without at least some compilers complaining.  However, these days
 * I think "noexcept" gives us everything the old "__attribute__((nothrow))"
 * used to so dropping the old pre-C++11 attribute isn't a big loss
 *
 * MSVC also offers the similar "__declspec(nothrow)" specifier, but it
 * has to be attached to the return type so it can't be used in these
 * macros.  Supposedly it just acts the same as "throw()" so we don't
 * bother with it.
 */
#  ifdef WITH_CXX_NOEXCEPT_KEYWORD
#    define DEFINE_NEVER_THROWS		noexcept
#  else /* !WITH_CXX_NOEXCEPT_KEYWORD */
#    define DEFINE_NEVER_THROWS		throw()
#  endif /* WITH_CXX_NOEXCEPT_KEYWORD */
#  if (CLANG_SUPPORTS_GNU_ATTRIBUTE(nothrow) || GCC_AT_LEAST_VERSION(3, 3)) && !defined(WITH_CXX_OVERRIDE_FINAL)
#    define DECLARE_NEVER_THROWS	DEFINE_NEVER_THROWS __attribute__ ((nothrow))
#  else
#    define DECLARE_NEVER_THROWS 	DEFINE_NEVER_THROWS
#  endif

#endif //_COMPILER_H_
