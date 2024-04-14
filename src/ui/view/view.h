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

#ifndef __quark__ui__view__
#define __quark__ui__view__

#include "../types.h"
#include "../event.h"
#include "../view_prop.h"
#include "../pre_render.h"

namespace qk {
	class Action;
	class TextInput;
	class TextLines;
	class TextConfig;
	class TextOptions;
	class UIRender;
	class Window;
	class Transform;
	class ScrollBase;
	class CStyleSheetsClass;
	class Button;

	/**
		* View tree node base type,
		* Provide APIs that do not use security locks on worker and rendering threads,
		* When setting properties on the main thread and calculating view results on the rendering thread, 
		* data that can only be accessed by the rendering thread is usually suffixed with "RT" or mark as @safe Rt
		*
	 * @class View
		*/
	class Qk_EXPORT View: public Notification<UIEvent, UIEventName, Reference> {
		Qk_HIDDEN_ALL_COPY(View);
		Qk_DEFINE_INLINE_CLASS(InlEvent);
		CStyleSheetsClass *_cssclass;
	public:

		// Layout mark key values
		enum LayoutMark: uint32_t {
			kLayout_None              = (0),      /* 没有任何标记 */
			kLayout_Size_Width        = (1 << 0), /* 布局尺寸改变, 尺寸改变可能影响父布局 */
			kLayout_Size_Height       = (1 << 1),
			kLayout_Typesetting       = (1 << 2), /* 布局内容偏移, 需要重新对子布局排版 */
			kTransform_Origin         = (1 << 3),
			kInput_Status             = (1 << 4), /* 输入状态这不包含布局的改变 */
			kScroll                   = (1 << 5), /* scroll status change */
			kStyle_Class              = (1 << 6), /* 变化class引起的样式变化 */
			// RECURSIVE MARKS
			kRecursive_Transform      = (1 << 30), /* 矩阵变换 recursive mark */
			kRecursive_Visible_Region = (1U << 31), /* 可见范围 */
			kRecursive_Mark           = (kRecursive_Transform | kRecursive_Visible_Region),
		};

		// child layout change mark key values
		enum ChildLayoutChangeMark : uint32_t {
			kChild_Layout_Size     = (1 << 0),
			kChild_Layout_Visible  = (1 << 1),
			kChild_Layout_Align    = (1 << 2),
			kChild_Layout_Weight   = (1 << 3),
			kChild_Layout_Text     = (1 << 4),
		};

		// layout size
		struct Size {
			Vec2 layout_size, content_size;
			bool wrap_x, wrap_y;
		};

		/**
		 * @prop style sheets class object
		 * @safe Mt
		 * @note Can only be used in main threads
		*/
		Qk_DEFINE_PROP_ACC_GET(CStyleSheetsClass*, cssclass);

		/**
		 * @prop parent view
		 * @safe Mt
		 * @note Can only be used in main threads
		*/
		Qk_DEFINE_PROP_GET(View*, parent); // @safe Mt
		Qk_DEFINE_PROP_GET(View*, prev); // @safe Mt
		Qk_DEFINE_PROP_GET(View*, next); // @safe Mt
		Qk_DEFINE_PROP_GET(View*, first); // @safe Mt
		Qk_DEFINE_PROP_GET(View*, last); // @safe Mt

		/**
		 * @prop parent_Rt view
		 * @safe Rt
		 * @note Can only be used in rendering threads
		*/
		Qk_DEFINE_PROP_GET(View*, parent_Rt); // @safe Rt
		Qk_DEFINE_PROP_GET(View*, prev_Rt); // @safe Rt
		Qk_DEFINE_PROP_GET(View*, next_Rt); // @safe Rt
		Qk_DEFINE_PROP_GET(View*, first_Rt); // @safe Rt
		Qk_DEFINE_PROP_GET(View*, last_Rt); // @safe Rt

		/**
		* @prop window
		*/
		Qk_DEFINE_PROP_GET(Window*, window);

		/**
		 * the objects that automatically adjust view properties
		*/
		Qk_DEFINE_PROP(Action*, action) throw(Error);

		/**
		 * @prop props accessor
		*/
		Qk_DEFINE_PROP_GET(PropAccessor*, accessor);

		/**
		 * @prop transform
		*/
		Qk_DEFINE_PROP_ACC_GET(Transform*, transform);

		/**
		* @prop mark_value
		* @safe Rt
		* @note Can only be used in rendering threads
		*
		* The marked view will be updated before starting frame drawing.
		* During operation, view local attributes may be updated frequently or the view may rarely change.
		*/
		Qk_DEFINE_PROP_GET(uint32_t, mark_value, Const);

		/**
		* Next preprocessing view tag
		* You need to call `layout_forward` and `layout_reverse` to process these marked views before drawing.
		* Not all views will change at the same time. If the view tree is very large,
		* If it comes to view, in order to track changes in one of the view nodes, it is necessary to traverse the entire view tree. In order to avoid this situation
		* Separate the marked views outside the view, classify them according to the view level, and store them in the form of a two-way circular linked list (PreRender)
		* This avoids accessing views that have not changed and allows them to be accessed sequentially according to the view hierarchy.
		* 
		* @prop mark_index
		* @safe Rt
		* @note Can only be used in rendering threads
		*/
		Qk_DEFINE_PROP_GET(int32_t, mark_index, Const);

		/**
		* @prop level
		* @safe Rt
			* @note Can only be used in rendering threads
		*
		* 布局在UI树中所处的深度，0表示还没有加入到UI视图树中
		* 这个值受`View::_visible`影响, View::_visible=false时_level=0
		*/
		Qk_DEFINE_PROP_GET(uint32_t, level, Const);

		/**
		 *
		 * View at the final position on the screen (parent.matrix * (offset + offset_inside))
		 *
		 * @prop position
		 * @safe Rt
			* @note Can only be used in rendering threads
		 */
		Qk_DEFINE_PROP_GET(Vec2, position, ProtectedConst);

		/**
		 *  can affect the transparency of subviews
		 */
		Qk_DEFINE_PROP(float, opacity, Const);

		/**
		 * @prop Cursor style
		*/
		Qk_DEFINE_PROP(CursorStyle, cursor, Const);

		/**
		 * Set the visibility of the view. When this value is set to 'false',
		 * the view is invisible and does not occupy any view space
		*/
		Qk_DEFINE_PROP(bool, visible, Const);

		/**
		 *  这个值与`visible`不相关，这个代表视图在当前显示区域是否可见，这个显示区域大多数情况下就是屏幕
		*/
		Qk_DEFINE_PROP(bool, visible_region, ProtectedConst);

		/**
		 * Do views need to receive or handle system event throws? In most cases,
		 * these events do not need to be handled, which can improve overall event processing efficiency
		*/
		Qk_DEFINE_PROP(bool, receive, Const);

		/**
		 * keyboard focus view
		*/
		Qk_DEFINE_PROP_ACC(bool, is_focus, Const);

		/**
		 * @method Make(...)
		*/
		template<class T = View, typename... Args>
		inline T* Make(Args... args) {
			return static_cast<T*>(static_cast<View*>(new T(args...))->init(_window));
		}

		template<class T = View, typename... Args>
		inline T* prepend_new(Args... args) {
			return Make<T>(args...)->template prepend_to<T>(this);
		}

		template<class T = View, typename... Args>
		inline T* append_new(Args... args) {
			return Make<T>(args...)->template append_to<T>(this);
		}

		template<class Return = View>
		inline Return* prepend_to(View* parent) {
			return parent->prepend(this), static_cast<Return*>(this);
		}

		template<class Return = View>
		inline Return* append_to(View* parent) {
			return parent->append(this), static_cast<Return*>(this);
		}

		/**
		 * @method destroy() heap memory destructor
		 */
		void destroy() override;

		/**
		 * @method safe_view() Returns safe self
		*/
		Sp<View> safe_view();

		/**
		 *
		 * focus keyboard
		 *
		 * @method focus()
		 */
		bool focus();

		/**
		 *
		 * Unfocus keyboard
		 *
		 * @method blur()
		 */
		bool blur();

		/**
		 * @method is_self_child(child)
		 */
		bool is_self_child(View *child);

		/**
			*
			* Add a sibling view to the front
			*
			* @method before(view)
			*/
		void before(View *view);

		/**
			*
			* Add a sibling view to the back
			*
			* @method after(view)
			*/
		void after(View *view);

		/**
			* 
			* Append sub view to front
			* 
			* @method prepend(child)
			*/
		void prepend(View *child);

		/**
			*
			* Append sub view to end
			*
			* @method append(child)
			*/
		void append(View *child);

		/**
		 *
		 * Remove self from parent view
		 *
		 * @method remove()
		 */
		void remove();

		/**
		 *
		 * remove all sub view
		 *
		 * @method remove_all_child()
		 */
		void remove_all_child();

		/**
		 *
		 * Can it be the focus
		 *
		 * @method can_become_focus()
		 */
		virtual bool can_become_focus();

		/**
		 * 
		 * Returns button object
		 * 
		 * @method asButton()
		*/
		virtual Button* asButton();

		/**
		 * 
		 * Returns text input object
		 * 
		 * @method asTextInput()
		*/
		virtual TextInput* asTextInput();

		/**
		 * 
		 * Returns as transform
		 * 
		 * @method asTransform()
		*/
		virtual Transform* asTransform();

		/**
		 * 
		 * Returns as textOptions
		 * 
		 * @method asTextOptions()
		*/
		virtual TextOptions* asTextOptions();

		/**
		 * 
		 * Returns as ScrollBase
		 * 
		 * @method asScrollBase()
		*/
		virtual ScrollBase* asScrollBase();

		/**
			*
			* 布局权重（比如在flex布局中代表布局的尺寸）
			*
			* @method layout_weight()
			*/
		virtual float layout_weight();

		/**
			*
			* 布局的对齐方式（九宫格）
			*
			* @method layout_align()
			*/
		virtual Align layout_align();

		/**
		 *
		 * is clip render the view
		 *
		 * @method is_clip()
		 */
		virtual bool is_clip();

		/**
		 * @method viewType()
		*/
		virtual ViewType viewType() const;

		/**
			* 
			* Relative to the parent view (layout_offset) to start offset
			* 
			* @method layout_offset()
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual Vec2 layout_offset();

		/**
			*
			* Returns the view size of view object (if is box view the: size=margin+border+padding+content)
			*
			* Returns the view content size of object view, 
			* Returns false to indicate that the size is unknown,
			* indicates that the size changes with the size of the sub view, and the content is wrapped
			*
			* @method layout_size()
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual Size layout_size();

		/**
			*
			* Returns the and compute view size of object view
			*
			* @method layout_raw_size(parent_content_size)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual Size layout_raw_size(Size parent_content_size);

		/**
			* Returns internal view offset compensation of the view, which affects the sub view offset position
			* 
			* For example: when a view needs to set the scrolling property scroll of a sub view, you can set this property
			*
			* @method layout_offset_inside()
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual Vec2 layout_offset_inside();

		/**
			*
			* whether the child view has been locked
			*
			* @method is_lock_child_layout_size()
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual bool is_lock_child_layout_size();

		/**
			* 
			* Setting the view offset of the view object in the parent view
			*
			* @method set_layout_offset(val)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual void set_layout_offset(Vec2 val);

		/**
			* 
			* Setting view offset values lazily mode for the view object
			*
			* @method set_layout_offset_lazy(size)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual void set_layout_offset_lazy(Vec2 size);

		/**
			* 锁定布局的尺寸。在特定的布局类型中自身无法直接确定其自身尺寸，一般由父布局调用如：flex布局类型
			*
			* 这个方法应该在`layout_forward()`正向迭代中由父布局调用,因为尺寸的调整一般在正向迭代中
			* 
			* 返回锁定后的最终尺寸，调用后视返回后的尺寸为最终尺寸
			* 
			* @method layout_lock(layout_size)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual Vec2 layout_lock(Vec2 layout_size);

		/**
			*
			* (计算布局自身的尺寸)
			*
			* 从外向内正向迭代布局，比如一些布局方法是先从外部到内部先确定盒子的明确尺寸
			* 
			* 这个方法被调用时父视图尺寸一定是有效的，在调用`content_size`时有两种情况，
			* 返回`false`表示父视图尺寸是wrap的，返回`true`时表示父视图有明确的尺寸
			* 
			* @method layout_forward(mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual bool layout_forward(uint32_t/*LayoutMark*/ mark);

		/**
			* 
			* (计算子布局的偏移位置，以及确定在`layout_forward()`函数没有能确定的尺寸)
			* 
			* 从内向外反向迭代布局，比如有些视图外部并没有明确的尺寸，
			* 尺寸是由内部视图挤压外部视图造成的，所以只能先明确内部视图的尺寸。
			*
			* 这个方法被调用时子视图尺寸一定是明确的有效的，调用`layout_size()`返回子视图外框尺寸。
			* 
			* @method layout_reverse(mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual bool layout_reverse(uint32_t/*LayoutMark*/ mark);

		/**
		 * 
		 * solve text view
		 * 
		 * @method layout_text(lines)
		 * @safe Rt
			* @note Can only be used in rendering threads
		 */
		virtual void layout_text(TextLines *lines, TextConfig* textSet);

		/**
			* 
			* This method of the parent layout is called when the layout content of the child layout changes
			*
			* This is not necessarily called by the child layout
			*
			* @method onChildLayoutChange(child, mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual void onChildLayoutChange(View* child, uint32_t/*ChildLayoutChangeMark*/ mark);

		/**
			* 
			* This method of the child layout is called when the layout size of the parent layout changes
			* 
			* @method onParentLayoutContentSizeChange(parent, mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual void onParentLayoutContentSizeChange(View* parent, uint32_t/*LayoutMark*/ mark);

		/**
		 * Overlap test, test whether the point on the screen overlaps with the view
		 * @method overlap_test
		 * @safe Rt
			* @note Can only be used in rendering threads
		*/
		virtual bool overlap_test(Vec2 point);

		/**
		 * 
		 * returns view position center in the position
		 * 
		 * @method center()
		 * @safe Rt
			* @note Can only be used in rendering threads
		*/
		virtual Vec2 center();

		/**
		 * @method solve_marks(mark)
		 * @safe Rt
			* @note Can only be used in rendering threads
		*/
		virtual void solve_marks(const Mat &mat, uint32_t mark);

		/**
			* @method solve_visible_region()
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		virtual bool solve_visible_region(const Mat &mat);

		/**
		 * notice update for set parent or level
		 * 
		 * @method onActivate()
		 * @safe Rt
			* @note Can only be used in rendering threads
		*/
		virtual void onActivate();

		/**
		 * @method draw()
		 * @safe Rt
		 * @note Can only be used in rendering threads
		 */
		virtual void draw(UIRender *render);

		/**
			* @method async_mark(mark)
			* @safe Mt
			*/
		void async_mark(uint32_t mark = kLayout_None);

		/**
			* @method async_mark_layout(mark)
			* @safe Mt
			*/
		void async_mark_layout(uint32_t mark);

		/**
			* @method mark(mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		void mark(uint32_t mark = kLayout_None);

		/**
			* @method unmark(mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		inline void unmark(uint32_t mark = (~kLayout_None/*default unmark all*/)) {
			_mark_value &= (~mark);
		}

		/**
			* @method mark_layout(mark)
			* @safe Rt
			* @note Can only be used in rendering threads
			*/
		void mark_layout(uint32_t mark);

		/**
		 * @method preRender()
		*/
		PreRender& preRender();

	protected:
		virtual View* init(Window* win);
		View(); // @constructor
	private:
		void set_parent(View *parent); // setting parent view
		void clear_link(); // Cleaning up associated view information
		void before_Rt(View *view);
		void after_Rt(View *view);
		void prepend_Rt(View *child);
		void append_Rt(View *child);
		void remove_Rt();
		void set_parent_Rt(View *parent);
		void clear_link_Rt(); // Cleaning up associated view information rt
		void clear_level(); //  clear view depth rt
		void set_level_(uint32_t level); // settings depth
		void set_visible_Rt_(bool visible, uint32_t level);
		void applyClass_Rt(CStyleSheetsClass* parentSsc);
		CStyleSheetsClass* parentSsclass_Rt();

		friend class UIRender;
		friend class PreRender;
		friend class EventDispatch;
		friend class Root;
	};

}
#endif