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

#include "tesselator.h"
#include "./path.h"
#include "./bezier.h"
#include "../util/handle.h"
#include <math.h>

namespace qk {

	Path Path::Oval(const qk::Rect& r) {
		Path path;
		path.ovalTo(r);
		path.close();
		return std::move(path);
	}

	Path Path::Arc(const qk::Rect& r, float startAngle, float sweepAngle, bool useCenter) {
		Path path;
		path.arcTo(r, startAngle, sweepAngle, useCenter);
		path.close();
		return std::move(path);
	}

	Path Path::Rect(const qk::Rect& r) {
		Path path;
		path.rectTo(r);
		path.close();
		return std::move(path);
	}

	Path Path::Circle(Vec2 center, float radius) {
		return Oval({ Vec2(center.x() - radius, center.y() - radius), Vec2(radius) * 2 });
	}

	Path::Path(Vec2 move): _IsNormalized(true) {
		moveTo(move);
	}

	Path::Path(Vec2* pts, int len, PathVerb* verbs, int verbsLen): _IsNormalized(false) {
		// Qk_ASSERT(verbs[0] == kVerb_Move);
		_pts.write((float*)pts, -1, len * 2);
		_verbs.write((uint8_t*)verbs, -1, verbsLen);
	}

	Path::Path(): _IsNormalized(true) {}

	void Path::moveTo(Vec2 to) {
		// _pts.push(to.x()); _pts.push(to.y());
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Move);
		_hash.update((uint32_t*)&to, 2);
	}

	void Path::lineTo(Vec2 to) {
		// _pts.push(to);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Line);
		_hash.update((uint32_t*)&to, 2);
	}

	void Path::quadTo(Vec2 control, Vec2 to) {
		_pts.write(control.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
		// _hash.update((&_pts.back()) - 4, sizeof(float) * 4);
		_hash.update((uint32_t*)(&_pts.back()) - 4, 4);
	}

	void Path::cubicTo(Vec2 control1, Vec2 control2, Vec2 to) {
		//_pts.push(control1[0]); _pts.push(control1[1]);
		//_pts.push(control2[0]); _pts.push(control2[1]);
		//_pts.push(to[0]); _pts.push(to[1]);
		_pts.write(control1.val, -1, 2);
		_pts.write(control2.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Cubic);
		_IsNormalized = false;
		_hash.update((uint32_t*)(&_pts.back()) - 6, 6);
	}

	constexpr float magicCircle = 0.551915024494f; // 0.552284749831f

	void Path::ovalTo(const qk::Rect& r) {
		float w = r.size.x(), h = r.size.y();
		float x = r.origin.x(), y = r.origin.y();
		float x2 = x + w / 2, y2 = y + h / 2;
		float x3 = x + w, y3 = y + h;
		float cx = w / 2 * magicCircle, cy = h / 2 * magicCircle;
		moveTo(Vec2(x2, y));
		float a[] = {x2 + cx, y, x3, y2 - cy, x3, y2}; cubicTo2(a); // top,right
		float b[] = {x3, y2 + cy, x2 + cx, y3, x2, y3}; cubicTo2(b); // right,bottom
		float c[] = {x2 - cx, y3, x, y2 + cy, x, y2}; cubicTo2(c); // bottom,left
		float d[] = {x, y2 - cy, x2 - cx, y, x2, y}; cubicTo2(d); // left,top
	}

	void Path::rectTo(const qk::Rect& r) {
		moveTo(r.origin);
		float x2 = r.origin.x() + r.size.x();
		float y2 = r.origin.y() + r.size.y();
		lineTo(Vec2(x2, r.origin.y()));
		lineTo(Vec2(x2, y2));
		lineTo(Vec2(r.origin.x(), y2));
		lineTo(r.origin); // origin point
	}

	void Path::arcTo(const qk::Rect& r, float startAngle, float sweepAngle, bool useCenter) {

		float rx = r.size.x() * 0.5f;
		float ry = r.size.y() * 0.5f;
		float cx = r.origin.x() + rx;
		float cy = r.origin.y() + ry;

		float n = ceilf(abs(sweepAngle) / Qk_PI2);
		float sweep = sweepAngle / n;
		float magic = abs(sweep) == Qk_PI2 ? 
			magicCircle: tanf(sweep / 4.0f) * 1.3333333333333333f/*4.0 / 3.0*/;

		startAngle = -startAngle;

		float x0 = cosf(startAngle);
		float y0 = sinf(startAngle);

		Vec2 start(x0 * rx + cx, y0 * ry + cy);

		if (useCenter) {
			moveTo(Vec2(cx, cy));
			lineTo(start);
		} else {
			moveTo(start);
		}

		for (int i = 0, j = n; i < j; i++) {
			startAngle -= sweep;
			float x3 = cosf(startAngle);
			float y3 = sinf(startAngle);
			float x1 = x0 + magic * y0;
			float y1 = y0 - magic * x0;
			float x2 = x3 - magic * y3;
			float y2 = y3 + magic * x3;
			float pts[] = {
				x1 * rx + cx, y1 * ry + cy, // p1
				x2 * rx + cx, y2 * ry + cy, // p2
				x3 * rx + cx, y3 * ry + cy  // p3
			};
			cubicTo2(pts);
			x0 = x3;
			y0 = y3;
		}

		if (useCenter) {
			lineTo(Vec2(cx, cy));
		}
	}

	void Path::quadTo2(float *p) {
		_pts.write(p, -1, 4);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
		_hash.update((uint32_t*)p, 4);
	}

	void Path::cubicTo2(float *p) {
		_pts.write(p, -1, 6);
		_verbs.push(kVerb_Cubic);
		_IsNormalized = false;
		_hash.update((uint32_t*)p, 6);
	}

	void Path::close() {
		_verbs.push(kVerb_Close);
	}

	Array<Vec2> Path::getPolygons(int polySize, float epsilon) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp, false,epsilon);
		auto tess = tessNewTess(nullptr); // TESStesselator*
		auto pts = (const Vec2*)*self->_pts;
		int len = 0;
		Array<Vec2> polygons,tmpV;

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kVerb_Move:
					if (len > 1) { // auto close
						tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
					}
					tmpV.push(*pts++); len = 1;
					break;
				case kVerb_Line:
					if (len == 0) {
						tmpV.push(Vec2(0)); len=1; // use Vec2(0,0) start point
					}
					tmpV.push(*pts++); len++;
					break;
				default: // close
					Qk_ASSERT(verb == kVerb_Close);
					if (len) {
						tmpV.push(tmpV[tmpV.length() - len++]);
						tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
						len = 0;
					}
					break;
			}
		}

		if (len > 1) { // auto close
			tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
		}

		// Convert to convex contour vertex data
		if ( tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, polySize, 2, 0) ) {

			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const Vec2* verts = (const Vec2*)tessGetVertices(tess);

			polygons.extend(nelems * polySize);

			for (int i = 0; i < polygons.length(); i++) {
				polygons[i] = verts[*elems++];
			}
		}

		tessDeleteTess(tess);

		return std::move(polygons);
	}

	Array<Vec3> Path::getEdgeLines(bool close, bool girth, float epsilon) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp, false,epsilon);
		Array<Vec3> edges;
		auto pts = (const Vec2*)*self->_pts;
		int  len = 0;
		Vec3 prev;
		
		auto closeLine = [&]() {
			auto vertex = edges[edges.length() - len].xy();
			edges.push(prev);
			edges.push(Vec3(vertex, girth ? (vertex - prev.xy()).distance() + prev.z(): 0));
		};

		for (auto verb: self->_verbs) {
			switch(verb) {
				case kVerb_Move:
					if (len && close)
						closeLine();
					prev = Vec3(*pts++, 0);
					len = 0;
					break;
				case kVerb_Line: {
					edges.push(prev);
					prev = Vec3(*pts, girth ? (prev.xy() - *pts).distance() + prev.z(): 0);
					edges.push(prev); // edge 0
					pts++;
					len+=2;
					break;
				}
				default: // close
					Qk_ASSERT(verb == kVerb_Close);
					if (len) // add close edge line
						closeLine();
					prev = Vec3();
					len = 0;
					break;
			}
		}
		
		if (len && close)
			closeLine();

		return edges;
	}

	Path Path::strokePath(float width, Join join, float offset) const {
		Path tmp;
		const Path *self = _IsNormalized ? this: normalized(&tmp, false, 1);

		// TODO ...
		return *this;
	}

	Path Path::normalizedPath(float epsilon) const {
		if (_IsNormalized)
			return *this; // copy self
		Path line;
		normalized(&line, true, epsilon);
		return std::move(line);
	}

	Path* Path::normalized(Path *out, bool updateHash, float epsilon) const {
		Path &line = *out;
		auto pts = ((const Vec2*)_pts.val());
		bool isZeor = true;
		
		auto add = [&](Vec2 to, PathVerb verb) {
			line._pts.write(to.val, -1, 2);
			line._verbs.push(verb);
			if (updateHash)
				line._hash.update((uint32_t*)&to, 2);
		};

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					add(*pts++, kVerb_Move);
					isZeor = false;
					break;
				case kVerb_Line:
					if (isZeor)
						add(Vec2(), kVerb_Move); // add zeor
					add(*pts++, kVerb_Line);
					isZeor = false;
					break;
				case kVerb_Quad: { // quadratic bezier
					if (isZeor)
						add(Vec2(), kVerb_Move);
					QuadraticBezier bezier(pts[-1], pts[0], pts[1]);
					pts+=2;
					int sample = Path::getQuadraticBezierSample(bezier, epsilon) - 1;
					// |0|1| = sample = 3
					int sampleSize  = sample * 2;
					line._pts.extend(line._pts.length() + sampleSize);
					auto points = &line._pts[line._pts.length() - sampleSize];
					bezier.sample_curve_points(sample+1, points - 2);
					if (updateHash)
						line._hash.update((uint32_t*)points, sampleSize); // update hash
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					isZeor = false;
					break;
				}
				case kVerb_Cubic: { // cubic bezier
					if (isZeor)
						add(Vec2(), kVerb_Move);
					CubicBezier bezier(pts[-1], pts[0], pts[1], pts[2]);
					pts+=3;
					int sample = Path::getCubicBezierSample(bezier, epsilon) - 1;
					// |0|1| = sample = 3
					int sampleSize = sample * 2;
					line._pts.extend(line._pts.length() + sampleSize);
					auto points = &line._pts[line._pts.length() - sampleSize];
					bezier.sample_curve_points(sample+1, points - 2);
					if (updateHash)
						line._hash.update((uint32_t*)points, sampleSize); // update hash
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					isZeor = false;
					break;
				}
				default: // close
					line._verbs.push(kVerb_Close);
					isZeor = true;
					break;
			}
		}
		
		return out;
	}

	void Path::transfrom(const Mat& matrix) {
		float* pts = *_pts;
		float* e = pts + _pts.length();
		while (pts < e) {
			*((Vec2*)pts) = matrix * (*(Vec2*)pts);
			pts += 2;
		}
	}

	void Path::scale(Vec2 scale) {
		float* pts = *_pts;
		float* e = pts + _pts.length();
		while (pts < e) {
			pts[0] *= scale[0];
			pts[1] *= scale[1];
			pts += 2;
		}
	}

	static float sqrt_sqrtf(int i) {
		static Array<float> num;
		if (!num.length()) {
			num.extend(5001);
			uint64_t t = qk::time_monotonic();
			for (int i = 0; i < 5001; i++) {
				num[i] = sqrtf(sqrtf(float(i)));
			}
			Qk_DEBUG("sqrt_sqrtf, %ld", qk::time_monotonic() - t);
		}
		if (i > 5000) {
			return num[5000] * i * 0.0002; // 5000.0
		}
		return num[i];
	}

	int Path::getQuadraticBezierSample(const QuadraticBezier& curve, float epsilon) {
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2();

		// calculate triangle area by point cross multiplication

		float S_ABC = (A.x()*B.y() - A.y()*B.x()) + (B.x()*C.y() - B.y()*C.x()) + (C.x()*A.y() - C.y()*A.x());
		float S_2 = abs(S_ABC); // *0.5

		if (S_2 < 5000.0) {
			constexpr float count = 22.0 / 8.408964152537145;
			int i = Uint32::max(sqrt_sqrtf(S_2) * count * epsilon, 2);
		} else {
			return 22;
		}
	}

	int Path::getCubicBezierSample(const CubicBezier& curve, float epsilon) {
		/*
		function get_cubic_bezier_sample(A, B, C, D, epsilon = 1) {
			let S_ABC = (A.x*B.y - A.y*B.x) + (B.x*C.y - B.y*C.x);
			let S_CDA = (C.x*D.y - C.y*D.x) + (D.x*A.y - D.y*A.x);
			let S = (S_ABC + S_CDA) * 0.5 * epsilon;
			return S;
		}
		console.log(get_cubic_bezier_sample({x:0,y:0}, {x:10,y:0}, {x:10,y:10}, {x:0,y:10}));
		*/
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2(), D = curve.p3();

		// calculate the area of two triangles by point cross multiplication

		float S_ABC = (A.x()*B.y() - A.y()*B.x()) + (B.x()*C.y() - B.y()*C.x());// + (C.x()*A.y() - C.y()*A.x());
		float S_CDA = (C.x()*D.y() - C.y()*D.x()) + (D.x()*A.y() - D.y()*A.x());// + (A.x()*C.y() - A.y()*C.x());
		float S_2 = abs(S_ABC + S_CDA); // S = S_2 * 0.5

		if (S_2 < 5000.0) { // circle radius < 80
			constexpr float count = 30.0 / 8.408964152537145;//sqrtf(sqrtf(5000.0));
			int i = Uint32::max(sqrt_sqrtf(S_2) * count * epsilon, 2);
			return i;
		} else {
			return 30;
		}
	}

}
