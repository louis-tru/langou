/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./array.h"
#include "./string.h"
#include <string.h>
#include <math.h>

namespace noug {

	#define N_DEF_ARRAY_SPECIAL_IMPLEMENTATION_(T, A, APPEND_ZERO) \
		\
		template<> void Array<T, A>::extend(uint32_t length, uint32_t capacity) \
		{ \
			if (length > _length) {  \
				_length = length; \
				realloc_(N_MAX(_length + APPEND_ZERO, capacity)); \
				if (APPEND_ZERO) _val[_length] = 0; \
			}\
		}\
		\
		template<> std::vector<T> Array<T, A>::vector() const { \
			std::vector<T> r(_length); \
			if (_length) \
				memcpy(r.data(), _val, sizeof(T) * _length); \
			return std::move(r); \
		} \
		\
		template<> Array<T, A>& Array<T, A>::concat_(T* src, uint32_t src_length) { \
			if (src_length) {\
				_length += src_length; \
				realloc_(_length + APPEND_ZERO); \
				T* src = _val; \
				T* to = _val + _length - src_length; \
				memcpy((void*)to, src, src_length * sizeof(T)); \
				if (APPEND_ZERO) _val[_length] = 0; \
			} \
			return *this; \
		} \
		\
		template<> uint32_t Array<T, A>::write(const T* src, int to, uint32_t size) { \
			if (size) { \
				if ( to == -1 ) to = _length; \
				_length = N_MAX(to + size, _length); \
				realloc_(_length + APPEND_ZERO); \
				memcpy((void*)(_val + to), src, size * sizeof(T) ); \
				if (APPEND_ZERO) _val[_length] = 0; \
			} \
			return size; \
		} \
		\
		template<> Array<T, A>& Array<T, A>::pop(uint32_t count) { \
			uint32_t j = uint32_t(N_MAX(_length - count, 0)); \
			if (_length > j) {  \
				_length = j;  \
				realloc_(_length + APPEND_ZERO); \
				if (APPEND_ZERO) _val[_length] = 0; \
			} \
			/*return _length;*/ \
			return *this; \
		} \
		\
		template<> void Array<T, A>::clear() { \
			if (_val) { \
				if (!is_weak()) { \
					A::free(_val); /* free */ \
					_capacity = 0; \
				} \
				_length = 0; \
				_val = nullptr; \
			} \
		} \
		\
		template<> void Array<T, A>::realloc(uint32_t capacity) { \
			N_ASSERT(!is_weak(), "the weak holder cannot be changed"); \
			if (capacity < _length) { /* clear Partial data */ \
				_length = capacity;\
			} \
			realloc_(capacity + 1); \
			if (APPEND_ZERO) _val[_length] = 0; \
		} \
		\
		template<> ArrayBuffer<T, A> Array<T, A>::copy(uint32_t start, uint32_t end) const { \
			end = N_MIN(end, _length); \
			if (start < end) { \
				ArrayBuffer<T, A> arr(end - start, end - start + APPEND_ZERO); \
				memcpy((void*)arr.val(), _val + start, arr.length() * sizeof(T)); \
				if (APPEND_ZERO) (*arr)[arr.length()] = 0; \
				return arr; \
			} \
			return ArrayBuffer<T, A>();\
		} \

	#define N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(T) \
		N_DEF_ARRAY_SPECIAL_IMPLEMENTATION_(T, MemoryAllocator, 1)
	

#ifndef N_ARRAY_NO_IMPL
	template<> void Array<char, MemoryAllocator>::_Reverse(void *src, size_t size, uint32_t len) {
		if (len > 1) {
			char* _src = (char*)src;
			void* tmp = malloc(size);
			uint32_t len2 = floor(len / 2);
			uint32_t i = 0;
			while (i < len2) {
				char* src = _src + (i * size);
				char* dest = _src + ((len - i - 1) * size);
				memcpy(tmp, src, size);
				memcpy(src, dest, size);
				memcpy(dest, tmp, size);
				i++;
			}
			free(tmp);
		}
	}
	
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(char);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(unsigned char);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int16_t);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint16_t);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int32_t);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint32_t);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(int64_t);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(uint64_t);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(float);
	N_DEF_ARRAY_SPECIAL_IMPLEMENTATION(double);
#endif

}