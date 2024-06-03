/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "./label.h"
#include "../screen.h"
#include "../app.h"

namespace qk {

	void Label::set_value(String val, bool isRt) {
		if (_value != val) {
			_value = std::move(val);
			mark_layout(kLayout_Size_Width | kLayout_Size_Height, isRt);
		}
	}

	View* Label::getViewForTextOptions() {
		return this;
	}

	bool Label::layout_forward(uint32_t mark) {
		return false; // continue iteration
	}

	bool Label::layout_reverse(uint32_t mark) {
		if (mark & (kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting)) {
			parent_Rt()->onChildLayoutChange(this, kChild_Layout_Text);
			unmark(kLayout_Size_Width | kLayout_Size_Height | kLayout_Typesetting);
		}
		return true; // complete
	}

	void Label::layout_text(TextLines *lines, TextConfig *base) {
		TextConfig cfg(this, base);

		_blob_visible.clear();
		_blob.clear();
		_lines = lines;

		TextBlobBuilder tbb(lines, this, &_blob);

		tbb.make(_value);

		auto v = first_Rt();
		while(v) {
			if (v->visible()) {
				v->layout_text(lines, &cfg);
			}
			v = v->next_Rt();
		}
	}

	void Label::set_layout_offset(Vec2 val) {
		Sp<TextLines> lines = new TextLines(this, text_align_value(), Vec2(), false);
		layout_text(*lines, shared_app()->defaultTextOptions());
		lines->finish();
		mark(kRecursive_Transform, true);
	}

	void Label::set_layout_offset_lazy(Vec2 size) {
		Sp<TextLines> lines = new TextLines(this, text_align_value(), size, false);
		layout_text(*lines, shared_app()->defaultTextOptions());
		lines->finish();
		mark(kRecursive_Transform, true);
	}

	void Label::onParentLayoutContentSizeChange(View* parent, uint32_t value) {
		mark_layout(value, true);
	}

	bool Label::solve_visible_region(const Mat &mat) {
		if (_lines->host() == this)
			_lines->solve_visible_region(mat);
		_lines->solve_visible_region_blob(&_blob, &_blob_visible);
		return _blob_visible.length();
	}

	void Label::onActivate() {
		_textFlags = 0xffffffff;
	}

	TextOptions* Label::asTextOptions() {
		return this;
	}

	ViewType Label::viewType() const {
		return kLabel_ViewType;
	}

}
