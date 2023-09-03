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

#include "./pathv_cache.h"

namespace qk {

	PathvCache::PathvCache(RenderBackend *render): _render(render) {}
	PathvCache::~PathvCache() {
	}

	const Path& PathvCache::getNormalizedPath(const Path &path) {
		if (path.isNormalized()) return path;
		auto hash = path.hashCode();
		Path *const *out;
		if (_NormalizedPathCache.get(hash, out)) return **out;
		if (_NormalizedPathCache.length() >= 1024) _NormalizedPathCache.clear(); // TODO ... release data
		return *_NormalizedPathCache.set(hash, new Path(path.normalizedPath(1)));
	}

	const Path& PathvCache::getStrokePath(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miterLimit) 
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&miterLimit;
		hash += (hash << 5) + hash_part + ((cap << 2) | join);
		Path *const*out;
		if (_StrokePathCache.get(hash, out)) **out;
		if (_StrokePathCache.length() >= 1024) _StrokePathCache.clear(); // TODO ... release data
		auto stroke = path.strokePath(width,cap,join,miterLimit);
		return *_StrokePathCache.set(hash,
			new Path(stroke.isNormalized() ? std::move(stroke): stroke.normalizedPath(1))
		);
	}

	const VertexData& PathvCache::getPathTriangles(const Path &path) {
		auto hash = path.hashCode();
		GpuBuffer<VertexData> *const*out;
		if (_PathTrianglesCache.get(hash, out)) return (*out)->base;
		if (_PathTrianglesCache.length() >= 1024) { // TODO ... release data
			_PathTrianglesCache.clear();
		}
		auto gb = new GpuBuffer<VertexData>{{size_t(this),path.getTriangles(1).vertex},0,0};
		return _PathTrianglesCache.set(hash, gb)->base;
	}

	const VertexData& PathvCache::getAAFuzzStrokeTriangle(const Path &path, float width) {
		auto hash = path.hashCode();
		hash += (hash << 5) + *(int32_t*)&width;
		//Qk_DEBUG("getAAFuzzTriangle, %lu", hash);
		GpuBuffer<VertexData> *const *out;
		if (_AAFuzzStrokeTriangleCache.get(hash, out)) return (*out)->base;
		if (_AAFuzzStrokeTriangleCache.length() >= 1024) { // TODO ... release data
			_AAFuzzStrokeTriangleCache.clear();
		}
		auto gb = new GpuBuffer<VertexData>{{size_t(this),path.getAAFuzzStrokeTriangle(width, 1).vertex},0,0};
		return _AAFuzzStrokeTriangleCache.set(hash, gb)->base;
	}

	const RectPath& PathvCache::setRRectPathFromHash(uint64_t hash, RectPath&& rect) {
		if (_RectPathCache.length() >= 1024) _RectPathCache.clear(); // TODO ... release data
		rect.id = size_t(this);
		auto gb = new GpuBuffer<RectPath>{std::move(rect),0,0};
		return _RectPathCache.set(hash, gb)->base;
	}

	const RectOutlinePath& PathvCache::setRRectOutlinePathFromHash(uint64_t hash, RectOutlinePath&& outline) {
		if (_RectOutlinePathCache.length() >= 1024) {
			_RectPathCache.clear(); // TODO ... release data
		}
		outline.top.id = size_t(this);
		outline.right.id = size_t(this);
		outline.bottom.id = size_t(this);
		outline.left.id = size_t(this);
		auto gb = new GpuBuffer<RectOutlinePath,4>{std::move(outline),{0,0,0,0},{0,0,0,0}};
		return _RectOutlinePathCache.set(hash, gb)->base;
	}

	const RectPath* PathvCache::getRRectPathFromHash(uint64_t hash) {
		GpuBuffer<RectPath> *const *out;
		if (_RectPathCache.get(hash, out)) return &(*out)->base;
		return nullptr;
	}

	const RectOutlinePath* PathvCache::getRRectOutlinePathFromHash(uint64_t hash) {
		GpuBuffer<RectOutlinePath,4> *const *out;
		if (_RectOutlinePathCache.get(hash, out)) return &(*out)->base;
		return nullptr;
	}

	const RectPath& PathvCache::getRectPath(const Rect &rect) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		GpuBuffer<RectPath> *const *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return (*out)->base;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
	}

	const RectPath& PathvCache::getRRectPath(const Rect &rect, const Path::BorderRadius &radius) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius.leftTop.val);
		hash.updatefv4(radius.rightBottom.val);
		GpuBuffer<RectPath> *const *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return (*out)->base;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, radius));
	}

	const RectPath& PathvCache::getRRectPath(const Rect &rect, const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius);
		GpuBuffer<RectPath> *const *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return (*out)->base;

		if (*reinterpret_cast<const uint64_t*>(radius) == 0 && *reinterpret_cast<const uint64_t*>(radius+2) == 0)
		{
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
		} else {
			float xy_0_5 = Float::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius Br{
				Qk_MIN(radius[0], xy_0_5), Qk_MIN(radius[1], xy_0_5),
				Qk_MIN(radius[2], xy_0_5), Qk_MIN(radius[3], xy_0_5),
			};
			auto rectv = RectPath::MakeRRect(rect, Br);
			rectv.id = size_t(this);
			return setRRectPathFromHash(hash.hashCode(), std::move(rectv));
		}
	}

	const RectOutlinePath& PathvCache::getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(border);
		hash.updatefv4(radius);
		GpuBuffer<RectOutlinePath,4> *const *out;
		if (_RectOutlinePathCache.get(hash.hashCode(), out)) return (*out)->base;

		if (*reinterpret_cast<const uint64_t*>(radius) == 0 && *reinterpret_cast<const uint64_t*>(radius+2) == 0)
		{
				return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRectOutline(rect, border));
		} else {
			float xy_0_5 = Float::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius Br{
				{Qk_MIN(radius[0],xy_0_5)}, {Qk_MIN(radius[1],xy_0_5)},
				{Qk_MIN(radius[2],xy_0_5)}, {Qk_MIN(radius[3],xy_0_5)},
			};
			return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRRectOutline(rect, border, Br));
		}
	}

}