/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "./label.h"
#include "../util/codec.h"
#include "../pre_render.h"
#include "../app.h"

namespace noug {

	void Label::set_text_value(String val) {
		if (_text_value != val) {
			_text_value = std::move(val);
			mark(kLayout_Size_Width | kLayout_Size_Height);
		}
	}

	void Label::onTextChange(uint32_t value, uint32_t flags) {
		value ? mark(value): mark_none();
	}

	bool Label::layout_forward(uint32_t mark) {
		return true; // continue iteration
	}

	bool Label::layout_reverse(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting)) {
			parent()->onChildLayoutChange(this, kChild_Layout_Text);
			unmark(kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting);
		}
		return false;
	}

	void Label::layout_text(TextRows *rows, TextConfig *base) {
		TextConfig cfg(this, base);
		ArrayBuffer<Unichar> bf = Codec::decode_to_uint32(Encoding::UTF8, _text_value);
		
		
		//	enum class TextWhiteSpace: uint8_t {
		//		INHERIT, // inherit
		//		NORMAL,        /* 保留所有空白,使用自动wrap */
		//		NO_WRAP,       /* 合并空白序列,不使用自动wrap */
		//		NO_SPACE,      /* 合并空白序列,使用自动wrap */
		//		PRE,           /* 保留所有空白,不使用自动wrap */
		//		PRE_LINE,      /* 合并空白符序列,但保留换行符,使用自动wrap */
		//		WRAP,          /* 保留所有空白,强制使用自动wrap */
		//		DEFAULT = NORMAL,
		//	};
		
		Array<FontGlyphs> fgs = text_family().value->makeFontGlyphs(bf, cfg.font_style(), cfg.text_size());

		for (auto& fg: fgs) {
			Array<float> offset = fg.get_offset();
		}
	}

	void Label::set_layout_offset(Vec2 val) {
		auto size = parent()->layout_size();
		Sp<TextRows> rows = new TextRows(size.content_size, false, false, TextAlign::LEFT); // use left align
		layout_text(*rows, pre_render()->host()->default_text_options());
		mark_none(kRecursive_Transform);
	}

	void Label::set_layout_offset_lazy(Vec2 size) {
		Sp<TextRows> rows = new TextRows(size, false, false, TextAlign::LEFT); // use left align
		layout_text(*rows, pre_render()->host()->default_text_options());
		mark_none(kRecursive_Transform);
	}

	void Label::onParentLayoutContentSizeChange(Layout* parent, uint32_t value) {
		mark(value);
	}

	bool Label::solve_visible_region() {
		// TODO ...
		return true;
	}

}
