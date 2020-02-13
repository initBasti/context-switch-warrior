SUFFIXES += .d
CLEANUP = rm -f
MKDIR = mkdir -p
TARGET_EXTENSION=out

.PHONY: clean
.PHONY: test


PATHU = unity/src/
PATHS = source/
PATHT = test/
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHR = build/results/
PATHI = source/include/
PATHBIN = bin/
PATHM = doc/man/

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

BIN_NAME := csw
MAN_NAME := csw.1

SRCT = $(wildcard $(PATHT)*.c)

SOURCES = $(wildcard $(PATHS)*.c)
OBJECTS = $(SOURCES:$(PATHS)%.c=$(PATHO)%.o)
DEPS = $(SOURCES:$(PATHS)%.c=$(PATHD)%.d)
TDEPS = $(SRCT:$(PATHT)%.c=$(PATHD)%.d)
DEPS += $(TDEPS)

COMPILE=gcc -c -g -Wall -pedantic -Wextra -std=c99 -fPIC
LINK=gcc
DEPEND=gcc -MM -MT $(@:$(PATHD)%.d=$(PATHO)%.o) >$@
INCLUDES = -I$(PATHS) -I$(PATHU) -I$(PATHI) -I$(PATHT)

RESULTS = $(patsubst $(PATHT)test_%.c,$(PATHR)test_%.txt,$(SRCT) )

#PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	#@echo "-----------------------\nPASSED:\n-----------------------"
	#@echo "$(PASSED)"
	@echo "\nDONE"

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

all: $(PATHBIN)$(BIN_NAME)
	@echo "Making symlink: $(BIN_NAME) -> $<"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

test: $(PATHB)test_config.out $(PATHB)test_substring.out $(PATHB)test_exclude.out $(PATHB)test_switch.out $(PATHB)test_cronjob.out $(PATHB)test_helper.out $(PATHB)test_delay.out $(PATHB)test_args.out

$(PATHBIN)$(BIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	$(LINK) $(OBJECTS) -o $@

$(PATHB)test_config.out: $(PATHO)test_config.o $(PATHO)config.o $(PATHU)unity.o $(PATHO)helper.o $(PATHO)substring.o $(PATHO)exclude.o $(PATHO)delay.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_exclude.out: $(PATHO)test_exclude.o $(PATHO)exclude.o $(PATHU)unity.o $(PATHO)helper.o $(PATHO)substring.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_substring.out: $(PATHO)test_substring.o $(PATHO)substring.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_switch.out: $(PATHO)test_switch.o $(PATHO)switch.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_cronjob.out: $(PATHO)test_cronjob.o $(PATHO)cronjob.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_delay.out: $(PATHO)test_delay.o $(PATHO)delay.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_args.out: $(PATHO)test_args.o $(PATHO)args.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHB)test_helper.out: $(PATHO)test_helper.o $(PATHO)helper.o $(PATHU)unity.o
	@echo "Linking: $@"
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHO)%.o:: $(PATHT)%.c
	@echo "Compiling: $< -> $@"
	$(COMPILE) $(INCLUDES) $< -o $@

$(PATHO)%.o:: $(PATHS)%.c
	@echo "Compiling: $< -> $@"
	$(COMPILE) $(INCLUDES) $< -o $@

$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	@echo "Compiling: $< -> $@"
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHD)%.d:: $(PATHT)%.c | $(PATHD)
	$(DEPEND) -o $@ $<

$(PATHD)%.d:: $(PATHS)%.c | $(PATHD)
	$(DEPEND) -o $@ $<


$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHD):
	$(MKDIR) $(PATHD)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

install: all docs
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(PATHBIN)$(BIN_NAME) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(BIN_NAME)
	#mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	#sed "s/VERSION/$(VERSION)/g" < $(PATHM)$(MAN_NAME) > $(DESTDIR)$(MANPREFIX)/man1/$(MAN_NAME)
	#chmod 644 $(DESTDIR)$(MANPREFIX)/man1/$(MAN_NAME)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN_NAME)#\
		#$(DESTDIR)$(MANPREFIX)/man1/$(MAN_NAME)\

clean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt
	$(CLEANUP) $(PATHD)*.d

docs:
	@doxygen $(PATHS)doxygen-config

.PRECIOUS: $(PATHB)test_%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt

-include $(DEPS)
