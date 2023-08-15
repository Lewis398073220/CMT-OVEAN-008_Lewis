cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cc))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cc=.o)
