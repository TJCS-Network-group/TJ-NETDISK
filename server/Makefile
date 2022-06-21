src = Main.cpp
obj = Main.o
target = Main

common_src_path = $(wildcard ./common/*.cpp)
common_obj_path=$(patsubst %.cpp, %.o, $(common_src_path))

CC = g++
.SECONDARY: $(obj) $(common_obj_path)

all: $(common_obj_path) $(target)

%:%.o
	$(CC) $< $(common_obj_path) -o $@

%.o: %.cpp
	$(CC) -c $< -o $@	

.PHONY: clean
clean:
	rm -rf $(obj) $(target) $(common_obj_path)
