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

#include "./view.h"
#include "../../ui/css/css.h"
#include "../../ui/app.h"

#define Js_Set_StyleSheets_Accessor(T, Prop, Name) \
	Js_Set_Class_Accessor_Set(Name, { \
		Js_Parse_Type(T, val, "@prop StyleSheets."#Name" = %s"); \
		Js_Self(Type); \
		self->set_##Prop(out); \
	})

namespace qk { namespace js {
	typedef qk::Wrap Wrap;

	class WrapStyleSheets: public WrapObject {
	public:
		typedef StyleSheets Type;

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(StyleSheets, 0, { Js_Throw("Access forbidden."); });
			Js_Set_StyleSheets_Accessor(float, opacity, opacity);
			Js_Set_StyleSheets_Accessor(bool, visible, visible);
			Js_Set_StyleSheets_Accessor(bool, receive, receive);
			Js_Set_StyleSheets_Accessor(bool, clip, clip);
			Js_Set_StyleSheets_Accessor(BoxSize, width, width);
			Js_Set_StyleSheets_Accessor(BoxSize, height, height);
			Js_Set_StyleSheets_Accessor(BoxSize, width_limit, widthLimit);
			Js_Set_StyleSheets_Accessor(BoxSize, height_limit, heightLimit);
			Js_Set_StyleSheets_Accessor(float, margin_top, marginTop);
			Js_Set_StyleSheets_Accessor(float, margin_right, marginRight);
			Js_Set_StyleSheets_Accessor(float, margin_bottom, marginBottom);
			Js_Set_StyleSheets_Accessor(float, margin_left, marginLeft);
			Js_Set_StyleSheets_Accessor(float, padding_top, paddingTop);
			Js_Set_StyleSheets_Accessor(float, padding_right, paddingRight);
			Js_Set_StyleSheets_Accessor(float, padding_bottom, paddingBottom);
			Js_Set_StyleSheets_Accessor(float, padding_left, paddingLeft);
			Js_Set_StyleSheets_Accessor(float, border_radius_left_top, borderRadiusLeftTop);
			Js_Set_StyleSheets_Accessor(float, border_radius_right_top, borderRadiusRightTop);
			Js_Set_StyleSheets_Accessor(float, border_radius_right_bottom, borderRadiusRightBottom);
			Js_Set_StyleSheets_Accessor(float, border_radius_left_bottom, borderRadiusLeftBottom);
			Js_Set_StyleSheets_Accessor(Color, border_color_top, borderColorTop);
			Js_Set_StyleSheets_Accessor(Color, border_color_right, borderColorRight);
			Js_Set_StyleSheets_Accessor(Color, border_color_bottom, borderColorBottom);
			Js_Set_StyleSheets_Accessor(Color, border_color_left, borderColorLeft);
			Js_Set_StyleSheets_Accessor(float, border_width_top, borderWidthTop);
			Js_Set_StyleSheets_Accessor(float, border_width_right, borderWidthRight);
			Js_Set_StyleSheets_Accessor(float, border_width_bottom, borderWidthBottom);
			Js_Set_StyleSheets_Accessor(float, border_width_left, borderWidthLeft);
			Js_Set_StyleSheets_Accessor(Color, background_color, backgroundColor);
			Js_Set_StyleSheets_Accessor(BoxFilterPtr, background, background);
			Js_Set_StyleSheets_Accessor(BoxShadowPtr, box_shadow, boxShadow);
			Js_Set_StyleSheets_Accessor(float, weight, weight);
			Js_Set_StyleSheets_Accessor(Align, align, align);
			Js_Set_StyleSheets_Accessor(Direction, direction, direction);
			Js_Set_StyleSheets_Accessor(ItemsAlign, items_align, itemsAlign);
			Js_Set_StyleSheets_Accessor(CrossAlign, cross_align, crossAlign);
			Js_Set_StyleSheets_Accessor(Wrap, wrap, wrap);
			Js_Set_StyleSheets_Accessor(WrapAlign, wrap_align, wrapAlign);
			Js_Set_StyleSheets_Accessor(String, src, src);
			Js_Set_StyleSheets_Accessor(TextAlign, text_align, textAlign);
			Js_Set_StyleSheets_Accessor(TextWeight, text_weight, textWeight);
			Js_Set_StyleSheets_Accessor(TextSlant, text_slant, textSlant);
			Js_Set_StyleSheets_Accessor(TextDecoration, text_decoration, textDecoration);
			Js_Set_StyleSheets_Accessor(TextOverflow, text_overflow, textOverflow);
			Js_Set_StyleSheets_Accessor(TextWhiteSpace, text_white_space, textWhiteSpace);
			Js_Set_StyleSheets_Accessor(TextWordBreak, text_word_break, textWordBreak);
			Js_Set_StyleSheets_Accessor(TextSize, text_size, textSize);
			Js_Set_StyleSheets_Accessor(TextColor, text_background_color, textBackgroundColor);
			Js_Set_StyleSheets_Accessor(TextColor, text_color, textColor);
			Js_Set_StyleSheets_Accessor(TextSize, text_line_height, textLineHeight);
			Js_Set_StyleSheets_Accessor(TextShadow, text_shadow, textShadow);
			Js_Set_StyleSheets_Accessor(TextFamily, text_family, textFamily);
			Js_Set_StyleSheets_Accessor(bool, security, security);
			Js_Set_StyleSheets_Accessor(bool, readonly, readonly);
			Js_Set_StyleSheets_Accessor(KeyboardType, type, type);
			Js_Set_StyleSheets_Accessor(KeyboardReturnType, return_type, returnType);
			Js_Set_StyleSheets_Accessor(Color, placeholder_color, placeholderColor);
			Js_Set_StyleSheets_Accessor(Color, cursor_color, cursorColor);
			Js_Set_StyleSheets_Accessor(uint32_t, max_length, maxLength);
			Js_Set_StyleSheets_Accessor(String, placeholder, placeholder);
			Js_Set_StyleSheets_Accessor(Color, scrollbar_color, scrollbarColor);
			Js_Set_StyleSheets_Accessor(float, scrollbar_width, scrollbarWidth);
			Js_Set_StyleSheets_Accessor(float, scrollbar_margin, scrollbarMargin);
			Js_Set_StyleSheets_Accessor(float, x, x);
			Js_Set_StyleSheets_Accessor(float, y, y);
			Js_Set_StyleSheets_Accessor(float, scale_x, scaleX);
			Js_Set_StyleSheets_Accessor(float, scale_y, scaleY);
			Js_Set_StyleSheets_Accessor(float, skew_x, skewX);
			Js_Set_StyleSheets_Accessor(float, skew_y, skewY);
			Js_Set_StyleSheets_Accessor(float, rotate_z, rotateZ);
			Js_Set_StyleSheets_Accessor(float, origin_x, originX);
			Js_Set_StyleSheets_Accessor(float, origin_y, originY);
		}
	};

	class WrapCStyleSheets: public WrapObject {
	public:
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(CStyleSheets, StyleSheets, { Js_Throw("Access forbidden."); });
		}
	};

	class NativeCSS {
	public:
		static void create(Worker *worker, JSArray *names, JSObject *arg) {
			auto rss = shared_app()->styleSheets();

			for ( uint32_t i = 0, len = names->length(); i < len; i++ ) {
				auto key = names->get(worker, i);
				auto val = arg->get(worker, key);
				if ( !val ) return; // js error
				if ( !val->isObject() ) {
					Js_Throw("Bad argument.");
				}
				auto arr = rss->search( key->toStringValue(worker, true) );

				if ( arr.length() ) {
					auto props = val->cast<JSObject>();
					auto names = props->getPropertyNames(worker);

					for ( uint32_t j = 0, len = names->length(); j < len; j++ ) {
						auto key = names->get(worker, j);
						auto val = props->get(worker, key);
						if ( !val ) return; // js error
						for ( auto ss : arr ) {
							auto that = WrapObject::wrap<CStyleSheets>(ss)->that();
							if ( !that->set(worker, key, val) ) {
								return; // js error
							}
						}
					}
				} // if (arr.length)
			} // for
		}

		static void binding(JSObject* exports, Worker* worker) {
			worker->bindingModule("_types");
			WrapStyleSheets::binding(exports, worker);

			Js_Set_Method("create", {
				Qk_Fatal_Assert(shared_app());

				if ( args.length() < 1 || !args[0]->isObject() || args[0]->isNull() ) {
					Js_Throw("Bad argument.");
				}
				Js_Handle_Scope();

				auto arg = args[0]->cast<JSObject>();
				auto names = arg->getPropertyNames(worker);
				if (names->length()) {
					shared_app()->lockAllRenderThreads(Cb([worker,names,arg](auto& e) {
						create(worker, names, arg);
					}));
				}
			});
		}
	};

	Js_Set_Module(_css, NativeCSS);
}	}
