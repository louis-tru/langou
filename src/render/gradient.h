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

#ifndef __quark_render_gradient__
#define __quark_render_gradient__

#include "../math.h"

namespace quark {

	class Qk_EXPORT GradientPaint: public Reference {
	public:
		enum GradientType { kLinear, kRadial, /*kConical,*/};

		static GradientPaint* Linear(Array<Color4f>&& colors,
			Array<float> &&pos, Vec2 start, Vec2 end);
		static GradientPaint* Radial(Array<Color4f>&& colors,
			Array<float> &&pos, Vec2 center, Vec2 radius);

		Qk_DEFINE_PROP_GET(GradientType, type);
		Qk_DEFINE_PROP_GET(Vec2, start);
		Qk_DEFINE_PROP_GET(Vec2, end);

		inline const float* range() const { // range, start/end or center/radius
			return reinterpret_cast<const float*>(&_start);
		}
		inline Vec2 center() const { return _start; }
		inline Vec2 radius() const { return _end; }
		inline const Array<Color4f>& colors() const { return _colors; }
		inline const Array<float>&   positions() const { return _positions; }

	private:
		Array<Color4f>   _colors;
		Array<float>     _positions;
	};

}

#endif