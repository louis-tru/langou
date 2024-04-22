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

#include "../_js.h"
#include "../_view.h"
#include "../../views2/div.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapDiv
 */
class WrapDiv: public WrapViewBase {
	public:

	static void constructor(FunctionCall args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapDiv>(args, new Div());
	}
	
	static void content_align(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Div);
		Js_Return( worker->values()->New(self->content_align()) );
	}
	
	static void set_content_align(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(ContentAlign, value, "Div.contentAlign = %s");
		Js_Self(Div);
		self->set_content_align(out);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Define_Class(Div, constructor, {
			Js_Set_Class_Accessor(contentAlign, content_align, set_content_align);
		}, Box);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Div), View::DIV);
	}
};

void binding_div(Local<JSObject> exports, Worker* worker) {
	WrapDiv::binding(exports, worker);
}

Js_END
