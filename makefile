cc = clang-3.8
cflags = -std=c11

dir_src = ./script
dir_target = ./target

src = $(wildcard ${dir_src}/*.c)
files = $(notdir ${src})
objects = $(patsubst %.c, ${dir_target}/%.o, $(notdir ${src}))

all : $(dir_target)/main

$(dir_target)/main : $(objects) 
	$(cc) -o $@ $^

$(dir_target)/%.o: $(dir_src)/%.c
	$(cc) $(cflags) -c $^ -o $@

run : $(dir_target)/main
	$(dir_target)/main

.PHONY:clean
clean:
	rm -f $(dir_target)/*