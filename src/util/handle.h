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

#ifndef __noug__util__handle__
#define __noug__util__handle__

#include "./object.h"
#include <functional>

namespace noug {

	/**
	* @class Handle
	*/
	template<class T, class T2 = typename T::Traits> class Handle {
		//! Copy constructor is not permitted.
		N_HIDDEN_ALL_COPY(Handle);
		
		inline T* move() {
			return Traits::Retain(_data) ? _data : collapse();
		}
		
	public:
		typedef T Type;
		typedef T2 Traits;
		
		inline Handle(): _data(nullptr) {}
		inline Handle(T* data): _data(data) { Traits::Retain(data); }
		inline Handle(Handle& handle) { _data = handle.move(); }
		inline Handle(Handle&& handle) { _data = handle.move(); }
		
		~Handle() { release(); }
		
		inline Handle& operator=(Handle& handle) {
			release();
			_data = handle.move();
			return *this;
		}
		
		inline Handle& operator=(Handle&& handle) {
			release();
			_data = handle.move();
			return *this;
		}
		
		inline operator bool() const { return _data != nullptr; }
		inline T* operator->() { return _data; }
		inline T* operator*() { return _data; }
		inline T* value() { return _data; }
		inline const T* operator->() const { return _data; }
		inline const T* operator*() const { return _data; }
		inline const T* value() const { return _data; }

		/**
		* @func is_null() Is null data available ?
		*/
		inline bool is_null() const {
			return _data == nullptr;
		}
		
		/**
		* @func collapse() 解绑数据,用函数失去对数据的管理权,数据被移走
		*/
		inline T* collapse() {
			T* data = _data; 
			_data = nullptr;
			return data;
		}

		inline void release() {
			Traits::Release(_data);
			_data = nullptr;
		}
		
	private:

		T* _data;
	};

	/**
	 * Shared pointer
	 */
	template <class T, class T2 = typename T::Traits> using Sp = Handle<T, T2>;

	/**
	* @class ScopeClear
	*/
	class ClearScope {
	public:
		typedef std::function<void()> Clear;
		ClearScope(Clear clear): _clear(clear) { }
		~ClearScope() { _clear(); }
		inline void cancel() { _clear = [](){}; }
	private:
		Clear _clear;
	};

}
#endif