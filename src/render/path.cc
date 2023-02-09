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
#include "../util/handle.h"
#include "../bezier.h"
#include <math.h>

namespace quark {

	Path Path::Oval(const quark::Rect& r) {
		Path path;
		path.oval_to(r);
		path.close();
		return std::move(path);
	}

	Path Path::Arc(const quark::Rect& r, float startAngle, float sweepAngle, bool useCenter) {
		Path path;
		path.arc_to(r, startAngle, sweepAngle, useCenter);
		path.close();
		return std::move(path);
	}

	Path Path::Rect(const quark::Rect& r) {
		Path path;
		path.rect_to(r);
		path.close();
		return std::move(path);
	}

	Path Path::Circle(Vec2 center, float radius) {
		return Oval({ Vec2(center.x() - radius, center.y() - radius), Vec2(radius) * 2 });
	}

	Path::Path(Vec2 move): _IsNormalized(true) {
		move_to(move);
	}

	Path::Path(Vec2* pts, int len, PathVerb* verbs, int verbsLen): _IsNormalized(false) {
		// Qk_ASSERT(verbs[0] == kVerb_Move);
		_pts.write((float*)pts, -1, len * 2);
		_verbs.write((uint8_t*)verbs, -1, verbsLen);
	}

	Path::Path(): _IsNormalized(true) {}

	void Path::move_to(Vec2 to) {
		// _pts.push(to.x()); _pts.push(to.y());
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Move);
	}

	void Path::line_to(Vec2 to) {
		// _pts.push(to);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Line);
	}

	void Path::quad_to(Vec2 control, Vec2 to) {
		_pts.write(control.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
	}

	void Path::cubic_to(Vec2 control1, Vec2 control2, Vec2 to) {
		//_pts.push(control1[0]); _pts.push(control1[1]);
		//_pts.push(control2[0]); _pts.push(control2[1]);
		//_pts.push(to[0]); _pts.push(to[1]);
		_pts.write(control1.val, -1, 6);
		_pts.write(control2.val, -1, 2);
		_pts.write(to.val, -1, 2);
		_verbs.push(kVerb_Cubic);
		_IsNormalized = false;
	}

	constexpr float magicCircle = 0.551915024494f; // 0.552284749831f

	void Path::oval_to(const quark::Rect& r) {
		float w = r.size.x(), h = r.size.y();
		float x = r.origin.x(), y = r.origin.y();
		float x2 = x + w / 2, y2 = y + h / 2;
		float x3 = x + w, y3 = y + h;
		float cx = w / 2 * magicCircle, cy = h / 2 * magicCircle;
		move_to(Vec2(x2, y));
		float a[] = {x2 + cx, y, x3, y2 - cy, x3, y2}; cubic_to2(a); // top,right
		float b[] = {x3, y2 + cy, x2 + cx, y3, x2, y3}; cubic_to2(b); // right,bottom
		float c[] = {x2 - cx, y3, x, y2 + cy, x, y2}; cubic_to2(c); // bottom,left
		float d[] = {x, y2 - cy, x2 - cx, y, x2, y}; cubic_to2(d); // left,top
	}

	void Path::rect_to(const quark::Rect& r) {
		move_to(r.origin);
		float x2 = r.origin.x() + r.size.x();
		float y2 = r.origin.y() + r.size.y();
		line_to(Vec2(x2, r.origin.y()));
		line_to(Vec2(x2, y2));
		line_to(Vec2(r.origin.x(), y2));
		line_to(r.origin); // origin point
	}

	void Path::arc_to(const quark::Rect& r, float startAngle, float sweepAngle, bool useCenter) {

		float rx = r.size.x() / 2.0f;
		float ry = r.size.y() / 2.0f;
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
			move_to(Vec2(cx, cy));
			line_to(start);
		} else {
			move_to(start);
		}

		for (int i = 0; i < n; i++) {
			startAngle -= sweep;
			float x3 = cosf(startAngle);
			float y3 = sinf(startAngle);
			float x1 = x0 - magic * y0;
			float y1 = y0 + magic * x0;
			float x2 = x3 + magic * y3;
			float y2 = y3 - magic * x3;
			float pts[] = {
				x1 * rx + cx, y1 * ry + cy, // p1
				x2 * rx + cx, y2 * ry + cy, // p2
				x3 * rx + cx, y3 * ry + cy  // p3
			};
			cubic_to2(pts);
		}

		if (useCenter) {
			line_to(Vec2(cx, cy));
		}
	}

	void Path::quad_to2(float *p) {
		_pts.write(p, -1, 4);
		_verbs.push(kVerb_Quad);
		_IsNormalized = false;
	}

	void Path::cubic_to2(float *p) {
		_pts.write(p, -1, 6);
		_verbs.push(kVerb_Cubic);
		_IsNormalized = false;
	}

	void Path::close() {
		_verbs.push(kVerb_Close);
	}

	Array<Vec3> Path::to_polygons(int polySize, bool antiAlias, float epsilon) const {
		TESStesselator* tess = tessNewTess(nullptr);
		CPointer<TESStesselator> hold(tess, [](TESStesselator*p) { tessDeleteTess(p); });

		const Vec2* pts = (const Vec2*)*_pts;
		Array<Vec3> polygons;
		Array<Vec2> tmpV;
		int len = 0;

		for (auto verb: _verbs) {
			if (len == 0 && verb != kVerb_Move) {
				tmpV.push(Vec2(0)); // use Vec2(0,0) start point
				len++;
			}

			switch(verb) {
				case kVerb_Move:
					if (len > 1) {
						tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
						len = 1;
					}
					tmpV.push(*pts++);
					break;
				case kVerb_Line:
					tmpV.push(*pts++);
					len++;
					break;
				case kVerb_Quad: { // quadratic
					// Qk_DEBUG("conic_to:%f,%f|%f,%f", pts[0].x(), pts[0].y(), pts[1].x(), to[1].y());
					QuadraticBezier bezier(tmpV.back(), pts[0], pts[1]);
					pts+=2;
					int sample = Path::get_quadratic_bezier_sample(bezier, epsilon);
					tmpV.extend(tmpV.length() + sample - 1);
					bezier.sample_curve_points(sample, (float*)&tmpV[tmpV.length() - sample]);
					len += sample - 1;
					break;
				}
				case kVerb_Cubic: {// cubic
					//  Qk_DEBUG("cubic_to:%f,%f|%f,%f|%f,%f",
					//           pts[0].x(), pts[0].y(), pts[1].x(), to[1].y(), pts[2].x(), to[2].y());
					CubicBezier bezier(tmpV.back(), pts[0], pts[1], pts[2]);
					pts+=3;
					int sample = Path::get_cubic_bezier_sample(bezier, epsilon);
					tmpV.extend(tmpV.length() + sample - 1);
					bezier.sample_curve_points(sample, (float*)&tmpV[tmpV.length() - sample]);
					len += sample - 1;
					break;
				}
				default: // close
					if (len) {
						tmpV.push(tmpV[tmpV.length() - len++]);
						tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
						len = 0;
					}
					break;
			}
		}

		if (len > 1) { // closure
			tessAddContour(tess, 2, (float*)&tmpV[tmpV.length() - len], sizeof(Vec2), len);
		}

		// Convert to convex contour vertex data
		if ( tessTesselate(tess, TESS_WINDING_POSITIVE,
											TESS_CONNECTED_POLYGONS/*TESS_POLYGONS*/, polySize, 2, 0) 
		) {
			const int nelems = tessGetElementCount(tess);
			const TESSindex* elems = tessGetElements(tess);
			const TESSreal* verts = tessGetVertices(tess);
			for (int i = 0; i < nelems * polySize; i++) {
				float x = verts[elems[0]];
				float y = verts[elems[1]];
				polygons.push(Vec3(x, y, 1.0));
				elems += 2;
			}
		}

		return std::move(polygons);
	}

	Array<Vec2> Path::to_edge_lines(float epsilon) const {
		const Vec2* pts = ((const Vec2*)*_pts) - 1;
		Array<Vec2> edges;
		int len = 0;
		bool isZeor = true;

		for (auto verb: _verbs) {

			switch(verb) {
				case kVerb_Move:
					pts++;
					len = 0;
					isZeor = false;
					break;
				case kVerb_Line:
					edges.push(isZeor ? (pts++, Vec2()): *pts++); edges.push(*pts); // edge 0
					len+=2;
					isZeor = false;
					break;
				case kVerb_Quad: { // Quadratic
					//  Qk_DEBUG("conic_to:%f,%f|%f,%f", pts[0].x(), pts[0].y(), pts[1].x(), to[1].y());
					QuadraticBezier bezier(isZeor ? Vec2(): pts[0], pts[1], pts[2]); pts+=2;
					int sample = Path::get_quadratic_bezier_sample(bezier, epsilon);
					auto points = bezier.sample_curve_points(sample);
					for (int i = 0; i < sample - 1; i++) {
						edges.push(points[i]); edges.push(points[i + 1]); // add edge line
					}
					len += (sample * 2 - 2);
					isZeor = false;
					break;
				}
				case kVerb_Cubic: { // cubic
					//  Qk_DEBUG("cubic_to:%f,%f|%f,%f|%f,%f",
					//           pts[0].x(), pts[0].y(), pts[1].x(), to[1].y(), pts[2].x(), to[2].y());
					CubicBezier bezier(isZeor ? Vec2(): pts[0], pts[1], pts[2], pts[3]); pts+=3;
					int sample = Path::get_cubic_bezier_sample(bezier, epsilon);
					auto points = bezier.sample_curve_points(sample);
					for (int i = 0; i < sample - 1; i++) {
						edges.push(points[i]); edges.push(points[i + 1]); // add edge line
					}
					len += (sample * 2 - 2);
					isZeor = false;
					break;
				}
				default: // close
					if (len) {
						edges.push(*pts); edges.push(edges[edges.length() - len - 1]); // add close edge line
					}
					len = 0;
					isZeor = true;
					break;
			}
		}

		return edges;
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

	Path Path::normalized(float epsilon) const {
		if (_IsNormalized)
			return *this; // copy self

		const Vec2* pts = ((const Vec2*)*_pts);
		Path line;
		bool isZeor = true;

		for (auto verb: _verbs) {
			switch(verb) {
				case kVerb_Move:
					line.move_to(*pts++);
					isZeor = false;
					break;
				case kVerb_Line:
					if (isZeor)
						line.move_to(Vec2()); // add zeor
					line.line_to(*pts++);
					isZeor = false;
					break;
				case kVerb_Quad: { // quadratic bezier
					if (isZeor)
						line.move_to(Vec2());
					QuadraticBezier bezier(line._pts.back(), pts[0], pts[1]);
					pts+=2;
					int sample = Path::get_quadratic_bezier_sample(bezier, epsilon) - 1;
					line._pts.extend(line._pts.length() + sample * 2);
					bezier.sample_curve_points(sample+1, &line._pts[line._pts.length() - (sample+1) * 2]);
					line._verbs.extend(line._verbs.length() + sample);
					memset(line._verbs.val() + (line._verbs.length() - sample), kVerb_Line, sample);
					isZeor = false;
					break;
				}
				case kVerb_Cubic: { // cubic bezier
					if (isZeor)
						line.move_to(Vec2());
					CubicBezier bezier(line._pts.back(), pts[0], pts[1], pts[2]);
					pts+=3;
					int sample = Path::get_cubic_bezier_sample(bezier, epsilon) - 1;
					line._pts.extend(line._pts.length() + sample * 2);
					bezier.sample_curve_points(sample+1, &line._pts[line._pts.length() - (sample+1) * 2]);
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

		return std::move(line);
	}

	Path Path::clip(const Path& path) const {
		// TODO ...
		auto path0 = path.normalized();
		auto path1 = normalized();
		return Path();
	}

	Path Path::genStrokePath(float width, Join join, bool containFill, StrokeMode mode) const {
		// TODO ...
		return *this;
	}

	int Path::get_quadratic_bezier_sample(const QuadraticBezier& curve, float epsilon) {
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2();

		float S_ABC = (A.x()*B.y() - A.y()*B.x()) + (B.x()*C.y() - B.y()*C.x()) + (C.x()*A.y() - C.y()*A.x());
		float S = S_ABC * 0.5 * epsilon;

		if (S < 10000.0) { // < 100^2
			constexpr float count = (16.0 - 2.0) / 10000.0;
			return int(S * count) + 2;
		} else {
			return 16;
		}
	}

	/*
	function get_cubic_bezier_sample(A, B, C, D, epsilon = 1) {
		let S_ABC = (A.x*B.y - A.y*B.x) + (B.x*C.y - B.y*C.x);
		let S_CDA = (C.x*D.y - C.y*D.x) + (D.x*A.y - D.y*A.x);
		let S = (S_ABC + S_CDA) * 0.5 * epsilon;
		return S;
	}
	console.log(get_cubic_bezier_sample({x:0,y:0}, {x:10,y:0}, {x:10,y:10}, {x:0,y:10}));
	*/

	int Path::get_cubic_bezier_sample(const CubicBezier& curve, float epsilon) {
		Vec2 A = curve.p0(), B = curve.p1(), C = curve.p2(), D = curve.p3();

		float S_ABC = (A.x()*B.y() - A.y()*B.x()) + (B.x()*C.y() - B.y()*C.x());// + (C.x()*A.y() - C.y()*A.x());
		float S_CDA = (C.x()*D.y() - C.y()*D.x()) + (D.x()*A.y() - D.y()*A.x());// + (A.x()*C.y() - A.y()*C.x());
		float S = (S_ABC + S_CDA) * 0.5 * epsilon;

		if (S < 10000.0) { // < 100^2
			constexpr float count = (20.0 - 2.0) / 10000.0;
			return int(S * count) + 2;
		} else {
			return 20;
		}
	}

}
