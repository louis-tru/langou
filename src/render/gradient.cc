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

#include "./gradient.h"
#include "./paint.h"

namespace quark {

	GradientPaint* GradientPaint::Linear(Array<Color4f>&& colors, Array<float> &&pos, Vec2 start, Vec2 end) {
		auto g = new GradientPaint();
		g->_colors = std::move(colors);
		g->_positions = std::move(pos);
		g->_type = kLinear;
		g->_start = start;
		g->_end = end;
		return g;
	}

	GradientPaint* GradientPaint::Radial(Array<Color4f>&& colors, Array<float> &&pos, Vec2 center, float radial) {
		auto g = new GradientPaint();
		g->_colors = std::move(colors);
		g->_positions = std::move(pos);
		g->_type = kRadial;
		g->_start = center;
		g->_end[0] = radial;
		return g;
	}

	// -------------------------------------------------------

	void test() {
		Paint paint;
		paint.type = Paint::kGradient_Type;
		paint.style = Paint::kFill_Style;
		paint.antiAlias = true;
		// Array<Color4f>&& colors, Array<float> &&pos, Vec2 start, Vec2 end
		Sp<GradientPaint> g = 
		GradientPaint::Linear({Color4f(0,0,0),Color4f(1,1,1)},{0,0.5,1},{0,0},{1,1});
	}

}