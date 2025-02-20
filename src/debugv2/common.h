#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdarg.h>

#define NULL ((char *)0)

/**
 * Compatibility macro for unused function parameter warnings or errors.
 * If you need it, just put -DUNUSED_MACRO on CFLAGS.
 */
#ifndef DISABLE_UNUSED_MACRO
#define UNUSED(x) ((void)x)
#else
#define UNUSED(x)
#endif /* DISABLE_UNUSED_MACRO */

#ifdef USE_DEBUG_MODE

/**
 * [Optional] Change emulator's debug mode. This allows programs to output text
 * and values to the emulator for debugging purposes. The format of the output
 * value is defined below. This is the old behavior which is present only for
 * compatibility with old code, the new debug_printf function is much more
 * flexible and its use is recommended.
 *
 * | Name                  | Value |
 * | ---                   | ----- |
 * | DEBUG_HEX             |   0   |
 * | DEBUG_INT             |   1   |
 * | DEBUG_BIN             |   2   |
 * | DEBUG_CHAR            |   3   |
 *
 * See [debug()](#debug) for more details about the output.
 *
 * 📌 **Implementation details**
 *
 * * You can undefine `USE_DEBUG_MODE` macro to transform all the debug macros
 * into empty macros, so no additional code is generated on the release version.
 */
void debug_mode(uint8_t mode);


/**
 * [Optional] Send text message to emulator for debugging (printing).  This is
 * the old behavior which is present only for compatibility with old code, the
 * new debug_printf function is much more flexible and its use is recommended.
 *
 * 📌 **Implementation details**
 *
 * * You can undefine `USE_DEBUG_MODE` macro to transform all the debug macros
 * into empty macros, so no additional code is generated on the release version.
 */
#define debug_msg(msg)  debug_printf("%s\n", msg)


/**
 * [Optional] Pause emulator at debug_break function.
 *
 * 📌 **Implementation details**
 *
 * * You can undefine `USE_DEBUG_MODE` macro to transform all the debug macros
 * into empty macros, so no additional code is generated on the release version.
 */
void debug_break();


/**
 * [Optional] Send message to emulator for debugging (printing) along with a
 * numeric value. See [debug_mode()](#debug_mode) for output options. This is
 * the old behavior which is present only for compatibility with old code, the
 * new debug_printf function is much more flexible and its use is recommended.
 *
 * 📌 **Implementation details**
 *
 * * You can undefine `USE_DEBUG_MODE` macro to transform all the debug macros
 * into empty macros, so no additional code is generated on the release version.
 */
#define debug(msg, num)  debug_printf("%s%?\n", msg, num)


#ifndef DISABLE_DEBUG_PRINTF
/**
 * A printf structure in memory. This is a new way of printing strings that is more
 * complete like `printf`, but using Tcl for most of the formatting and printing.
 *
 * 📌 **Implementation details**
 *
 * Be warned that if there is a mismatch between args and placeholders in the format
 * string, Tcl may print garbage to stdout if it runs out of arguments.
 *
 * | Allowed placeholder types                  |  Value and alternatives  |  status  |
 * | ------------------------------------------ | ------------------------ | -------- |
 * | The "%" character                          |                     "%%" |          |
 * | Character                                  |                     "%c" |          |
 * | Nul-terminated string                      |                     "%s" |          |
 * | Nul-terminated string uppercase            |                     "%S" |          |
 * | Left-padded nul-terminated string          |              "%[width]s" |          |
 * | Right-padded nul-terminated string         |             "%-[width]s" |          |
 * | Truncate string at [width] size            |             "%.[width]s" |          |
 * | MSX-BASIC float (exponent + BCD mantissa)  |                     "%f" |          |
 * | SDCC float                                 |                    "%hf" |          |
 * | 16-bit fixed point                         |                   "%hhf" | missing  |
 * | 8-bit unsigned integer                     |                   "%hhu" |          |
 * | 8-bit signed integer                       |                   "%hhi" |          |
 * | 8-bit hexadecimal (a-f)                    |                   "%hhx" |          |
 * | 8-bit hexadecimal (A-F)                    |                   "%hhX" |          |
 * | 8-bit binary                               |                   "%hhb" |          |
 * | 8-bit octal                                |                   "%hho" |          |
 * | 16-bit unsigned integer                    |               "%u" "%hu" |          |
 * | 16-bit signed integer                      |    "%i" "%d" "%hi" "%hd" |          |
 * | Left-padded integer                        |             "%0[count]u" |          |
 * | 16-bit hexadecimal (a-f)                   |               "%x" "%hx" |          |
 * | 16-bit hexadecimal (A-F)                   |               "%X" "%hX" |          |
 * | 16-bit binary                              |               "%b" "%hb" |          |
 * | 16-bit octal                               |               "%o" "%ho" |          |
 * | 32-bit unsigned integer                    |                    "%lu" |          |
 * | 32-bit signed integer                      |              "%li" "%ld" |          |
 * | 32-bit hexadecimal (a-f)                   |                    "%lx" |          |
 * | 32-bit hexadecimal (A-F)                   |                    "%lX" |          |
 * | 32-bit binary                              |                    "%lb" |          |
 * | 32-bit octal                               |                    "%lo" |          |
 * | Void pointer (platform specific)           |                     "%p" | missing  |
 * | Debug_mode output (for compatibility)      |                     "%?" |          |
 */
void debug_printf(char *fmt, ...);

#endif // DISABLE_DEBUG_PRINTF

/**
 * Breaks execution if not `ok` and displays optional message after the `ok` parameter.
 *
 * 📌 **Implementation details**
 *
 * * You can undefine `USE_DEBUG_MODE` macro to transform all the debug macros
 * into empty macros, so no additional code is generated on the release version.
 *
 * * Regular C `assert`-like macros are not appropriate for graphics mode, so
 * we use these debug functions to implement our own.
 *
 * * This macro requires that the platform's C compiler supports stringification
 * of arguments in macros. Otherwise, the `NO_STRINGIFICATION` macro should be
 * defined first.
 *
 * * This macro can use variable argument support (variadic macros) to include a
 * message in case of assertion failure, but you can set NO_VARIADIC first to
 * disable this feature.
 */
#ifndef NO_STRINGIFICATION
# ifndef NO_VARIADIC
#  define assert(ok, ...) \
    do { if (!(ok)) { debug_printf("Assertion `" #ok "' failed. " __VA_ARGS__ "\nPaused\n", NULL); debug_break(); } } while(0)
# else /* NO_VARIADIC */
#  define assert(ok) \
    do { if (!(ok)) { debug_printf("Assertion `" #ok "' failed.\nPaused\n", NULL); debug_break(); } } while(0)
# endif /* NO_VARIADIC */
#else /* NO_STRINGIFICATION */
# ifndef NO_VARIADIC
#  define assert(ok, ...) \
    do { if (!(ok)) { debug_printf("Assertion failed. " __VA_ARGS__ "\nPaused\n", NULL); debug_break(); } } while(0)
# else /* NO_VARIADIC */
#  define assert(ok) \
    do { if (!(ok)) { debug_printf("Assertion failed.\nPaused\n", NULL); debug_break(); } } while(0)
# endif /* NO_VARIADIC */
#endif /* NO_STRINGIFICATION */

#else

/* empty macros */
#define debug(x, y)
#define debug_msg(x)
#define debug_break()

#ifdef DISABLE_DEBUG_PRINTF
# define debug_printf(fmt, args)
#endif /* DISABLE_DEBUG_PRINTF */

# ifndef NO_VARIADIC
#  define assert(x, ...)
# else
#  define assert(x)
# endif /* NO_VARIADIC */

#endif /* USE_DEBUG_MODE */

#endif /* COMMON_H */
