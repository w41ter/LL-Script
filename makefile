cc = clang-3.8
cxx = clang++-3.8
cflags = -std=c11
cxxflags = -std=c++11

dir_src = ./script
dir_target = ./target

src_c = $(wildcard ${dir_src}/*.c)
src_cxx = $(wildcard ${dir_src}/*.cpp)

files_c = $(notdir ${src_c})
files_cxx = $(notdir ${src_cxx})
objects_c = $(files_c:%.c=$(dir_target)/%.o)
objects_cxx = $(files_cxx:%.cpp=$(dir_target)/%.o)

all : $(dir_target)/main

$(dir_target)/main : $(objects_c) $(objects_cxx)
	$(cxx) -o $@ $^

$(dir_target)/%.o : $(dir_src)/%.c
	$(cc) $(cflags) -c $^ -o $@

$(dir_target)/%.o : $(dir_src)/%.cpp
	$(cxx) $(cxxflags) -c $^ -o $@

run : $(dir_target)/main
	$(dir_target)/main

.PHONY : clean
clean :
	rm -f $(dir_target)/*