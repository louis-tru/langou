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

#include "../app.h"
#include "../window.h"
#include "./view.h"
#include "../text/text_lines.h"
#include "./root.h"
#include <math.h>

namespace qk {

	// --------------- L a y o u t  V i e w ---------------

	// view private members method
	Qk_DEFINE_INLINE_MEMBERS(View, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<View::Inl*>(self)

		static void set_visible_static(View::Inl* self, bool val, uint32_t layout_depth) {
			self->_visible = val;
			if (self->_parent) {
				self->_parent->onChildLayoutChange(self, kChild_Layout_Visible); // mark parent layout 
			}
			if (val) {
				self->mark(kLayout_Size_Width | kLayout_Size_Height); // reset layout size
			}
			if (layout_depth) {
				self->set_layout_depth_(layout_depth);
			} else {
				self->clear_layout_depth();
			}
		}

		// get transform instance
		Transform* transform_ptr() {
			if (!_transform) {
				_transform = new Transform();
				_transform->scale = Vec2(1);
				_transform->rotate = 0;
			}
			return _transform;
		}

		void remove_all_child_() { // remove all child views
			while (_first) {
				_first->remove();
			}
		}

		void clear() { // Cleaning up associated view information
			if (_parent) {
				/* 当前为第一个子视图 */
				if (_parent->_first == this) {
					_parent->_first = _next;
				} else {
					_prev->_next = _next;
				}
				/* 当前为最后一个子视图 */
				if (_parent->_last == this) {
					_parent->_last = _prev;
				} else {
					_next->_prev = _prev;
				}
			}
		}

		void clear_layout_depth() { //  clear layout depth
			if ( layout_depth() ) {
				blur();
				set_layout_depth(0);
				View *v = _first;
				while ( v ) {
					_inl(v)->clear_layout_depth();
					v = v->_next;
				}
			}
		}

		void set_layout_depth_(uint32_t depth) { // settings depth
			if (_visible) {
				if ( layout_depth() != depth ) {
					set_layout_depth(depth++);
					set_window(_parent->window()); // set pre render

					if ( layout_mark() ) { // remark
						mark(kLayout_None);
					}
					mark_render(kRecursive_Transform);

					View *v = _first;
					while ( v ) {
						_inl(v)->set_layout_depth_(depth);
						v = v->_next;
					}
				}
			} else {
				clear_layout_depth();
			}
		}

	};

	/**
		*
		* Setting parent parent view
		*
		* @method set_parent(parent)
		*/
	void View::set_parent(View* parent) {
		// clear parent
		if (parent != _parent) {
			_this->clear();

			if ( _parent ) {
				_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			} else {
				retain(); // link to parent and retain ref
			}
			_parent = parent;
			_parent->onChildLayoutChange(this, kChild_Layout_Visible); // notice parent layout
			mark(kLayout_Size_Width | kLayout_Size_Height); // mark layout size, reset layout size

			uint32_t depth = parent->layout_depth();
			if (depth) {
				_this->set_layout_depth_(depth + 1);
			} else {
				_this->clear_layout_depth();
			}
		}
	}

	void __View_set_visible(View* self, bool val, uint32_t layout_depth) {
		View::Inl::set_visible_static(_inl(self), val, layout_depth);
	}

	View::View()
		: Notification<UIEvent, UIEventName, Layout>()
		, _action(nullptr), _parent(nullptr)
		, _prev(nullptr), _next(nullptr)
		, _first(nullptr), _last(nullptr)
		, _transform(nullptr), _opacity(1.0)
		, _visible(true)
		, _visible_region(false)
		, _receive(false)
	{
	}

	View::~View() {
		Qk_ASSERT(_parent == nullptr); // 被父视图所保持的对像不应该被析构,这里parent必须为空
		blur();
		set_action(nullptr); // del action
		_this->remove_all_child_(); // 删除子视图
		delete _transform; _transform = nullptr;
	}

	/**
		*
		* Add a sibling view to the front
		*
		* @method before(view)
		*/
	void View::before(View* view) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent) {
				_inl(view)->clear();  // 清除关联
			} else {
				view->set_parent(_parent);
			}
			if (_prev) {
				_prev->_next = view;
			} else { // 上面没有兄弟
				_parent->_first = view;
			}
			view->_prev = _prev;
			view->_next = this;
			_prev = view;
		}
	}

	/**
		*
		* Add a sibling view to the back
		*
		* @method after(view)
		*/
	void View::after(View* view) {
		if (_parent) {
			if (view == this) return;
			if (view->_parent == _parent) {
				_inl(view)->clear(); // 清除关联
			} else {
				view->set_parent(_parent);
			}
			if (_next) {
				_next->_prev = view;
			} else { // 下面没有兄弟
				_parent->_last = view;
			}
			view->_prev = this;
			view->_next = _next;
			_next = view;
		}
	}

	/**
		* 
		* Append subview to front
		* 
		* @method prepend(child)
		*/
	void View::prepend(View* child) {
		if (!is_allow_append_child()) {
			Qk_WARN("Not can allow prepend child view");
			return;
		}
		if (this == child->_parent) {
			_inl(child)->clear();
		} else {
			child->set_parent(this);
		}
		if (_first) {
			child->_prev = nullptr;
			child->_next = _first;
			_first->_prev = child;
			_first = child;
		} else { // 当前还没有子视图
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
	}

	/**
		*
		* Append subview to end
		*
		* @method append(child)
		*/
	void View::append(View* child) {
		if (!is_allow_append_child()) {
			Qk_WARN("Not can allow append child view");
			return;
		}
		if (this == child->_parent) {
			_inl(child)->clear();
		} else {
			child->set_parent(this);
		}
		if (_last) {
			child->_prev = _last;
			child->_next = nullptr;
			_last->_next = child;
			_last = child;
		} else { // 当前还没有子视图
			child->_prev = nullptr;
			child->_next = nullptr;
			_first = child;
			_last = child;
		}
	}

	bool View::is_allow_append_child() {
		return true;
	}

	/**
		*
		* Remove destroy self and child views from parent view
		* 
		* @method remove()
		*/
	void View::remove() {
		if (_parent) {
			blur(); // 辞去焦点
			set_action(nullptr); // del action
			_this->remove_all_child_(); // 删除子视图
			_this->clear();
			// remove_event_listener();
			set_layout_depth(0);
			_parent = _prev = _next = nullptr;
			release(); // Disconnect from parent view strong reference
		}
		else {
			// remove_event_listener();
			set_action(nullptr); // del action
			_this->remove_all_child_(); // 删除子视图
		}
	}

	/**
		*
		* remove all subview
		*
		* @method remove_all_child()
		*/
	void View::remove_all_child() {
		_this->remove_all_child_();
	}

	/**
		* 
		* Setting the visibility properties the view object
		*
		* @method set_visible(val)
		*/
	void View::set_visible(bool val) {
		if (_visible != val) {
			if (_parent) {
				uint32_t depth = _parent->layout_depth();
				View::Inl::set_visible_static(_inl(this), val, depth ? depth + 1: 0);
			} else {
				View::Inl::set_visible_static(_inl(this), val, 0);
			}
		}
	}

	/**
		* 
		* Sets whether the view needs to receive or handle event thlines from the system
		*
		* @method set_receive()
		*/
	void View::set_receive(bool val) {
		_receive = val;
	}

	// *******************************************************************

	/**
		* Set the `action` properties of the view object
		*
		* @method set_action()
		*/
	void View::set_action(Action* val) {
		if (_action != val) {
			// TODO ...
			if (_action) {
				// unload _action
				_action = nullptr;
			}
			if (val) {
				_action = val;
				// load new action
			}
		}
	}

	/**
		* Returns matrix displacement for the view
		*
		* @method translate
		*/
	Vec2 View::translate() const {
		return _transform ? _transform->translate: Vec2();
	}

	/**
		* Returns the Matrix scaling
		*
		* @method scale()
		*/
	Vec2 View::scale() const {
		return _transform ? _transform->scale: Vec2(1);
	}

	/**
		* Returns the Matrix skew
		*
		* @method skew()
		*/
	Vec2 View::skew() const {
		return _transform ? _transform->skew * 57.29577951308232f: Vec2();
	}

	/**
		* Returns the z-axis rotation of the matrix
		*
		* @method rotate()
		*/
	float View::rotate() const {
		return _transform ? _transform->rotate * 57.29577951308232f: 0;
	}

	/**
		* Set the matrix `translate` properties of the view object
		*
		* @method set_translate(val)
		*/
	void View::set_translate(Vec2 val) {
		if (translate() != val) {
			_this->transform_ptr()->translate = val;
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*
		* @method set_scale(val)
		*/
	void View::set_scale(Vec2 val) {
		if (scale() != val) {
			_this->transform_ptr()->scale = val;
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*
		* @method set_skew(val)
		*/
	void View::set_skew(Vec2 val) {
		val *= Qk_PI_RATIO_180;
		if (!_transform || _transform->skew != val) {
			_this->transform_ptr()->skew = val;
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*
		* @method set_rotate(val)
		*/
	void View::set_rotate(float val) {
		val *= Qk_PI_RATIO_180;
		if (!_transform || _transform->rotate != val) {
			_this->transform_ptr()->rotate = val;
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix displacement for the view
		*
		* @method x()
		*/
	float View::x() const { return translate()[0]; }

	/**
		* 
		* Returns y-axis matrix displacement for the view
		*
		* @method y()
		*/
	float View::y() const { return translate()[1]; }

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @method scale_x()
		*/
	float View::scale_x() const { return scale()[0]; }

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @method scale_y()
		*/
	float View::scale_y() const { return scale()[1]; }

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @method skew_x()
		*/
	float View::skew_x() const { return skew()[0]; }

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @method skew_y()
		*/
	float View::skew_y() const { return skew()[1]; }

	/**
		* 
		* Setting x-axis matrix displacement for the view
		*
		* @method set_x(val)
		*/
	void View::set_x(float val) {
		if (translate().x() != val) {
			_this->transform_ptr()->translate.set_x(val);
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Setting y-axis matrix displacement for the view
		*
		* @method set_y(val)
		*/
	void View::set_y(float val) {
		if (translate().y() != val) {
			_this->transform_ptr()->translate.set_y(val);
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @method set_scale_x(val)
		*/
	void View::set_scale_x(float val) {
		if (scale().x() != val) {
			_this->transform_ptr()->scale.set_x(val);
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @method set_scale_y(val)
		*/
	void View::set_scale_y(float val) {
		if (scale().y() != val) {
			_this->transform_ptr()->scale.set_y(val);
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @method set_skew_x(val)
		*/
	void View::set_skew_x(float val) {
		if (skew().x() != val) {
			_this->transform_ptr()->skew.set_x(val);
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @method set_skew_y(val)
		*/
	void View::set_skew_y(float val) {
		if (skew().y() != val) {
			_this->transform_ptr()->skew.set_y(val);
			mark_render(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the `opacity` properties of the view object
		*
		* @method set_opacity(val)
		*/
	void View::set_opacity(float val) {
		if (_opacity != val) {
			_opacity = Qk_MAX(0, Qk_MIN(val, 1));
			mark(kLayout_None); // mark none
		}
	}

	// *******************************************************************

	/**
		* 
		* Returns layout transformation matrix of the object view
		* 
		* Mat(layout_offset + transform_origin + translate + parent->layout_offset_inside, scale, rotate, skew)
		* 
		* @method layout_matrix()
		*/
	Mat View::layout_matrix() {
		if (_transform) {
			return Mat(
				layout_offset() + _transform->translate + _parent->layout_offset_inside(), // translate
				_transform->scale,
				-_transform->rotate,
				_transform->skew
			);
		} else {
			Vec2 translate = layout_offset() + _parent->layout_offset_inside();
			return Mat(
				1, 0, translate.x(),
				0, 1, translate.y()
			);
		}
	}

	// --------------- o v e r w r i t e ---------------

	bool View::layout_forward(uint32_t mark) {
		return (mark & kLayout_Typesetting);
	}

	bool View::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			auto v = _first;
			while (v) {
				v->set_layout_offset_lazy(Vec2()); // lazy layout
				v = v->next();
			}
			unmark(kLayout_Typesetting | kLayout_Size_Width | kLayout_Size_Height);
		}
		return false;
	}

	void View::layout_text(TextLines *lines, TextConfig* cfg) {
		// NOOP
	}

	void View::onChildLayoutChange(Layout* child, uint32_t value) {
		if (value & (kChild_Layout_Size | kChild_Layout_Visible | kChild_Layout_Align | kChild_Layout_Text)) {
			mark(kLayout_Typesetting);
		}
	}

	void View::onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) {
		// NOOP
	}

	void View::solve_marks(uint32_t mark) {

		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			if (_parent) {
				_parent->matrix().mul(layout_matrix(), _matrix);
			} else {
				_matrix = layout_matrix();
			}
			goto visible_region;
		}

		if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
		visible_region:
			_visible_region = solve_visible_region();
		}
	}

	Vec2 View::position() {
		return Vec2(_matrix[2], _matrix[5]);
	}

	bool View::solve_visible_region() {
		return true;
	}

	/**
	* @method overlap_test Overlap test, test whether the point on the screen overlaps with the view
	*/
	bool View::overlap_test(Vec2 point) {
		return false;
	}
	
	void View::accept(ViewVisitor *visitor) {
		visitor->visitView(this);
	}

	TextInput* View::as_text_input() {
		return nullptr;
	}

	Button* View::as_button() {
		return nullptr;
	}

	/**
	* @func overlap_test_from_convex_quadrilateral
	*/
	bool View::overlap_test_from_convex_quadrilateral(Vec2* quadrilateral_vertex, Vec2 point) {
		/*
		* 直线方程：(x-x1)(y2-y1)-(y-y1)(x2-x1)=0
		* 平面座标系中凸四边形内任一点是否存在：
		* [(x-x1)(y2-y1)-(y-y1)(x2-x1)][(x-x4)(y3-y4)-(y-y4)(x3-x4)] < 0  and
		* [(x-x2)(y3-y2)-(y-y2)(x3-x2)][(x-x1)(y4-y1)-(y-y1)(x4-x1)] < 0
		*/
		
		float x = point.x();
		float y = point.y();
		
		#define x1 quadrilateral_vertex[0].x()
		#define y1 quadrilateral_vertex[0].y()
		#define x2 quadrilateral_vertex[1].x()
		#define y2 quadrilateral_vertex[1].y()
		#define x3 quadrilateral_vertex[2].x()
		#define y3 quadrilateral_vertex[2].y()
		#define x4 quadrilateral_vertex[3].x()
		#define y4 quadrilateral_vertex[3].y()
		
		if (((x-x1)*(y2-y1)-(y-y1)*(x2-x1))*((x-x4)*(y3-y4)-(y-y4)*(x3-x4)) < 0 &&
				((x-x2)*(y3-y2)-(y-y2)*(x3-x2))*((x-x1)*(y4-y1)-(y-y1)*(x4-x1)) < 0
		) {
			return true;
		}
		
		#undef x1
		#undef y1
		#undef x2
		#undef y2
		#undef x3
		#undef y3
		#undef x4
		#undef y4
		
		return false;
	}

	/**
	* @method screen_rect_from_convex_quadrilateral
	*/
	Rect View::screen_rect_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]) {
		Region re = screen_region_from_convex_quadrilateral(quadrilateral_vertex);
		return { re.origin, re.end - re.origin };
	}

	/**
	* @method screen_region_from_convex_quadrilateral
	*/
	Region View::screen_region_from_convex_quadrilateral(Vec2* quadrilateral_vertex) {
		#define A quadrilateral_vertex[0]
		#define B quadrilateral_vertex[1]
		#define C quadrilateral_vertex[2]
		#define D quadrilateral_vertex[3]
		
		Vec2 min, max;//, size;
		
		float w1 = fabs(A.x() - C.x());
		float w2 = fabs(B.x() - D.x());
		
		if (w1 > w2) {
			if ( A.x() > C.x() ) {
				max.set_x( A.x() ); min.set_x( C.x() );
			} else {
				max.set_x( C.x() ); min.set_x( A.x() );
			}
			if ( B.y() > D.y() ) {
				max.set_y( B.y() ); min.set_y( D.y() );
			} else {
				max.set_y( D.y() ); min.set_y( B.y() );
			}
			//size = Vec2(w1, max.y() - min.y());
		} else {
			if ( B.x() > D.x() ) {
				max.set_x( B.x() ); min.set_x( D.x() );
			} else {
				max.set_x( D.x() ); min.set_x( B.x() );
			}
			if ( A.y() > C.y() ) {
				max.set_y( A.y() ); min.set_y( C.y() );
			} else {
				max.set_y( C.y() ); min.set_y( A.y() );
			}
			//size = Vec2(w2, max.y() - min.y());
		}
		
		#undef A
		#undef B
		#undef C
		#undef D
			
		return {
			min, max
		};
	}

	/**
	 * @method set_is_focus(value)
	 */
	void View::set_is_focus(bool value) {
		if ( value ) {
			focus();
		} else {
			blur();
		}
	}

	/**
	 * @method is_focus()
	 */
	bool View::is_focus() const {
		return window() && window()->dispatch()->focus_view() == this;
	}

	/**
	 *
	 * Can it be the focus
	 * 
	 * @method can_become_focus()
	 */
	bool View::can_become_focus() {
		return false;
	}

	/**
	 * @method blur()
	 */
	bool View::blur() {
		if ( is_focus() ) {
			auto root = window()->root();
			if ( root && root != this ) {
				return root->focus();
			}
			return false;
		}
		return true;
	}

	/**
		*
		* is clip render the view
		* 
		* @method clip()
		*/
	bool View::clip() {
		return false;
	}

	/**
		* @overwrite
		*/
	void View::trigger_listener_change(uint32_t name, int count, int change) {
		if ( change > 0 ) {
			_receive = true; // bind event auto open option
		}
	}

	/**
		* @method has_child(child)
		*/
	bool View::has_child(View *child) {
		if ( child ) {
			View *parent = child->_parent;
			while (parent) {
				if ( parent == this ) {
					return true;
				}
				parent = parent->_parent;
			}
		}
		return false;
	}

}