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

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

all: unity $(PATHBIN)$(BIN_NAME)
	@echo "Making symlink: $(BIN_NAME) -> $<"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

unity:
ifeq (,$(wildcard ./unity))
	wget https://github.com/ThrowTheSwitch/Unity/archive/master.zip -O unity.zip && unzip unity.zip && mkdir unity && cp -r Unity-master/src/ unity/ && rm -rf Unity-master/ unity.zip
endif

test: unity $(PATHBIN)test_config.out $(PATHBIN)test_substring.out $(PATHBIN)test_exclude.out $(PATHBIN)test_switch.out $(PATHBIN)test_cronjob.out $(PATHBIN)test_helper.out $(PATHBIN)test_delay.out $(PATHBIN)test_args.out

$(PATHBIN)$(BIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(OBJECTS) -o $@

$(PATHBIN)test_config.out: $(PATHO)test_config.o $(PATHO)config.o $(PATHU)unity.o $(PATHO)helper.o $(PATHO)substring.o $(PATHO)exclude.o $(PATHO)delay.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_exclude.out: $(PATHO)test_exclude.o $(PATHO)exclude.o $(PATHU)unity.o $(PATHO)helper.o $(PATHO)substring.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_substring.out: $(PATHO)test_substring.o $(PATHO)substring.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_switch.out: $(PATHO)test_switch.o $(PATHO)switch.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_cronjob.out: $(PATHO)test_cronjob.o $(PATHO)cronjob.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_delay.out: $(PATHO)test_delay.o $(PATHO)delay.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_args.out: $(PATHO)test_args.o $(PATHO)args.o $(PATHU)unity.o $(PATHO)helper.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHBIN)test_helper.out: $(PATHO)test_helper.o $(PATHO)helper.o $(PATHU)unity.o
	@echo "Linking: $@"
	@mkdir -p $(@D)
	$(LINK) $(INCLUDES) -o $@ $^

$(PATHO)%.o:: $(PATHT)%.c
	@echo "Compiling: $< -> $@"
	@mkdir -p $(@D)
	$(COMPILE) $(INCLUDES) $< -o $@

$(PATHO)%.o:: $(PATHS)%.c
	@echo "Compiling: $< -> $@"
	@mkdir -p $(@D)
	$(COMPILE) $(INCLUDES) $< -o $@

$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	@echo "Compiling: $< -> $@"
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHD)%.d:: $(PATHT)%.c | $(PATHD)
	@mkdir -p $(@D)
	$(DEPEND) -o $@ $<

$(PATHD)%.d:: $(PATHS)%.c | $(PATHD)
	@mkdir -p $(@D)
	$(DEPEND) -o $@ $<

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
