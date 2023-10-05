CC := gcc

SRCS := $(shell find ./* -type f | grep ".c" | grep -v "lc3bsim2.c")
$(warning SRCS is $(SRCS))

# PATtern SUBSTitute
# $(patsubst pattern,replacement,text)
OBJS := $(patsubst %.c, %.o, $(filter %.c, $(SRCS)))
$(warning OBJS is $(OBJS))

# 编译选项
CFLAGS := -g -O2 -Wall -Werror -Wno-unused -ldl -std=c11

# 搜索路径
INCLUDE := -I. #-I./include

MAIN_SRC := lc3bsim2.c
MAIN_OBJ := $(MAIN_SRC:%.c=%.o)
$(warning MAIN_OBJ is $(MAIN_OBJ))
MAIN_EXE := sim

compile: $(MAIN_EXE)

$(MAIN_EXE): $(OBJS) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(INCLUDE) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

run:
	./$(MAIN_EXE) test/test1/output.obj

clean:
	rm -rf ${OBJS} $(MAIN_EXE) $(MAIN_OBJ) dump*

