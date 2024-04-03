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

#ifndef __quark__font__font__
#define __quark__font__font__

#include "../../util/handle.h"
#include "./typeface.h"

namespace qk {
	class FontPool;

	class Qk_EXPORT FontGlyphs {
	public:
		FontGlyphs(float fontSize, Typeface *ft, const GlyphID glyphs[], uint32_t count);
		FontGlyphs(float fontSize, Typeface *ft, Array<GlyphID> &&glyphs);
		Qk_DEFINE_PROP(float, fontSize, Const); // fontSize prop
		Array<Vec2> getHorizontalOffset(Vec2 origin = 0) const;
		inline Typeface* typeface() const { return const_cast<Typeface*>(*_typeface); }
		inline cArray<GlyphID>& glyphs() const { return _glyphs; }
		inline uint32_t length() const { return _glyphs.length(); }
	private:
		Array<GlyphID> _glyphs;
		Sp<Typeface> _typeface;
	};

	class Qk_EXPORT FontFamilys: public Object {
	public:
		FontFamilys(FontPool* pool, Array<String>& familys);
		Qk_DEFINE_PROP_ACC_GET(cArray<String>&, familys, Const);
		Qk_DEFINE_PROP_GET(FontPool*, pool);
		Sp<Typeface> match(FontStyle style, uint32_t index = 0);
		Array<FontGlyphs> makeFontGlyphs(cArray<Unichar>& unichars, FontStyle style, float fontSize);
	private:
		Array<Sp<Typeface>>& matchs(FontStyle style);
		Array<String> _familys;
		Dict<FontStyle, Array<Sp<Typeface>>> _TFs;
		friend class FontPool;
	};

}
#endif
