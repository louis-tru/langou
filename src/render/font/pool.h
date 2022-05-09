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

#ifndef __noug__font__pool__
#define __noug__font__pool__

#include "../../value.h"
#include "../../util/dict.h"
#include "./typeface.h"
#include "./style.h"

namespace noug {

	class Application;

	class N_EXPORT FontFamilysID {
	public:
		inline const Array<String>& familys() const { return _familys; }
		N_DEFINE_PROP_READ(uint64_t, code);
		static FFID Make(cString familys);
		static FFID Make(const Array<String>& familys);
	private:
		FontFamilysID(Array<String>& familys, uint64_t code);
		Array<String> _familys;
		friend class FontPool;
	};

	class N_EXPORT FontPool: public Object {
		N_HIDDEN_ALL_COPY(FontPool);
	public:
		FontPool(Application* host);
		virtual ~FontPool();
		Array<String> family_names() const;
		Typeface typeface(cString& familyName, const FontStyle& style);
		void register_from_data(cBuffer& buff);
		void register_from_file(cString& path);
		const Array<Typeface>& default_typeface();
		// define ptops
		N_DEFINE_PROP_READ(Application*, host);
	private:
		void set_default_typeface();
		void           *_impl;
		Array<Typeface> _default_tf;
		Dict<String, Dict<int32_t, Typeface>> _register_tf;
	};

}
#endif
