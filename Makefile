LIBLOKATT_SOURCES := $(wildcard liblokatt/*.c)
LIBLOKATT_OBJECTS := $(patsubst liblokatt/%.c,out/.liblokatt/%.o,$(LIBLOKATT_SOURCES))
LIBLOKATT_PLISTS := $(LIBLOKATT_OBJECTS:.o=.plist)

TEST_SOURCES := $(wildcard test/*.c)
TEST_OBJECTS := $(patsubst test/%.c,out/.test/%.o,$(TEST_SOURCES))
TEST_PLISTS := $(TEST_OBJECTS:.o=.plist)

LOKATT_SOURCES := $(wildcard lokatt/*.c)
LOKATT_OBJECTS := $(patsubst lokatt/%.c,out/.lokatt/%.o,$(LOKATT_SOURCES))
LOKATT_PLISTS := $(LOKATT_OBJECTS:.o=.plist)

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

VALGRIND := valgrind
VALGRIND_FLAGS :=
VALGRIND_FLAGS += -q --trace-children=yes --error-exitcode=126
VALGRIND_FLAGS += --leak-check=full --show-reachable=yes

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
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@ $@: Makefile+" | sed 's+\.d+.o+' | sed 's+\.d+.plist+' >$@

out/.test/%.d: test/%.c | out/.test
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@ $@: Makefile+" | sed 's+\.d+.o+' | sed 's+\.d+.plist+' >$@

out/.lokatt/%.d: lokatt/%.c | out/.lokatt
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@ $@: Makefile+" | sed 's+\.d+.o+' | sed 's+\.d+.plist+' >$@

out/.liblokatt/%.o: liblokatt/%.c | out/.liblokatt
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

out/.test/%.o: test/%.c | out/.test
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

out/.lokatt/%.o: lokatt/%.c | out/.lokatt
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

out/.liblokatt/%.plist: liblokatt/%.c | out/.liblokatt
	$(QUIET_CC)$(CC) $(CFLAGS) --analyze -Xanalyzer -analyzer-opt-analyze-headers -o $@ $<

out/.test/%.plist: test/%.c | out/.test
	$(QUIET_CC)$(CC) $(CFLAGS) --analyze -Xanalyzer -analyzer-opt-analyze-headers -o $@ $<

out/.lokatt/%.plist: lokatt/%.c | out/.lokatt
	$(QUIET_CC)$(CC) $(CFLAGS) --analyze -Xanalyzer -analyzer-opt-analyze-headers -o $@ $<

all: $(TEST_BINARY) $(LOKATT_BINARY)

static-analysis: $(LIBLOKATT_PLISTS) $(TEST_PLISTS) $(LOKATT_PLISTS)

.PHONY: test
test: $(TEST_BINARY)
	@$(TEST_BINARY) $(T)

test-valgrind: $(TEST_BINARY)
	@$(VALGRIND) $(VALGRIND_FLAGS) $(TEST_BINARY) $(T)

run: $(LOKATT_BINARY)
	@$(LOKATT_BINARY)

run-valgrind: $(LOKATT_BINARY)
	@$(VALGRIND) $(VALGRIND_FLAGS) --suppressions=.lokatt.supp $(LOKATT_BINARY)

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
