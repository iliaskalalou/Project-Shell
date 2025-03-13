SUBDIRS = builtin lexer parser ast eval

bin_PROGRAMS = 42sh

42sh_SOURCES = 42sh.c

42sh_CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla

42sh_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/lexer -I$(top_srcdir)/src/ast -I$(top_srcdir)/src/builtin -I$(top_srcdir)/src/parser

42sh_LDADD = $(top_builddir)/src/eval/libeval.a \
	$(top_builddir)/src/builtin/libbuiltin.a \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/lexer/liblexer.a \
	$(top_builddir)/src/ast/libast.a
