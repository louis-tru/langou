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

#include "./font.h"
#include "./pool.h"
#include <skia/core/SkFont.h>
#include <skia/core/SkTypeface.h>
#include <skia/core/SkFontMetrics.h>
#include <skia/core/SkFontTypes.h>

namespace quark {

	inline const SkFont* CastSkFont(const FontGlyphs* fg) {
		return reinterpret_cast<const SkFont*>(reinterpret_cast<const Array<GlyphID>*>(fg) + 1);
	}

	FontGlyphs::FontGlyphs(const Typeface& typeface, float fontSize, const GlyphID glyphs[], uint32_t count)
		: _typeface( *((void**)(&typeface)) )
		, _fontSize(fontSize)
		, _scaleX(1)
		, _skewX(0)
		, _flags(1 << 5)
		, _edging(static_cast<unsigned>(SkFont::Edging::kAntiAlias))
		, _hinting(static_cast<unsigned>(SkFontHinting::kNormal))
	{
		_glyphs.write(glyphs, 0, count);
		_glyphs.realloc(count + 1);
		(*_glyphs)[count] = 0;
	}

	Array<float> FontGlyphs::get_offset() {
		auto font = CastSkFont(this);
		auto len = _glyphs.length() + 1;
		Array<float> offset(len);
		font->getXPos(*_glyphs, len, *offset);
		return offset;
	}
	
	const Typeface& FontGlyphs::typeface() const {
		return *((const Typeface*)&_typeface);
	}

	float FontGlyphs::get_metrics(FontMetrics* metrics) const {
		return CastSkFont(this)->getMetrics( (SkFontMetrics*)metrics );
	}

	float FontGlyphs::get_metrics(FontMetrics* metrics, FFID FFID, FontStyle style, float fontSize) {
		return FontGlyphs(FFID->match(style)[0], fontSize, nullptr, 0).get_metrics(metrics);
	}

	float FontGlyphs::get_metrics(FontMetrics* metrics, const Typeface& typeface, float fontSize) {
		return FontGlyphs(typeface, fontSize, nullptr, 0).get_metrics(metrics);
	}

}
