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

#ifndef __noug__layout__view__
#define __noug__layout__view__

#include "./layout.h"
#include "../event.h"

namespace noug {

	#define N_Each_View(F) \
		F(View)  F(Box) \
		F(Image) F(Video) F(Scroll) \
		F(Input) F(Text) F(Label) F(Root) F(FlexLayout) F(FlowLayout)

	#define N_Define_View(N) \
	public: \
		friend class SkiaRender; \
		virtual void accept(ViewVisitor *visitor) override { visitor->visit##N(this); } \

	class Action;
	class SkiaRender;

	N_Define_Visitor(View, N_Each_View);

	/**
		* The basic elements of UI tree
		*
		* @class View
		*/
	class N_EXPORT View: public Notification<UIEvent, UIEventName, Layout> {
		N_HIDDEN_ALL_COPY(View);
	public:

		View();

		virtual ~View();

		/**
			*
			* Add a sibling view to the front
			*
			* @func before(view)
			*/
		void before(View* view);

		/**
			*
			* Add a sibling view to the back
			*
			* @func after(view)
			*/
		void after(View* view);

		/**
			* 
			* Append subview to front
			* 
			* @func prepend(child)
			*/
		virtual void prepend(View* child);

		/**
			*
			* Append subview to end
			*
			* @func append(child)
			*/
		virtual void append(View* child);

		/**
		 * Append subview to parent
		 *
		 * @func append_to(parent)
		 */
		View* append_to(View* parent);

		/**
			*
			* Remove and destroy self
			* 
			* @func remove()
			*/
		virtual void remove();

		/**
			*
			* remove all subview
			*
			* @func remove_all_child()
			*/
		virtual void remove_all_child();

		/**
			* 
			* Setting the visibility properties the view object
			*
			* @func set_visible(val)
			*/
		virtual void set_visible(bool val);

		/**
			* 
			* focus keyboard
			*
			* @func focus()
			*/
		bool focus();
		
		/**
			*
			* Unfocus keyboard
			*
			* @func blur()
			*/
		bool blur();
		
		/**
			*
			* get is keyboard focus 
			*
			* @func is_focus()
			*/
		bool is_focus() const;

		/**
			*
			* setting focus value
			*
			* @func set_is_focus(value)
			*/
		void set_is_focus(bool value);
		
		/**
			*
			* Can it be the focus
			* 
			* @func can_become_focus()
			*/
		virtual bool can_become_focus();

		/**
		 * @overwrite
		 */
		virtual void trigger_listener_change(const NameType& name, int count, int change);

		/**
		 * @func has_child(child)
		 */
		bool has_child(View* child);
		
		/**
			* Returns matrix displacement for the view
			*
			* @func translate
			*/
		Vec2 translate() const;

		/**
			* Returns the Matrix scaling
			*
			* @func scale()
			*/
		Vec2 scale() const;

		/**
			* Returns the Matrix skew
			*
			* @func skew()
			*/
		Vec2 skew() const;

		/**
			* Returns the z-axis rotation of the matrix
			*
			* @func rotate()
			*/
		float rotate() const;

		/**
			* 
			* Returns x-axis matrix displacement for the view
			*
			* @func x()
			*/
		inline float x() const { return translate()[0]; }

		/**
			* 
			* Returns y-axis matrix displacement for the view
			*
			* @func y()
			*/
		inline float y() const { return translate()[1]; }

		/**
			* 
			* Returns x-axis matrix scaling for the view
			*
			* @func scale_x()
			*/
		inline float scale_x() const { return scale()[0]; }

		/**
			* 
			* Returns y-axis matrix scaling for the view
			*
			* @func scale_y()
			*/
		inline float scale_y() const { return scale()[1]; }

		/**
			* 
			* Returns x-axis matrix skew for the view
			*
			* @func skew_x()
			*/
		inline float skew_x() const { return skew()[0]; }

		/**
			* 
			* Returns y-axis matrix skew for the view
			*
			* @func skew_y()
			*/
		inline float skew_y() const { return skew()[1]; }

		/**
			* Set the matrix `translate` properties of the view object
			*
			* @func set_translate(val)
			*/
		void set_translate(Vec2 val);

		/**
			* Set the matrix `scale` properties of the view object
			*
			* @func set_scale(val)
			*/
		void set_scale(Vec2 val);

		/**
			* Set the matrix `skew` properties of the view object
			*
			* @func set_skew(val)
			*/
		void set_skew(Vec2 val);

		/**
			* Set the z-axis  matrix `rotate` properties the view object
			*
			* @func set_rotate(val)
			*/
		void set_rotate(float val);

		/**
			* 
			* Setting x-axis matrix displacement for the view
			*
			* @func set_x(val)
			*/
		void set_x(float val);

		/**
			* 
			* Setting y-axis matrix displacement for the view
			*
			* @func set_y(val)
			*/
		void set_y(float val);

		/**
			* 
			* Returns x-axis matrix scaling for the view
			*
			* @func set_scale_x(val)
			*/
		void set_scale_x(float val);

		/**
			* 
			* Returns y-axis matrix scaling for the view
			*
			* @func set_scale_y(val)
			*/
		void set_scale_y(float val);

		/**
			* 
			* Returns x-axis matrix skew for the view
			*
			* @func set_skew_x(val)
			*/
		void set_skew_x(float val);

		/**
			* 
			* Returns y-axis matrix skew for the view
			*
			* @func set_skew_y(val)
			*/
		void set_skew_y(float val);

		/**
			* 
			* Returns layout transformation matrix of the object view
			* 
			* Mat(layout_offset + transform_origin? + translate + parent->layout_offset_inside, scale, rotate, skew)
			* 
			* @func layout_matrix()
			*/
		virtual Mat layout_matrix();

		/**
			* 
			* Returns final transformation matrix of the view layout
			*
			* parent.matrix * layout_matrix
			* 
			* @func matrix()
			*/
		inline const Mat& matrix() const {
			return _matrix;
		}

		/**
		 * @func solve_recursive_marks(mark)
		*/
		void solve_recursive_marks(uint32_t mark);

		/**
			* @func solve_visible_region()
			*/
		virtual bool solve_visible_region();

		/**
		* @func overlap_test 重叠测试,测试屏幕上的点是否与视图重叠
		*/
		virtual bool overlap_test(Vec2 point);
		
		/**
		 * @func accept() 定义访问接收器
		 */
		virtual void accept(ViewVisitor *visitor);

		/**
		* @func overlap_test_from_convex_quadrilateral
		*/
		static bool overlap_test_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4], Vec2 point);
		
		/**
		* @func screen_rect_from_convex_quadrilateral
		*/
		static Rect screen_rect_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);
		
		/**
		* @func screen_region_from_convex_quadrilateral
		*/
		static Region screen_region_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);

		/**
			* @overwrite
			*/
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextRows *rows) override;
		virtual void onChildLayoutChange(Layout* child, uint32_t mark) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;

	protected:
		/**
			* @func set_parent(parent) setting parent view
			*/
		virtual void set_parent(View* parent);

		struct Transform {
			Vec2 translate, scale, skew; // 平移向量, 缩放向量, 倾斜向量
			float rotate; // z轴旋转角度值
		};
		Transform *_transform; // 矩阵变换
		Mat        _matrix; // 父视图矩阵乘以布局矩阵等于最终变换矩阵 (parent.matrix * layout_matrix)

	private:
		void remove_all_child_(); // remove all child views
		void clear(); // Cleaning up associated view information
		void clear_layout_depth(); //  clear layout depth
		void set_layout_depth_(uint32_t depth); // settings depth
		// get transform instance
		Transform* get_transform_instance();

	public:
		// the objects that automatically adjust view properties
		N_DEFINE_PROP(Action*, action); // 在指定的时间内根据动作设定运行连续一系列的动作命令，达到类似影片播放效果
		N_DEFINE_PROP_READ(View*, parent);
		N_DEFINE_PROP_READ(View*, prev);
		N_DEFINE_PROP_READ(View*, next);
		N_DEFINE_PROP_READ(View*, first);
		N_DEFINE_PROP_READ(View*, last);
		// can affect the transparency of subviews
		N_DEFINE_PROP(float, opacity); // 可影响子视图的透明度值
		// 视图是否需要接收或处理系统的事件抛出，大部情况下这些事件都是不需要处理的，这样可以提高整体事件处理效率
		// @prop Does the view need to receive or handle event throws from the system
		N_DEFINE_PROP(bool, receive);
		// 设置视图的可见性，这个值设置为`false`时视图为不可见且不占用任何布局空间
		N_DEFINE_PROP_READ(bool, visible);
		// 这个值与`visible`完全无关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		N_DEFINE_PROP_READ(bool, visible_region);

		N_DEFINE_INLINE_CLASS(Inl);
		N_DEFINE_INLINE_CLASS(InlEvent);

		// friend class
		friend class SkiaRender;
	};

}
#endif