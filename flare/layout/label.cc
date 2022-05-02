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
#include "../text_rows.h"
#include "../render/font/pool.h"

namespace flare {

	TextBasic::TextBasic()
		: _text_background_color{Color(0, 0, 0, 0), TextValueType::VALUE}
		, _text_color{Color(0, 0, 0), TextValueType::VALUE}
		, _text_size{16, TextValueType::VALUE}
		, _text_weight{TextWeightValue::REGULAR, TextValueType::VALUE}
		, _text_style{TextStyleValue::NORMAL, TextValueType::VALUE}
		, _text_family{FontPool::get_font_familys_id(String()), TextValueType::VALUE}
		, _text_shadow{{ 0, 0, 0, Color(0, 0, 0) }, TextValueType::VALUE}
		, _text_line_height{0, TextValueType::VALUE}
		, _text_decoration{TextDecorationValue::NONE, TextValueType::VALUE}
		, _text_overflow{TextOverflowValue::NORMAL, TextValueType::VALUE}
		, _text_white_space{TextWhiteSpaceValue::NORMAL, TextValueType::VALUE}
	{
	}

	void TextBasic::onTextChange(uint32_t mark) {
		// noop
	}

	void TextBasic::set_text_background_color(TextColor value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_background_color = value;
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextBasic::set_text_color(TextColor value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_color = value;
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextBasic::set_text_size(TextSize value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_size = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextBasic::set_text_style(TextStyle value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_style = value;
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextBasic::set_text_family(TextFamily value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_family = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextBasic::set_text_shadow(TextShadow value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_shadow = value;
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextBasic::set_text_line_height(TextLineHeight value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_line_height = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextBasic::set_text_decoration(TextDecoration value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_decoration = value;
			onTextChange(Layout::kLayout_None);
		}
	}

	void TextBasic::set_text_overflow(TextOverflow value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_overflow = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	void TextBasic::set_text_white_space(TextWhiteSpace value) {
		if ( value.type == TextValueType::VALUE ) {
			_text_white_space = value;
			onTextChange(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		}
	}

	// ---------------- L a b e l ----------------

	void Label::set_text_value(String val) {
		if (_text_value != val) {
			_text_value = std::move(val);
			mark(kLayout_Size_Width | kLayout_Size_Height);
		}
	}

	void Label::onTextChange(uint32_t value) {
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

	void Label::layout_text(TextRows *rows) {
		// TODO ...
		// TextAlign align = TextAlign::LEFT;
		// left / center / right / LEFT_REVERSE / CENTER_REVERSE / RIGHT_REVERSE
	}

	void Label::set_layout_offset(Vec2 val) {
		auto size = parent()->layout_size();
		TextRows rows(size.content_size, false, false, TextAlign::LEFT); // use left align
		layout_text(&rows);
		mark_none(kRecursive_Transform);
	}

	void Label::set_layout_offset_lazy(Vec2 size) {
		TextRows rows(size, false, false, TextAlign::LEFT); // use left align
		layout_text(&rows);
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
