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


#ifndef __noug__text_blob__
#define __noug__text_blob__

#include "./render/font/font.h"
#include "./render/source.h"
#include "./text_rows.h"
#include "./text_opts.h"

namespace noug {

	N_EXPORT Array<Array<Unichar>> string_to_unichar(cString& str, TextWhiteSpace space);

	struct TextBlob {
		Typeface        typeface;
		Array<GlyphID>  glyphs;
		Array<float>    offset;
		float           origin;
		uint32_t        row_num;
		Sp<ImageSource> cache;
	};

	class N_EXPORT TextBlobBuilder {
	public:
		TextBlobBuilder(
			TextRows *rows, TextConfig *cfg, Array<TextBlob>* blob);
		void make_as_no_auto_wrap(FontGlyphs &fg, Unichar *unichar);
		void make_as_auto_wrap(FontGlyphs &fg, Unichar *unichar);
		void make_as_normal(FontGlyphs &fg, Unichar *unichar);
		void make_as_break_all(FontGlyphs &fg, Unichar *unichar);
		void make_as_keep_all(FontGlyphs &fg, Unichar *unichar);
	private:
		TextRows *rows;
		TextConfig *cfg;
		Array<TextBlob>* blob;
	};

}
#endif
