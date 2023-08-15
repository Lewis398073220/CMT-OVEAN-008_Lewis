/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/micro/kernels/fully_connected.h"

#include "arm_nnfunctions.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/reference/fully_connected.h"
#include "tensorflow/lite/kernels/internal/reference/integer_ops/fully_connected.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "tensorflow/lite/micro/micro_location.h"

#include "stdio.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include <algorithm>

namespace tflite {
	namespace {

		struct OpData {
			OpDataFullyConnected reference_op_data;

			// Index to buffer for optimizations if applicable.
			int buffer_idx;
		};

		// TODO(b/169801227): This global struct is needed for the linker to drop unused
		// code (for example, by using Register_FULLY_CONNECTED_INT8 instead of
		// Register_FULLY_CONNECTED).
		TfLiteRegistration fully_connected_registration;

		TFLITE_FLASH_TEXT_LOC
		void* Init(TfLiteContext* context, const char* buffer, size_t length) {
			TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
			return context->AllocatePersistentBuffer(context, sizeof(OpData));
		}

		TFLITE_FLASH_TEXT_LOC
		TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
			TFLITE_DCHECK(node->user_data != nullptr);
			TFLITE_DCHECK(node->builtin_data != nullptr);

			OpData* data = static_cast<OpData*>(node->user_data);
			const auto params =
				static_cast<const TfLiteFullyConnectedParams*>(node->builtin_data);

			const TfLiteTensor* input =
				GetInput(context, node, kFullyConnectedInputTensor);
			TF_LITE_ENSURE(context, input != nullptr);
			const TfLiteTensor* filter =
				GetInput(context, node, kFullyConnectedWeightsTensor);
			TF_LITE_ENSURE(context, filter != nullptr);
			const TfLiteTensor* bias =
				GetOptionalInputTensor(context, node, kFullyConnectedBiasTensor);
			TfLiteTensor* output = GetOutput(context, node, kFullyConnectedOutputTensor);
			TF_LITE_ENSURE(context, output != nullptr);
			// printf("input->type=%d, filter->type=%d, output->type=%d\n", 
			//         input->type, filter->type, output->type);
			// TF_LITE_ENSURE_TYPES_EQ(context, input->type, output->type);
			// TF_LITE_ENSURE_MSG(context, input->type == filter->type,
			//                    "Hybrid models are not supported on TFLite Micro.");

			// Set buffer index to a reset value
			data->buffer_idx = -1;
			TF_LITE_ENSURE_STATUS(CalculateOpDataFullyConnected(
				context, params->activation, input->type, input, filter, bias, output,
				&(data->reference_op_data)));

			if (input->type == kTfLiteInt8) {
				RuntimeShape filter_shape = GetTensorShape(filter);
				RuntimeShape output_shape = GetTensorShape(output);

				TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 2);
				const int filter_dim_count = filter_shape.DimensionsCount();
				cmsis_nn_dims filter_dims;
				filter_dims.n = filter_shape.Dims(filter_dim_count - 1);
				filter_dims.h = 1;
				filter_dims.w = 1;
				filter_dims.c = output_shape.Dims(1);

				const int32_t buf_size =
					arm_fully_connected_s8_get_buffer_size(&filter_dims);

				if (buf_size > 0) {
					TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
						context, buf_size, &data->buffer_idx));
				}
				else {
					data->buffer_idx = -1;
				}
			}
			return kTfLiteOk;
		}

		TfLiteStatus EvalQuantizedInt8(TfLiteContext* context, TfLiteNode* node,
			const OpData& data,
			const TfLiteEvalTensor* input,
			const TfLiteEvalTensor* filter,
			const TfLiteEvalTensor* bias,
			TfLiteEvalTensor* output) {
			const RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
			TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 2);
			// const int batches = output_shape.Dims(0);
			// const int output_depth = output_shape.Dims(1);
			const RuntimeShape filter_shape = tflite::micro::GetTensorShape(filter);
			// const int filter_dim_count = filter_shape.DimensionsCount();
			// const int accum_depth = filter_shape.Dims(filter_dim_count - 1);
			const RuntimeShape input_shape = tflite::micro::GetTensorShape(input);

			// cmsis_nn_fc_params fc_params;
			// fc_params.input_offset = -data.reference_op_data.input_zero_point;
			// fc_params.output_offset = data.reference_op_data.output_zero_point;
			// fc_params.filter_offset = -data.reference_op_data.filter_zero_point;
			// fc_params.activation.min = data.reference_op_data.output_activation_min;
			// fc_params.activation.max = data.reference_op_data.output_activation_max;

			// cmsis_nn_per_tensor_quant_params quant_params;
			// quant_params.multiplier = data.reference_op_data.output_multiplier;
			// quant_params.shift = data.reference_op_data.output_shift;

			// cmsis_nn_dims input_dims;
			// input_dims.n = batches;
			// input_dims.h = 1;
			// input_dims.w = 1;
			// input_dims.c = accum_depth;

			// cmsis_nn_dims filter_dims;
			// filter_dims.n = accum_depth;
			// filter_dims.h = 1;
			// filter_dims.w = 1;
			// filter_dims.c = output_depth;

			// cmsis_nn_dims bias_dims;
			// bias_dims.n = 1;
			// bias_dims.h = 1;
			// bias_dims.w = 1;
			// bias_dims.c = output_depth;

			// cmsis_nn_dims output_dims;
			// output_dims.n = batches;
			// output_dims.h = 1;
			// output_dims.w = 1;
			// output_dims.c = output_depth;

			// cmsis_nn_context ctx;
			// ctx.buf = nullptr;
			// ctx.size = 0;

			// if (data.buffer_idx > -1) {
			//   ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
			// }

			// const int32_t* bias_data =
			//     nullptr != bias ? tflite::micro::GetTensorData<int32_t>(bias) : nullptr;
			// printf("Enter arm_fully_connected_s8_get_buffer_size\n");
			// TF_LITE_ENSURE_EQ(
			//     context,
			//     arm_fully_connected_s8(
			//         &ctx, &fc_params, &quant_params, &input_dims,
			//         tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
			//         tflite::micro::GetTensorData<int8_t>(filter), &bias_dims, bias_data,
			//         &output_dims, tflite::micro::GetTensorData<int8_t>(output)),
			//     ARM_MATH_SUCCESS);

			// int8_t* output_data = tflite::micro::GetTensorData<int8_t>(output);

			return kTfLiteOk;
		}

		static void FloatQuantizedInt8(const float* input_data,
			const int input_data_size,
			int8_t* output_data,
			float* scale) {

			static constexpr int32_t min_val = std::numeric_limits<int8_t>::min();
			static constexpr int32_t max_val = std::numeric_limits<int8_t>::max();

			float abs_max_val = 0;

			for (int i = 0; i < input_data_size; i++) {
				if (std::abs(input_data[i]) > abs_max_val) {
					abs_max_val = std::abs(input_data[i]);
				}
			}

			float _scale = abs_max_val / 127;
			if (_scale == 0) {
				*scale = _scale;
				return;
			}
			for (int i = 0; i < input_data_size; i++) {

				const float val = input_data[i];
				int32_t unclamped = static_cast<int32_t>(TfLiteRound(val / _scale));
				int32_t clamped = std::min(std::max(unclamped, min_val), max_val);
				output_data[i] = static_cast<int8_t>(clamped);
			}
			*scale = _scale;
		}

		TfLiteStatus EvalFloatQuantizedInt8(TfLiteContext* context, TfLiteNode* node,
			const OpData& data,
			const TfLiteEvalTensor* input,
			const TfLiteEvalTensor* filter,
			const TfLiteEvalTensor* bias,
			TfLiteEvalTensor* output,
			float filter_scale) {

			const RuntimeShape output_shape = tflite::micro::GetTensorShape(output); // float
			TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 2);
			const int batches = output_shape.Dims(0);
			const int output_depth = output_shape.Dims(1);
			const RuntimeShape filter_shape = tflite::micro::GetTensorShape(filter);
			const int filter_dim_count = filter_shape.DimensionsCount();
			const int accum_depth = filter_shape.Dims(filter_dim_count - 1);
			const RuntimeShape input_shape = tflite::micro::GetTensorShape(input);

			const float* input_tensor = tflite::micro::GetTensorData<float>(input);

#if defined(_MSC_VER)
#define INPUT_DATA_SIZE 1024
#define OUTPUT_DATA_SIZE 1024
#else
#define INPUT_DATA_SIZE input_data_size
#define OUTPUT_DATA_SIZE output_data_size
#endif

			const int input_data_size = batches * accum_depth;
			int8_t input_data[INPUT_DATA_SIZE];
			const int output_data_size = batches * output_depth;
			int32_t output_data[OUTPUT_DATA_SIZE];
			// int8_t bias_data_zero[OUTPUT_DATA_SIZE]={0};
			int16_t vec_buffer[INPUT_DATA_SIZE];

			float scale = 0;

			FloatQuantizedInt8(input_tensor,
				input_data_size,
				input_data,
				&scale);

			const int8_t* filter_tensor = tflite::micro::GetTensorData<int8_t>(filter);

			// arm_fully_connected_q7(input_data,
			//                        filter_tensor,
			//                        input_data_size,
			//                        output_depth,
			//                        0, 8,
			//                        bias_data_zero,
			//                        output_data,
			//                        vec_buffer);

			// const float* bias_data =
			//     nullptr != bias ? tflite::micro::GetTensorData<float>(bias) : nullptr;

			// float shift = scale * filter_scale * (1 << 8);
			// if(bias_data)
			//   for(int i = 0; i < output_data_size; i++)
			//   {
			//     output->data.f[i] = output_data[i] * shift + bias->data.f[i];
			//   }
			// else{
			//   for(int i = 0; i < output_data_size; i++)
			//   {
			//     output->data.f[i] = output_data[i] * shift;
			//   }
			// }
			arm_fully_connected_q7_bes_q31(input_data,
				filter_tensor,
				input_data_size,
				output_depth,
				output_data,
				vec_buffer);

			const float* bias_data =
				nullptr != bias ? tflite::micro::GetTensorData<float>(bias) : nullptr;

			float shift = scale * filter_scale;
			if (bias_data)
				for (int i = 0; i < output_data_size; i++)
				{
					output->data.f[i] = output_data[i] * shift + bias->data.f[i];
				}
			else {
				for (int i = 0; i < output_data_size; i++)
				{
					output->data.f[i] = output_data[i] * shift;
				}
			}
			return kTfLiteOk;
		}

		TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
			TFLITE_DCHECK(node->builtin_data != nullptr);

			const TfLiteTensor* filter_tensor =
				tflite::GetInput(context, node, kFullyConnectedWeightsTensor);

			const TfLiteEvalTensor* input =
				tflite::micro::GetEvalInput(context, node, kFullyConnectedInputTensor);
			const TfLiteEvalTensor* filter =
				tflite::micro::GetEvalInput(context, node, kFullyConnectedWeightsTensor);
			const TfLiteEvalTensor* bias =
				tflite::micro::GetEvalInput(context, node, kFullyConnectedBiasTensor);
			TfLiteEvalTensor* output =
				tflite::micro::GetEvalOutput(context, node, kFullyConnectedOutputTensor);

			TFLITE_DCHECK(node->user_data != nullptr);
			const OpData& data = *(static_cast<const OpData*>(node->user_data));

				// Checks in Prepare ensure input, output and filter types are all the same.
				switch (input->type) {
				case kTfLiteFloat32: {
					if (filter->type == kTfLiteInt8) {
						return EvalFloatQuantizedInt8(context, node, data, input, filter, bias,
							output, filter_tensor->params.scale);
					}
					else {
#ifndef BES_OPTIMIZE
						return EvalQuantizedInt8(context, node, data, input, filter, bias,
							output);
#else
						TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
							TfLiteTypeGetName(filter->type), filter->type);
						return kTfLiteError;
#endif
					}
				}
#ifndef BES_OPTIMIZE
				case kTfLiteInt8: {
					return EvalQuantizedInt8(context, node, data, input, filter, bias,
						output);
				}
#endif
				default: {
					TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
						TfLiteTypeGetName(input->type), input->type);
					return kTfLiteError;
				}
			}

			return kTfLiteOk;
		}

		// Note that the current function names are not ideal at all (this EvalInt8
		// function internally calls EvalQuantizedInt8, and there is similar name
		// aliasing in the Eval function too). We will be attempting to have a more
		// descriptive naming convention but holding off on that for now, since the
		// renaming might be coupled with reducing code duplication and some additional
		// refactoring.
		TfLiteStatus EvalInt8(TfLiteContext* context, TfLiteNode* node) {
			const TfLiteEvalTensor* input =
				tflite::micro::GetEvalInput(context, node, kFullyConnectedInputTensor);
			const TfLiteEvalTensor* filter =
				tflite::micro::GetEvalInput(context, node, kFullyConnectedWeightsTensor);
			const TfLiteEvalTensor* bias =
				tflite::micro::GetEvalInput(context, node, kFullyConnectedBiasTensor);
			TfLiteEvalTensor* output =
				tflite::micro::GetEvalOutput(context, node, kFullyConnectedOutputTensor);

			TFLITE_DCHECK(node->user_data != nullptr);
			const OpData& data = *(static_cast<const OpData*>(node->user_data));

			// Checks in Prepare ensure input, output and filter types are all the same.
			if (input->type != kTfLiteInt8) {
				TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
					TfLiteTypeGetName(input->type), input->type);
				return kTfLiteError;
			}

			return EvalQuantizedInt8(context, node, data, input, filter, bias, output);
		}

	}  // namespace

	TFLITE_FLASH_TEXT_LOC
	TfLiteRegistration Register_FULLY_CONNECTED() {
		fully_connected_registration.init = Init;
		fully_connected_registration.free = nullptr;
		fully_connected_registration.prepare = Prepare;
		fully_connected_registration.invoke = Eval;
		fully_connected_registration.profiling_string = nullptr;
		fully_connected_registration.builtin_code = 0;
		fully_connected_registration.custom_name = nullptr;
		fully_connected_registration.version = 0;
		return fully_connected_registration;
	}

	TFLITE_FLASH_TEXT_LOC
	TfLiteRegistration Register_FULLY_CONNECTED_INT8() {
		fully_connected_registration.init = Init;
		fully_connected_registration.free = nullptr;
		fully_connected_registration.prepare = Prepare;
		fully_connected_registration.invoke = EvalInt8;
		fully_connected_registration.profiling_string = nullptr;
		fully_connected_registration.builtin_code = 0;
		fully_connected_registration.custom_name = nullptr;
		fully_connected_registration.version = 0;
		return fully_connected_registration;
	}

}  // namespace tflite
