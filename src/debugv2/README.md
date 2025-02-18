# Next generation debugger with OpenMSX

Execute it with  `make run`  if you have OpenMSX in your PATH.

## Supported placeholders

| Placeholders                               |  Value and alternatives  |  status  |
| ------------------------------------------ | ------------------------ | -------- |
| The "%" character                          |                     "%%" |          |
| Character                                  |                     "%c" |          |
| Nul-terminated string                      |                     "%s" |          |
| Nul-terminated string uppercase            |                     "%S" |          |
| Left-padded nul-terminated string          |              "%[width]s" |          |
| Right-padded nul-terminated string         |             "%-[width]s" |          |
| Truncate string at [width] size            |             "%.[width]s" |          |
| MSX-BASIC float (exponent + BCD mantissa)  |                     "%f" |          |
| SDCC float                                 |                    "%hf" |          |
| 16-bit fixed point                         |                   "%hhf" | missing  |
| 8-bit unsigned integer                     |                   "%hhu" |          |
| 8-bit signed integer                       |                   "%hhi" |          |
| 8-bit hexadecimal (a-f)                    |                   "%hhx" |          |
| 8-bit hexadecimal (A-F)                    |                   "%hhX" |          |
| 8-bit binary                               |                   "%hhb" |          |
| 8-bit octal                                |                   "%hho" |          |
| 16-bit unsigned integer                    |               "%u" "%hu" |          |
| 16-bit signed integer                      |    "%i" "%d" "%hi" "%hd" |          |
| Left-padded integer                        |             "%0[count]u" |          |
| 16-bit hexadecimal (a-f)                   |               "%x" "%hx" |          |
| 16-bit hexadecimal (A-F)                   |               "%X" "%hX" |          |
| 16-bit binary                              |               "%b" "%hb" |          |
| 16-bit octal                               |               "%o" "%ho" |          |
| 32-bit unsigned integer                    |                    "%lu" |          |
| 32-bit signed integer                      |              "%li" "%ld" |          |
| 32-bit hexadecimal (a-f)                   |                    "%lx" |          |
| 32-bit hexadecimal (A-F)                   |                    "%lX" |          |
| 32-bit binary                              |                    "%lb" |          |
| 32-bit octal                               |                    "%lo" |          |
| Void pointer (platform specific)           |                     "%p" | missing  |
| Debug_mode output (for compatibility)      |                     "%?" |          |

> [!NOTE]
> Be warned that if there is a mismatch between args and placeholders in the format string, Tcl may print garbage to stdout if it runs out of arguments, but it will not cause any crash.
