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

#include <math.h>
#include "../util/loop.h"
#include "../util/codec.h"
#include "../app.h"
#include "../display.h"
#include "./render.h"
#include "./gl/gl_render.h"
// layout
#include "../layout/root.h"

#define Qk_ENABLE_DRAW 1

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return Qk_MIN(n, 8);
	}

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
		, _default_scale(1)
		, _alpha(1), _mark_recursive(0)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaaSampleCnt = massSample(_opts.msaaSampleCnt);
	}

	RenderBackend::~RenderBackend() {}

	void RenderBackend::activate(bool isActive) {
	}

	const Array<Vec2>& RenderBackend::getPathVertexsCache(const Path &path) {
		auto hash = path.hashCode();
		auto it = _PathVertexsCache.find(hash);
		if (it != _PathVertexsCache.end()) return it->value;

		if (_PathVertexsCache.length() >= 1024)
			_PathVertexsCache.clear();
		return _PathVertexsCache.set(hash, path.getVertexs(3, 1));
	}

	const Array<Vec2>& RenderBackend::getStrokePathVertexsCache(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miter_limit)
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&miter_limit;
		hash += (hash << 5) + hash_part + ((cap << 2) | join);

		auto it = _PathStrokesCache.find(hash);
		if (it != _PathStrokesCache.end()) {
			return it->value;
		}
		if (_PathStrokesCache.length() >= 1024)
			_PathStrokesCache.clear();
		return _PathStrokesCache
			.set(hash, path.strokePath(width, cap, join, miter_limit).getVertexs(3, 1));
	}

	const Path& RenderBackend::getNormalizedPathCache(const Path &path) {
		auto hash = path.hashCode();
		auto it = _PathNormalizedCache.find(hash);
		if (it != _PathNormalizedCache.end()) return it->value;

		if (_PathNormalizedCache.length() >= 1024)
			_PathNormalizedCache.clear();
		return _PathNormalizedCache.set(hash, path.normalizedPath());
	}

	void RenderBackend::visitView(View* view) {
		// visit child
		auto v = view->_first;
		if (v) {
			float alphaCurr = _alpha;
			uint32_t markCurr = _mark_recursive;
			do {
				if (v->_visible) {
					uint32_t mark = markCurr | v->layout_mark(); // inherit recursive
					if (mark) {
						v->solve_marks(mark);
						_mark_recursive = mark & Layout::kRecursive_Mark;
					}
					if (v->_visible_region && v->_opacity > 0) {
						_alpha = alphaCurr * v->_opacity;
						v->accept(this);
					}
				}
				v = v->_next;
			} while(v);
			_alpha = alphaCurr;
			_mark_recursive = markCurr;
		}
	}

	void RenderBackend::visitBox(Box* box) {
		// TODO ...
	}

	void RenderBackend::visitImage(Image* image) {
		// TODO ...
	}

	void RenderBackend::visitVideo(Video* video) {
		// TODO ...
	}

	void RenderBackend::visitScroll(Scroll* scroll) {
		// TODO ...
	}

	void RenderBackend::visitInput(Input* input) {
		// TODO ...
	}

	void RenderBackend::visitTextarea(Textarea* textarea) {
		// TODO ...
	}

	void RenderBackend::visitButton(Button* btn) {
		// TODO ...
	}

	void RenderBackend::visitTextLayout(TextLayout* text) {
		// TODO ...
	}

	void RenderBackend::visitLabel(Label* label) {
		// TODO ...
	}

	void RenderBackend::visitRoot(Root* v) {
		if (v->_visible) {
			uint32_t mark = v->layout_mark();
			if (mark) {
				v->solve_marks(mark);
				_mark_recursive = mark & Layout::kRecursive_Mark;
			}

			//if (v->_visible_region && v->_opacity > 0) {
				// solveBox(v, [](SkiaRender* render, Box* box, int &clip) {
				// 	if (Qk_ENABLE_DRAW)
				// 		render->_canvas->clear(box->_fill_color.to_uint32_xrgb());
				// 	render->solveFill(box, box->_fill, Color::from(0));
				// });
				//if (Qk_ENABLE_DRAW) _canvas->clearColor(Color4f(0,0,0));
			//} else {
				//if (Qk_ENABLE_DRAW) _canvas->clearColor(Color4f(0,0,0));
			//}
			
			visitView(v);
			
			_mark_recursive = 0;
		}
	}

	void RenderBackend::visitFloatLayout(FloatLayout* flow) {
		// TODO ...
	}

	void RenderBackend::visitFlowLayout(FlowLayout* flow) {
		// TODO ...
	}

	void RenderBackend::visitFlexLayout(FlexLayout* flex) {
		// TODO ...
	}

}
