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

#ifndef __quark__font__typeface__
#define __quark__font__typeface__

#include "../../util/string.h"
#include "../../math.h"
#include "./style.h"
#include "metrics.h"

namespace quark {

	class FontPool;
	typedef uint32_t Unichar;
	typedef uint16_t GlyphID;
	typedef uint32_t FontTableTag;

	class Qk_EXPORT Typeface: public Reference {
		Qk_HIDDEN_ALL_COPY(Typeface);
	public:
		Qk_DEFINE_PROP_GET(FontStyle, fontStyle);
		Qk_DEFINE_PROP_GET(bool, isFixedPitch);
		bool isBold() const { return _fontStyle.weight() >= TextWeight::SEMIBOLD; }
		bool isItalic() const { return _fontStyle.slant() != TextSlant::DEFAULT; }
		int countGlyphs() const;
		int countTables() const;
		int getTableTags(FontTableTag tags[]) const;
		Buffer getTableData(FontTableTag tag) const;
		size_t getTableSize(FontTableTag) const;
		int getUnitsPerEm() const;
		String getFamilyName() const;
		bool getPostScriptName(String* name) const;
		void unicharsToGlyphs(const Unichar unichar[], uint32_t count, GlyphID glyphs[]) const;
		Array<GlyphID> unicharsToGlyphs(const Array<Unichar>& unichar) const;
		GlyphID unicharToGlyph(Unichar unichar) const;
		float getMetrics(FontMetrics* metrics, float fontSize);
	protected:
		Typeface(FontStyle fs, bool isFixedPitch);
		void setIsFixedPitch(bool isFixedPitch) { _isFixedPitch = isFixedPitch; }
		void setFontStyle(FontStyle style) { _fontStyle = style; }
		virtual int onCountGlyphs() const = 0;
		virtual int onGetUPEM() const = 0;
		virtual String onGetFamilyName() const = 0;
		virtual bool onGetPostScriptName(String*) const = 0;
		virtual int onGetTableTags(FontTableTag tags[]) const = 0;
		virtual size_t onGetTableData(FontTableTag, size_t offset, size_t length, void* data) const = 0;
		virtual void onCharsToGlyphs(const Unichar* chars, int count, GlyphID glyphs[]) const = 0;
		virtual void onGetMetrics(FontMetrics* metrics, float fontSize) const = 0;
	private:
		Dict<float, FontMetrics> _MetricsCaches;
	};

}
#endif
