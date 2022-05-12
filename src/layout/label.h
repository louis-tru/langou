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

#ifndef __noug__layout__label__
#define __noug__layout__label__

#include "./view.h"
#include "../render/font/font.h"
#include "../text_rows.h"

namespace noug {

	class FontPool;

	class N_EXPORT TextBasic {
	public:
		TextBasic();
		TextBasic(FontPool* pool);
		N_DEFINE_PROP(TextColor, text_background_color);
		N_DEFINE_PROP(TextColor, text_color);
		N_DEFINE_PROP(TextSize, text_size);
		N_DEFINE_PROP(TextWeight, text_weight);
		N_DEFINE_PROP(TextStyle, text_style);
		N_DEFINE_PROP(TextFamily, text_family);
		N_DEFINE_PROP(TextShadow, text_shadow);
		N_DEFINE_PROP(TextLineHeight, text_line_height);
		N_DEFINE_PROP(TextDecoration, text_decoration);
		N_DEFINE_PROP(TextOverflow, text_overflow);
		N_DEFINE_PROP(TextWhiteSpace, text_white_space);
	protected:
		virtual void onTextChange(uint32_t mark);
	};

	class N_EXPORT Label: public View, public TextBasic {
		N_Define_View(Label);
	public:
		N_DEFINE_PROP(String, text_value);
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextRows *rows) override;
		virtual void set_layout_offset(Vec2 val) override;
		virtual void set_layout_offset_lazy(Vec2 size) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;
		virtual bool solve_visible_region() override;
	protected:
		virtual void onTextChange(uint32_t mark) override;
		Array<TextBlob> _blob;
		Sp<TextRows>    _rows;
	};

}
#endif
