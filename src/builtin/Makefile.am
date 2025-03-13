lib_LIBRARIES = libbuiltin.a 

libbuiltin_a_SOURCES = echo.c true.c false.c cd.c dot.c builtin.h

libbuiltin_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

libbuiltin_a_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/lexer -I$(top_srcdir)/src/ast -I$(top_srcdir)/src/parser -I$(top_srcdir)/src/eval

noinst_LIBRAIRIES = libbuiltin.a
