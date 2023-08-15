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

// @private head

#ifndef __quark__render_render__
#define __quark__render_render__

#include "../util/json.h"
#include "./math.h"
#include "./source.h"
#include "./canvas.h"
#include "./font/pool.h"

namespace qk {

	/**
	 * @class RenderBackend drawing device backend
	 * @thread render
	 */
	class Qk_EXPORT RenderBackend: public Object, public PostMessage {
	public:
		struct Options {
			ColorType   colorType;
			uint32_t    msaaSampleCnt; // gpu msaa
		};
		class Delegate {
		public:
			virtual bool onRenderBackendReload(Region region, Vec2 size,
								float defaultScale, Mat4 *surfaceMat, Vec2* surfaceScale) = 0;
			virtual bool onRenderBackendPreDisplay() = 0;
			virtual void onRenderBackendDisplay() = 0;
		};

		static  RenderBackend* Make(Options opts, Delegate *delegate);
		virtual        ~RenderBackend();
		virtual void    reload() = 0; // surface size and scale change
		virtual void    begin() = 0; // start render task
		virtual void    submit() = 0; // submit render task
		virtual void    activate(bool isActive);
		inline  Canvas* getCanvas() { return _canvas; } // default canvas object
		inline  Vec2    surfaceSize() { return _surfaceSize; }
		inline  float   defaultScale() { return _defaultScale; }
		inline  Delegate* delegate() { return _delegate; }
		virtual uint32_t makeTexture(cPixel *src, uint32_t id) = 0;
		virtual void    deleteTextures(const uint32_t *ids, uint32_t count) = 0;
		virtual void    makeVertexData(VertexData *data) = 0;
		virtual void    deleteVertexData(const VertexData &data) = 0;
		virtual void    copyVertexData(const VertexData &src, VertexData *dest) = 0;

		// @overwrite class PostMessage
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;

		/**
		 * @dev get normalized path cache
		 */
		const Path& getNormalizedPath(const Path &path);

		/**
		 * @dev get stroke path cache
		 */
		const Path& getStrokePath(const Path &path,
			float width, Path::Cap cap, Path::Join join, float miterLimit
		);

		/**
		 * @dev get path triangles cache
		*/
		const VertexData& getPathTriangles(const Path &path);

		/**
		 * @dev get sdf stroke path triangle strip cache
		*/
		const VertexData& getSDFStrokeTriangleStrip(const Path &path, float width);

		/**
		 * @dev get radius rect path cache from hash code
		*/
		const RectPath* getRRectPathFromHash(uint64_t hash);

		/**
		 * @dev set radius rect path cache from hash code
		*/
		const RectPath& setRRectPathFromHash(uint64_t hash, RectPath&& rect);

		/**
		 * @dev get radius rect outline path cache from hash code
		*/
		const RectOutlinePath* getRRectOutlinePathFromHash(uint64_t hash);

		/**
		 * @dev set rect outline path cache from hash code
		*/
		const RectOutlinePath& setRRectOutlinePathFromHash(uint64_t hash, RectOutlinePath&& outline);

		/**
		 * @dev get rect path cache
		 */
		const RectPath& getRectPath(const Rect &rect);

		/**
		 * @dev get radius rect path cache
		 * @param rect {Rect} rect
		 * @param radius {BorderRadius} border radius
		*/
		const RectPath& getRRectPath(const Rect &rect, const Path::BorderRadius &radius);

		/**
		 * @dev get radius rect path cache and limit radius size
		 * @param rect {Rect} rect
		 * @param radius {float[4]} border radius leftTop,rightTop,rightBottom,leftBottom
		*/
		const RectPath& getRRectPath(const Rect &rect, const float radius[4]);

		/**
		 * @dev get radius rect outline path cache and limit radius size
		 * @param rect {Rect} outside border rect
		 * @param border {float[4]} inside border width top,right,bottom,left
		 * @param radius {float[4]} outside border radius leftTop,rightTop,rightBottom,leftBottom
		 */
		const RectOutlinePath& getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4]);

	protected:
		RenderBackend(Options opts);
		virtual Vec2  getSurfaceSize() = 0;
		virtual float getDefaultScale() = 0;
		Options       _opts;
		Canvas       *_canvas; // default canvas
		Delegate     *_delegate;
		Vec2          _surfaceSize; // current surface size
		float         _defaultScale; // recommend default surface scale
		Dict<uint64_t, Path> _NormalizedPathCache, _StrokePathCache; // path hash => path
		Dict<uint64_t, VertexData> _PathTrianglesCache; // path hash => triangles
		Dict<uint64_t, VertexData> _SDFStrokeTriangleStripCache; // path hash => aa triangles strip
		Dict<uint64_t, RectPath> _RectPathCache; // rect hash => rect path
		Dict<uint64_t, RectOutlinePath> _RectOutlinePathCache; // rect hash => rect outline path
	};

	typedef RenderBackend Render;
}
#endif
