lib_LIBRARIES = libparser.a 

libparser_a_SOURCES = parser.c parser.h

libparser_a_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

libparser_a_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/lexer -I$(top_srcdir)/src/ast

noinst_LIBRAIRIES = libparser.a
