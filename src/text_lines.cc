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

#include "./display.h"
#include "./pre_render.h"
#include "./app.h"
#include "./layout/view.h"
#include "./text_lines.h"
#include "./text_opts.h"
#include "./text_blob.h"
#include "./render/font/font.h"

namespace noug {

	TextLines::TextLines(View *host, TextAlign text_align, Vec2 size, bool no_wrap)
		: _pre_width(0), _trim_start(false), _host(host)
		, _size(size), _no_wrap(no_wrap), _text_align(text_align), _visible_region(false)
	{
		clear();
	}

	void TextLines::clear() {
		_lines.clear();
		_lines.push({ 0, 0, 0, 0, 0, 0, 0, 0 });
		_last = &_lines[0];
		_max_width = 0;
		_min_origin = Float::limit_max;
		_visible_region = false;
	}

	void TextLines::push(bool trim_start) {
		finish_line();
		_lines.push({ _last->end_y, 0, 0, 0, 0, 0, 0, _lines.length() });
		_last = &_lines.back();
		_trim_start = trim_start;
		
		if (trim_start) {
			// skip line start space
			for (auto &blob: _preBlob) {
				auto id = blob.typeface.unicharToGlyph(0x20); // space
				int i = 0, len = blob.glyphs.length();
			check:
				if (blob.glyphs[i] != id) {
					blob.glyphs = blob.glyphs.copy(i);
					blob.offset = blob.offset.copy(i);
					break;
				}
				if (++i < len) {
					blob.index++;
					goto check; // skip space
				}
				blob.glyphs.clear();
				blob.offset.clear();
			}
		}
		
		_pre_width = 0;
		
		for (auto &blob: _preBlob) {
			_pre_width += blob.offset.back() - blob.offset.front();
		}
		if (_pre_width) {
			_trim_start = false;
		}
	}

	void TextLines::push(TextOptions *opts) {
		push(false); // new row
		set_metrics(opts);
	}

	void TextLines::finish_line() {

		switch(_text_align) {
			case TextAlign::LEFT: break;
			case TextAlign::CENTER: _last->origin = (_size.x() - _last->width) / 2; break;
			case TextAlign::RIGHT:  _last->origin = _size.x() - _last->width; break;
		}

		if ( _last->width > _max_width ) {
			_max_width = _last->width;
		}
		if ( _last->origin < _min_origin) {
			_min_origin = _last->origin;
		}

		if (_preLayout.length()) {
			auto top = _last->top;
			auto bottom = _last->bottom;

			for (auto layout: _preLayout) {
				auto height = layout->layout_size().layout_size.y();
				switch (layout->layout_align()) {
					case Align::START:  set_metrics(top, height - bottom); break;
					case Align::CENTER: height = (height - top - bottom) / 2;
						set_metrics(height + top, height + bottom); break;
					case Align::END:    set_metrics(height - bottom, bottom); break;
					default:            set_metrics(height, 0); break;
				}
			}

			for (auto layout: _preLayout) {
				auto size_y = layout->layout_size().layout_size.y();
				auto x = _last->origin + layout->layout_offset().x();
				float y;

				switch (layout->layout_align()) {
					case Align::START:  y = _last->baseline - top; break;
					case Align::CENTER: y = _last->baseline - (size_y + top - bottom) / 2; break;
					case Align::END: y = _last->baseline - size_y + bottom; break;
					default:         y = _last->baseline - size_y; break;
				}
				layout->set_layout_offset(Vec2(x, y));
			}

			_preLayout.clear();
		}
	}

	void TextLines::finish() {
		finish_text_blob();
		finish_line();
	}

	void TextLines::set_metrics(float top, float bottom) {
		if (top > _last->top || bottom > _last->bottom) {
			_last->top = top;
			_last->bottom = bottom;
			_last->baseline = _last->start_y + _last->top;
			_last->end_y = _last->baseline + _last->bottom;
		}
	}

	void TextLines::set_metrics(FontMetrics *metrics, float line_height) {
		auto top = -metrics->fAscent;
		auto bottom = metrics->fDescent + metrics->fLeading;
		auto height = top + bottom;
		if (line_height != 0) { // value
			auto y = (line_height - height) / 2;
			top += y; bottom += y;
			if (bottom < 0) {
				top += bottom;
				bottom = 0;
			}
		}
		set_metrics(top, bottom);
	}

	void TextLines::set_metrics(TextOptions *opts) {
		FontMetrics metrics;
		FontGlyphs::get_metrics(&metrics, opts->text_family().value, opts->font_style(), opts->text_size().value);
		set_metrics(&metrics, opts->text_line_height().value);
	}

	void TextLines::add_layout(Layout* layout) {
		_preLayout.push(layout);
	}

	void TextLines::finish_text_blob() {
		add_text_blob({}, Array<GlyphID>(), Array<float>(), false); // solve text blob
	}

	void TextLines::solve_visible_region() {
		// solve lines visible region
		auto& clip = _host->pre_render()->host()->display()->clip_region();
		auto& mat = _host->matrix();
		auto  offset_in = _host->layout_offset_inside();
		auto  x1 = _min_origin + offset_in.x();
		auto  x2 = x1 + _max_width;
		auto  y  = offset_in.y();

		Vec2 vertex[4];

		vertex[0] = mat * Vec2(x1, _lines.front().start_y + y);
		vertex[1] = mat * Vec2(x2, _lines.front().start_y + y);
		
		bool is_all_false = false;
		
		// TODO
		// Use optimization algorithm using dichotomy

		for (auto &line: _lines) {
			if (is_all_false) {
				line.visible_region = false;
				continue;
			}
			auto y2 = line.end_y + y;
			vertex[3] = mat * Vec2(x1, y2);
			vertex[2] = mat * Vec2(x2, y2);

			auto re = View::screen_region_from_convex_quadrilateral(vertex);

			if (N_MAX( clip.y2, re.end.y() ) - N_MIN( clip.y, re.origin.y() ) <= re.end.y() - re.origin.y() + clip.height &&
					N_MAX( clip.x2, re.end.x() ) - N_MIN( clip.x, re.origin.x() ) <= re.end.x() - re.origin.x() + clip.width
			) {
				line.visible_region = true;
				_visible_region = true;
			} else {
				is_all_false = true;
				line.visible_region = false;
			}
			vertex[0] = vertex[3];
			vertex[1] = vertex[2];
		}

	}

	void TextLines::solve_visible_region_blob(Array<TextBlob> *blob, Array<uint32_t> *blob_visible) {

		N_DEBUG("TextLines::solve_visible_region_blob");

		blob_visible->clear();

		if (!visible_region()) {
			return;
		}

		auto& clip = _host->pre_render()->host()->display()->clip_region();
		bool is_break = false;

		for (int i = 0, len = blob->length(); i < len; i++) {
			auto &item = (*blob)[i];
			auto &line = this->line(item.line);
			if (line.visible_region) {
				is_break = true;
				blob_visible->push(i);
			} else {
				if (is_break) break;
			}
			N_DEBUG("blob,%f,%d,%d,%i", blob.origin, blob.line, blob.glyphs.length(), line.visible_region);
		}
	}

	void TextLines::add_text_blob(PreTextBlob blob, const Array<GlyphID>& glyphs, const Array<float>& offset, bool is_pre) {

		if (is_pre) {
			if (glyphs.length()) {
				blob.glyphs = glyphs.copy();
				blob.offset = offset.copy();
				_preBlob.push(std::move(blob));
			}
			return;
		}

		auto add = [&](PreTextBlob& blob, const Array<GlyphID>& glyphs, const Array<float>& offset) {
			if (glyphs.length() == 0)
				return;

			auto line = _last->line;
			if (blob.blob->length()) {
				auto& last = blob.blob->back();
				// merge glyphs
				if (last.line == line && last.offset.back().x() == offset.front()) {
					last.glyphs.write(glyphs);
					// last.offset.write(offset, -1, -1, 1);
					for (int i = 1; i < offset.length(); i++)
						last.offset.push(Vec2(offset[i], 0));
					_last->width = last.origin + last.offset.back().x();
					return;
				}
			}

			FontMetrics metrics;
			FontGlyphs::get_metrics(&metrics, blob.typeface, blob.text_size);

			auto ascent = -metrics.fAscent;
			auto descent = metrics.fDescent + metrics.fLeading;
			auto height = ascent + descent;
			auto origin = _last->width - offset[0];

			Array<Vec2> pos(offset.length());
			for (int i = 0; i < offset.length(); i++)
				pos[i] = Vec2(offset[i], 0);
			blob.blob->push({ blob.typeface, glyphs.copy(), std::move(pos), ascent, height, origin, line, blob.index });
			_last->width = origin + offset.back();

			set_metrics(&metrics, blob.line_height);
		};

		if (_preBlob.length()) {
			for (auto& i: _preBlob)
				add(i, i.glyphs, i.offset);
			_preBlob.clear();
		}

		add(blob, glyphs, offset);
	}

	void TextLines::set_pre_width(float value) {
		_pre_width = value;
	}

	void TextLines::set_trim_start(bool value) {
		_trim_start = false;
	}

}
