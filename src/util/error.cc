/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "./error.h"
#include "./util.h"

namespace qk {

	Error::Error(const Error& e)
		: _code(e.code())
		, _msg(e._msg) {
	}
	
	Error::Error(cChar* msg, ...): _code(ERR_UNKNOWN_ERROR) {
		va_list arg;
		va_start(arg, msg);
		_msg = _Str::string_format(msg, arg);
		va_end(arg);
	}

	Error::Error(int code, cChar* msg, ...): _code(code) {
		va_list arg;
		va_start(arg, msg);
		_msg = _Str::string_format(msg, arg);
		va_end(arg);
	}

	Error::Error(int code, cString& msg)
		: _code(code)
		, _msg(msg) {
	}

	Error& Error::operator=(const Error& e) {
		_code = e._code;
		_msg = e._msg;
		return *this;
	}

	cString& Error::message() const throw() {
		return _msg;
	}

	int Error::code() const throw() {
		return _code;
	}
}
