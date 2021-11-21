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

#ifndef __flare__event__
#define __flare__event__

#include "./util/event.h"
#include "./math.h"
#include "./value.h"
#include "./keyboard.h"

// all ui events / NAME, FLAG
#define FX_UI_EVENTs(F) \
	/* can bubble event */ \
	F(Click, UI_EVENT_FLAG_CLICK | UI_EVENT_FLAG_BUBBLE) \
	F(Back, UI_EVENT_FLAG_CLICK | UI_EVENT_FLAG_BUBBLE) \
	F(KeyDown, UI_EVENT_FLAG_KEYBOARD | UI_EVENT_FLAG_BUBBLE) /* View */\
	F(KeyPress, UI_EVENT_FLAG_KEYBOARD | UI_EVENT_FLAG_BUBBLE) \
	F(KeyUp, UI_EVENT_FLAG_KEYBOARD | UI_EVENT_FLAG_BUBBLE) \
	F(KeyEnter, UI_EVENT_FLAG_KEYBOARD | UI_EVENT_FLAG_BUBBLE) \
	F(TouchStart, UI_EVENT_FLAG_TOUCH | UI_EVENT_FLAG_BUBBLE) \
	F(TouchMove, UI_EVENT_FLAG_TOUCH | UI_EVENT_FLAG_BUBBLE) \
	F(TouchEnd, UI_EVENT_FLAG_TOUCH | UI_EVENT_FLAG_BUBBLE) \
	F(TouchCancel, UI_EVENT_FLAG_TOUCH | UI_EVENT_FLAG_BUBBLE) \
	F(MouseOver, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseOut, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseLeave, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseEnter, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseMove, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseDown, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseUp, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(MouseWheel, UI_EVENT_FLAG_MOUSE | UI_EVENT_FLAG_BUBBLE) \
	F(Focus, UI_EVENT_FLAG_BUBBLE) \
	F(Blur, UI_EVENT_FLAG_BUBBLE) \
	/* canno bubble event */ \
	F(Highlighted, UI_EVENT_FLAG_HIGHLIGHTED) /* normal / hover / down */ \
	F(ActionKeyframe, UI_EVENT_FLAG_ACTION) \
	F(ActionLoop, UI_EVENT_FLAG_ACTION) \
	F(FocusMove, UI_EVENT_FLAG_FOCUS_MOVE) /*Panel*/ \
	F(Scroll, UI_EVENT_FLAG_NONE) /*BasicScroll*/\
	F(Change, UI_EVENT_FLAG_NONE) /*Input*/ \
	F(Load, UI_EVENT_FLAG_NONE) /* Image */ \
	/* player */ \
	F(Error, UI_EVENT_FLAG_PLAYER | UI_EVENT_FLAG_ERROR) \
	F(Ready, UI_EVENT_FLAG_PLAYER) /* AutoPlayer / Video */ \
	F(WaitBuffer, UI_EVENT_FLAG_PLAYER | UI_EVENT_FLAG_FLOAT) \
	F(StartPlay, UI_EVENT_FLAG_PLAYER) \
	F(SourceEnd, UI_EVENT_FLAG_PLAYER) \
	F(Pause, UI_EVENT_FLAG_PLAYER) \
	F(Resume, UI_EVENT_FLAG_PLAYER) \
	F(Stop, UI_EVENT_FLAG_PLAYER) \
	F(Seek, UI_EVENT_FLAG_PLAYER | UI_EVENT_FLAG_UINT64) \


namespace flare {

	class Application;
	class View;
	class Action;

	// event flags / category / cast
	enum {
		UI_EVENT_FLAG_NONE = 0,
		UI_EVENT_FLAG_CATEGORY = (255 << 0), // category flag
		UI_EVENT_FLAG_CAST = (255 << 8), // Event::data(), cast flag
		UI_EVENT_FLAG_KEYBOARD = (1 << 0),
		UI_EVENT_FLAG_CLICK,
		UI_EVENT_FLAG_HIGHLIGHTED,
		UI_EVENT_FLAG_TOUCH,
		UI_EVENT_FLAG_MOUSE,
		UI_EVENT_FLAG_ACTION,
		UI_EVENT_FLAG_FOCUS_MOVE,
		UI_EVENT_FLAG_ERROR = (1 << 8),
		UI_EVENT_FLAG_FLOAT,
		UI_EVENT_FLAG_UINT64,
		UI_EVENT_FLAG_BUBBLE = (1 << 16), // bubble, other flag
		UI_EVENT_FLAG_PLAYER = (1 << 17), // player
	};

	// event returl value mask
	enum ReturnValueMask {
		RETURN_VALUE_MASK_DEFAULT = (1 << 0),
		RETURN_VALUE_MASK_BUBBLE = (1 << 1),
		RETURN_VALUE_MASK_ALL = (RETURN_VALUE_MASK_DEFAULT | RETURN_VALUE_MASK_BUBBLE),
	};

	// event highlighed status
	enum HighlightedStatus {
		HIGHLIGHTED_NORMAL = 1,
		HIGHLIGHTED_HOVER,
		HIGHLIGHTED_DOWN,
	};

	// event name
	class FX_EXPORT UIEventName {
	 public:
		inline UIEventName() { FX_UNREACHABLE(); }
		inline UIEventName(cString& n, uint32_t flag)
			: name_(n), code_((uint32_t)n.hash_code()), flag_(flag) {}
		inline uint32_t hash_code() const { return code_; }
		inline bool equals(const UIEventName& o) const { return o.hash_code() == code_; }
		inline String to_string() const { return name_; }
		inline int flag() const { return flag_; }
		inline bool operator==(const UIEventName& type) const { return type.code_ == code_; }
		inline bool operator!=(const UIEventName& type) const { return type.code_ != code_; }
		inline bool operator>(const UIEventName& type) const { return code_ > type.code_; }
		inline bool operator<(const UIEventName& type) const { return code_ < type.code_; }
	 private:
		String  name_;
		uint32_t code_, flag_;
	};

	// event names string => UIEventName
	FX_EXPORT extern const Dict<String, UIEventName> UIEventNames;

	// define event names
	#define FX_FUN(NAME, FLAG) \
		FX_EXPORT extern const UIEventName UIEvent_##NAME;
	FX_UI_EVENTs(FX_FUN)
	#undef FX_FUN

	// -----------------------------------

	/**
	* @func UIEvent gui event
	*/
	class FX_EXPORT UIEvent: public Event<View, Object, View, int> {
	 public:
		// inline UIEvent(cSendData& data): Event<View, Object, View>() { FX_UNREACHABLE(); }
		UIEvent(View* origin);
		inline uint64_t timestamp() const { return time_; }
		inline bool is_default() const { return return_value & RETURN_VALUE_MASK_DEFAULT; }
		inline bool is_bubble() const { return return_value & RETURN_VALUE_MASK_BUBBLE; }
		inline void cancel_default() { return_value &= ~RETURN_VALUE_MASK_DEFAULT; }
		inline void cancel_bubble() { return_value &= ~RETURN_VALUE_MASK_BUBBLE; }
	 private:
		uint64_t time_;
	};

	/**
	* @class ActionEvent
	*/
	class FX_EXPORT ActionEvent: public UIEvent {
	 public:
		inline ActionEvent(Action* action, View* origin, uint64_t delay, uint32_t frame, uint32_t loop)
			: UIEvent(origin), action_(action), delay_(delay), frame_(frame), loop_(loop) {}
		inline Action* action() const { return action_; }
		inline uint64_t delay() const { return delay_; }
		inline uint32_t frame() const { return frame_; }
		inline uint32_t loop() const { return loop_; }
		virtual void release();
	 private:
		Action* action_;
		uint64_t  delay_;
		uint32_t  frame_, loop_;
	};

	/**
	* @func KeyEvent keyboard event
	*/
	class FX_EXPORT KeyEvent: public UIEvent {
	 public:
		inline KeyEvent(View* origin, uint32_t keycode,
										bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
										uint32_t repeat, int device, int source)
			: UIEvent(origin), keycode_(keycode)
			, device_(device), source_(source), repeat_(repeat), shift_(shift)
			, ctrl_(ctrl), alt_(alt), command_(command), caps_lock_(caps_lock), focus_move_(nullptr) {}
		inline int  keycode() const { return keycode_; }
		inline int  repeat() const { return repeat_; }
		inline int  device() const { return device_; }
		inline int  source() const { return source_; }
		inline bool shift() const { return shift_; }
		inline bool ctrl() const { return ctrl_; }
		inline bool alt() const { return alt_; }
		inline bool command() const { return command_; }
		inline bool caps_lock() const { return caps_lock_; }
		inline void set_keycode(int value) { keycode_ = value; }
		inline View* focus_move() const { return focus_move_; }
		inline void set_focus_move(View* view) { if (origin()) focus_move_ = view; }
		virtual void release();
	 private:
		int  keycode_;
		int  device_, source_, repeat_;
		bool shift_, ctrl_, alt_;
		bool command_, caps_lock_;
		View* focus_move_;
	};

	/**
	* @class ClickEvent click event
	*/
	class FX_EXPORT ClickEvent: public UIEvent {
	 public:
		enum Type { TOUCH = 1, KEYBOARD = 2, MOUSE = 3 };
		inline ClickEvent(View* origin, float x, float y, Type type, uint32_t count = 1)
			: UIEvent(origin), x_(x), y_(y), count_(count), type_(type) {}
		inline float x() const { return x_; }
		inline float y() const { return y_; }
		inline uint32_t count() const { return count_; }
		inline Type type() const { return type_; }
	 private:
		float x_, y_;
		uint32_t count_;
		Type type_;
	};

	/**
	* @class UIMouseEvent mouse event
	*/
	class FX_EXPORT MouseEvent: public KeyEvent {
	 public:
		inline MouseEvent(View* origin, float x, float y, uint32_t keycode,
											bool shift, bool ctrl, bool alt, bool command, bool caps_lock,
											uint32_t repeat = 0, int device = 0, int source = 0)
			: KeyEvent(origin, keycode, shift, ctrl, alt,
				command, caps_lock, repeat, device, source), x_(x), y_(y) {}
		inline float x() const { return x_; }
		inline float y() const { return y_; }
	 private:
		float x_, y_;
	};

	/**
	* @class HighlightedEvent status event
	*/
	class FX_EXPORT HighlightedEvent: public UIEvent {
	 public:
		inline HighlightedEvent(View* origin, HighlightedStatus status)
			: UIEvent(origin), _status(status) {}
		inline HighlightedStatus status() const { return _status; }
	 private:
		HighlightedStatus _status;
	};

	/**
	* @class TouchEvent touch event
	*/
	class FX_EXPORT TouchEvent: public UIEvent {
	 public:
		struct TouchPoint { // touch event point
			uint32_t id;
			float    start_x, start_y;
			float    x, y, force;
			bool     click_in;
			View*    view;
		};
		inline TouchEvent(View* origin, Array<TouchPoint>& touches)
			: UIEvent(origin), _change_touches(touches) {}
		inline Array<TouchPoint>& changed_touches() { return _change_touches; }
	 private:
		Array<TouchPoint> _change_touches;
	};

	typedef TouchEvent::TouchPoint TouchPoint;

	/**
	* @class FocusMoveEvent
	*/
	class FX_EXPORT FocusMoveEvent: public UIEvent {
	 public:
		inline FocusMoveEvent(View* origin, View* old_focus, View* new_focus)
			: UIEvent(origin), _old_focus(old_focus), _new_focus(new_focus) {}
		inline View* old_focus() { return _old_focus; }
		inline View* new_focus() { return _new_focus; }
		inline View* focus() { return _old_focus; }
		inline View* focus_move() { return _new_focus; }
		virtual void release();
	 private:
		View* _old_focus;
		View* _new_focus;
	};

	/**
	* @class ITextInput
	*/
	class FX_EXPORT ITextInput: public Protocol {
	 public:
		virtual void input_delete(int count) = 0;
		virtual void input_insert(cString& text) = 0;
		virtual void input_marked(cString& text) = 0;
		virtual void input_unmark(cString& text) = 0;
		virtual void input_control(KeyboardKeyName name) = 0;
		virtual bool input_can_delete() = 0;
		virtual bool input_can_backspace() = 0;
		virtual Vec2 input_spot_location() = 0;
		virtual KeyboardType input_keyboard_type() = 0;
		virtual KeyboardReturnType input_keyboard_return_type() = 0;
	};

	/**
	* @class EventDispatch
	*/
	class FX_EXPORT EventDispatch: public Object {
	 public:
		EventDispatch(Application* app);
		virtual ~EventDispatch();

		// handles
		void onTouchstart(List<TouchPoint>&& touches);
		void onTouchmove(List<TouchPoint>&& touches);
		void onTouchend(List<TouchPoint>&& touches);
		void onTouchcancel(List<TouchPoint>&& touches);
		void onMousemove(float x, float y);
		void onMousepress(KeyboardKeyName key, bool down);
		void onIme_delete(int count);
		void onIme_insert(cString& text);
		void onIme_marked(cString& text);
		void onIme_unmark(cString& text);
		void onIme_control(KeyboardKeyName name);

		/**
		* @func make_text_input(input)
		*/
		void make_text_input(ITextInput* input);
		
		/**
		* @func keyboard_adapter()
		*/
		inline KeyboardAdapter* keyboard_adapter() {
			return _keyboard;
		}
		
	 private:
		class OriginTouche;
		class MouseHandle;
		typedef Dict<View*, OriginTouche*> OriginTouches;
		
		Application*        _host;
		OriginTouches       _origin_touches;
		MouseHandle*        _mouse_h;
		KeyboardAdapter*    _keyboard;
		ITextInput*         _text_input;
		
		FX_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif
