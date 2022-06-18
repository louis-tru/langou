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

#include "./root.h"
#include "../effect.h"
#include "../app.inl"
#include "../util/handle.h"
#include "../display.h"
#include "../render/render.h"
#include "../pre_render.h"

namespace noug {

	void __View_set_visible(View* self, bool val, uint32_t layout_depth);

	Root* Root::create() throw(Error) {
		auto app = noug::app();
		N_CHECK(app, "Before you create a root, you need to create a Application");
		Handle<Root> r = new Root();
		r->set_layout_depth(1);
		r->set_receive(1);
		r->set_width({0, BoxSizeKind::MATCH});
		r->set_height({0, BoxSizeKind::MATCH});
		
		auto region = app->display()->display_region();
		float scale = app->display()->scale();

		r->set_translate(Vec2(region.x / scale, region.y / scale));

		r->mark(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		r->set_fill_color(Color(255, 255, 255, 255)); // 默认白色背景
		r->mark_none(kRecursive_Transform);
		_inl_app(app)->set_root(*r);
		return r.collapse();
	}

	void Root::onDisplayChange() {
		auto display = pre_render()->host()->display();
		auto region = display->display_region();
		float scale = display->scale();
		mark(Layout::kLayout_Size_Width | Layout::kLayout_Size_Height);
		set_translate(Vec2(region.x / scale, region.y / scale));
	}

	void Root::set_visible(bool val) {
		if (visible() != val) {
			__View_set_visible(this, val, val ? 1: 0);
		}
	}

	bool Root::layout_forward(uint32_t _mark) {
		if (_mark & (kLayout_Size_Width | kLayout_Size_Height)) {
			auto display = pre_render()->host()->display();
			Size size{ Vec2(), display->size(), false, false };
			Vec2 xy(solve_layout_content_width(size), solve_layout_content_height(size));

			xy += Vec2(margin_left() + margin_right(), margin_top() + margin_bottom());
			xy += Vec2(padding_left() + padding_right(), padding_top() + padding_bottom());
			if (_border)
				xy += Vec2(_border->width_left + _border->width_right, _border->width_top + _border->width_bottom);

			set_layout_size(xy, &size.wrap_x, false);
		}

		if (layout_mark() & kLayout_Typesetting) {
			return true;
		} else if (_mark & kTransform_Origin) {
			solve_origin_value();
		}

		return false;
	}

	bool Root::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			Vec2 size = content_size();
			auto v = first();
			while (v) {
				v->set_layout_offset_lazy(size); // lazy layout
				v = v->next();
			}
			unmark(kLayout_Typesetting);

			solve_origin_value();
		}
		return false; // stop iteration
	}

	Mat Root::layout_matrix() {
		if (_transform) {
			return Mat(
				layout_offset() + Vec2(margin_left(), margin_top()) +
									origin_value() + _transform->translate, // translate
				_transform->scale,
				-_transform->rotate,
				_transform->skew
			);
		} else {
			Vec2 translate = layout_offset() + Vec2(margin_left(), margin_top()) + origin_value();
			return Mat(
				1, 0, translate.x(),
				0, 1, translate.y()
			);
		}
	}

	void Root::set_parent(View* parent) {
		N_UNREACHABLE();
	}

	bool Root::can_become_focus() {
		return true;
	}

}
