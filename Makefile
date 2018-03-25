LIBLOKATT_SOURCES := $(wildcard liblokatt/*.c)
LIBLOKATT_OBJECTS := $(patsubst liblokatt/%.c,out/.liblokatt/%.o,$(LIBLOKATT_SOURCES))

TEST_SOURCES := $(wildcard test/*.c)
TEST_OBJECTS := $(patsubst test/%.c,out/.test/%.o,$(TEST_SOURCES))

LOKATT_SOURCES := $(wildcard lokatt/*.c)
LOKATT_OBJECTS := $(patsubst lokatt/%.c,out/.lokatt/%.o,$(LOKATT_SOURCES))

DEPS := $(LIBLOKATT_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d) $(LOKATT_OBJECTS:.o=.d)

CC := clang
CFLAGS :=
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809L
CFLAGS += -Wall -Wextra
CFLAGS += -O0 -g
CFLAGS += -I include

LD := clang
LDFLAGS :=
LDFLAGS += -pthread
LDFLAGS += -lncurses

LIBLOKATT := out/.liblokatt/liblokatt.a
LOKATT_BINARY := out/lokatt
TEST_BINARY := out/test-lokatt

ifndef V
        QUIET_AR = @echo "    AR $@";
        QUIET_CC = @echo "    CC $@";
        QUIET_DEP = @echo "    DEP $@";
        QUIET_LINK = @echo "    LINK $@";
        QUIET_MKDIR = @echo "    MKDIR $@";
endif

out/.liblokatt/%.d: liblokatt/%.c | out/.liblokatt
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@: Makefile+" | sed 's+\.d+.o+' >$@

out/.test/%.d: test/%.c | out/.test
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@: Makefile+" | sed 's+\.d+.o+' >$@

out/.lokatt/%.d: lokatt/%.c | out/.lokatt
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@: Makefile+" | sed 's+\.d+.o+' >$@

out/.liblokatt/%.o: liblokatt/%.c | out/.liblokatt
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

out/.test/%.o: test/%.c | out/.test
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

out/.lokatt/%.o: lokatt/%.c | out/.lokatt
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

all: $(TEST_BINARY) $(LOKATT_BINARY)

test: $(TEST_BINARY)
	@$(TEST_BINARY) $(T)

test-valgrind:
	@valgrind -q --trace-children=yes --leak-check=full --show-reachable=yes --error-exitcode=126 $(TEST_BINARY) $(T)

run: $(LOKATT_BINARY)
	@$(LOKATT_BINARY)

$(LIBLOKATT): $(LIBLOKATT_OBJECTS) | out/.liblokatt
	$(QUIET_AR)ar rcs $@ $^

$(TEST_BINARY): $(TEST_OBJECTS) $(LIBLOKATT) | out
	$(QUIET_LINK)$(LD) $(LDFLAGS) -o $@ $^

$(LOKATT_BINARY): $(LOKATT_OBJECTS) $(LIBLOKATT) | out
	$(QUIET_LINK)$(LD) $(LDFLAGS) -o $@ $^

out:
	$(QUIET_MKDIR)mkdir -p out

out/.liblokatt:
	$(QUIET_MKDIR)mkdir -p out/.liblokatt

out/.test:
	$(QUIET_MKDIR)mkdir -p out/.test

out/.lokatt:
	$(QUIET_MKDIR)mkdir -p out/.lokatt

clean:
	$(RM) -r out

-include $(DEPS)
