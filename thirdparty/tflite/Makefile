cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cc))

MICROLITE_CMSIS_KERNEL_SRCS := \
tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc

MICROLITE_CC_BASE_SRCS := \
$(wildcard thirdparty/tflite/tensorflow/lite/micro/*.cc) \
$(wildcard thirdparty/tflite/tensorflow/lite/micro/memory_planner/*.cc) \

obj_c += \
    tensorflow/lite/c/common.c

obj_cpp += \
    tensorflow/lite/core/api/error_reporter.cc \
    tensorflow/lite/core/api/flatbuffer_conversions.cc \
    tensorflow/lite/core/api/op_resolver.cc \
    tensorflow/lite/micro/all_ops_resolver.cc \
    tensorflow/lite/micro/debug_log.cc \
    tensorflow/lite/micro/flatbuffer_utils.cc \
    tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc \
    tensorflow/lite/micro/memory_helpers.cc \
    tensorflow/lite/micro/micro_allocator.cc \
    tensorflow/lite/micro/micro_error_reporter.cc \
    tensorflow/lite/micro/micro_graph.cc \
    tensorflow/lite/micro/micro_interpreter.cc \
    tensorflow/lite/micro/micro_resource_variable.cc \
    tensorflow/lite/micro/micro_string.cc \
    tensorflow/lite/micro/micro_utils.cc \
    tensorflow/lite/micro/simple_memory_allocator.cc \
    tensorflow/lite/micro/system_setup.cc \
    tensorflow/lite/schema/schema_utils.cc \
    tensorflow/lite/kernels/kernel_util.cc \
    tensorflow/lite/kernels/internal/quantization_util.cc \
    tensorflow/lite/micro/kernels/ \
    $(MICROLITE_CC_BASE_SRCS) \
    $(MICROLITE_CMSIS_KERNEL_SRCS)

tflite_micro_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cc=.o)

LIB_BIN_DIR := lib/thirdparty/tflite/

TFLITE_MICRO_LIB_NAME := tflite_micro

${TFLITE_MICRO_LIB_NAME}-y := ${tflite_micro_obj}

obj-y += ${TFLITE_MICRO_LIB_NAME}.a

subdir-ccflags-y += \
    -Iinclude/cmsis_dsp/ \
    -Iinclude/cmsis_nn/ \
    -Iinclude/beco/ \
    -Ithirdparty/tflite/ \
    -Ithirdparty/tflite/tensorflow/lite/micro/tools/make/downloads/flatbuffers/include/ \
    -Ithirdparty/tflite/tensorflow/lite/micro/tools/make/downloads/gemmlowp/ \
    -Ithirdparty/tflite/tensorflow/lite/micro/tools/make/downloads/ruy/ \
    -Iservices/multimedia/speech/inc/ \
    -Iutils/heap/

subdir-ccflags-y += -DTF_LITE_STATIC_MEMORY -DCMSIS_NN -DTF_LITE_STRIP_ERROR_STRINGS -DBES_PLATFORM -DBES_OPTIMIZE -Os
