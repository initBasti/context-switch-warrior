UNITY_DIR=unity/src
TEST_DIR=test
SRC_DIR=source
INC_DIR=src/include
OBJ_DIR=build
BIN_DIR=bin
RES_DIR=build/test_result

LINK=gcc
COMPILE=gcc -c
DEPEND=gcc -MM -MF -MG

CFLAGS=-Wall -pedantic -Wextra -g -I$(UNITY_DIR) -I$(TEST_DIR) -I.
BIN_NAME=$(BIN_DIR)/csw

SOURCES=$(wildcard $(SRC_DIR)/*.c)
TESTS=$(wildcard $(TEST_DIR)/*.c)
UNITY=$(UNITY_DIR)/unity.c

UNITY_OBJ=$(OBJ_DIR)/unity.o

TEST_BIN=$(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/%, $(TESTS))
# doesn't work TODO
RESULTS=$(patsubst $(BIN_DIR)/*, $(RES_DIR)/*.txt, $(TEST_BIN))

all: $(BIN_NAME) $(TEST_BIN) 

$(BIN_NAME): $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
	$(LINK) -o $@ $^ $(CFLAGS)

$(RES_DIR)/%.txt: $(BIN_DIR)/%
	-./$< > $@ 2>&1

# doesn't work TODO
test: $(RESULTS)
	#@echo "---------------\\nIGNORES\n----------------"
	#@echo `grep -s IGNORE $(RES_DIR)/*.txt`
	#@echo "---------------\\nFAILURES\\n----------------"
	#@echo `grep -s FAIL $(RES_DIR)/*.txt`
	#@echo "\\nDONE"

$(BIN_DIR)/test_%:$(OBJ_DIR)/test_%.o $(OBJ_DIR)/%.o $(OBJ_DIR)/helper.o $(OBJ_DIR)/substring.o $(OBJ_DIR)/parser.o $(OBJ_DIR)/switch.o $(OBJ_DIR)/delay.o $(OBJ_DIR)/cronjob.o $(UNITY_OBJ)
	$(LINK) -o $@ $^ $(CFLAGS)

$(OBJ_DIR)/test_%.o: $(TEST_DIR)/test_%.c $(OBJ_DIR)/%.o $(OBJ_DIR)/helper.o $(OBJ_DIR)/substring.o $(OBJ_DIR)/parser.o $(OBJ_DIR)/switch.o $(OBJ_DIR)/delay.o $(OBJ_DIR)/cronjob.o $(UNITY_OBJ)
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/config.o: $(SRC_DIR)/config.c $(SRC_DIR)/parser.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/parser.o: $(SRC_DIR)/parser.c $(SRC_DIR)/helper.c $(SRC_DIR)/substring.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/substring.o: $(SRC_DIR)/substring.c $(SRC_DIR)/helper.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/helper.o: $(SRC_DIR)/helper.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/switch.o: $(SRC_DIR)/switch.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/delay.o: $(SRC_DIR)/delay.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(OBJ_DIR)/cronjob.o: $(SRC_DIR)/cronjob.c
	$(COMPILE) -o $@ $< $(CFLAGS)

$(UNITY_OBJ): $(UNITY)
	$(COMPILE) -o $@ $< $(CFLAGS)

.PHONY: test
