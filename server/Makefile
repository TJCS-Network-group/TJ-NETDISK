#src = Main.cpp
#obj = Main.o
#target = Main
src = Main-thread.cpp
obj = Main-thread.o
target = Main-thread
select = -std=c++11 -Wall -g -O2 -lmysqlclient -lpthread
common_src_path = $(wildcard ./common/*.cpp)
common_obj_path=$(patsubst %.cpp, %.o, $(common_src_path))
source_src_path = $(wildcard ./source/*.cpp)
source_obj_path=$(patsubst %.cpp, %.o, $(source_src_path))

CC = g++
.SECONDARY: $(obj) $(common_obj_path) $(source_obj_path)

all: $(target)

$(target): $(common_obj_path) $(source_obj_path) $(obj)
	$(CC) $(select) $(obj) $(common_obj_path) $(source_obj_path) -o $@

%.o: %.cpp
	$(CC) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(obj) $(target) $(common_obj_path) $(source_obj_path)
