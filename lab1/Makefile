CC := gcc

SRCS := $(shell find ./* -type f | grep ".c" | grep -v "assembler.c")
$(warning SRCS is $(SRCS))

# PATtern SUBSTitute
# $(patsubst pattern,replacement,text)
OBJS := $(patsubst %.c, %.o, $(filter %.c, $(SRCS)))
$(warning OBJS is $(OBJS))

# 编译选项
CFLAGS := -g -O2 -Wall -Werror -Wno-unused -ldl -std=c11

# 搜索路径
INCLUDE := -I. #-I./include

MAIN_SRC := assembler.c
MAIN_OBJ := $(MAIN_SRC:%.c=%.o)
$(warning MAIN_OBJ is $(MAIN_OBJ))
MAIN_EXE := assembler

compile: $(MAIN_EXE)

$(MAIN_EXE): $(OBJS) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

run:
	./$(MAIN_EXE) test/test1/test.asm output.obj

run2:
	./$(MAIN_EXE) test/test2/test.asm output.obj

clean:
	rm -rf ${OBJS} $(MAIN_EXE) $(MAIN_OBJ) output.obj

