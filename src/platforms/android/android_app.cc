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

#include "quark/util/loop.h"
#include "quark/app.h"
#include "quark/event.h"
#include "quark/display.h"
#include "../linux/linux_gl.h"
#include "quark/util/platform/android_jni.h"
#include "quark/util/platform/android_api.h"
#include <android/native_activity.h>
#include <android/native_window.h>

namespace qk {

	class AndroidApplication;
	static AndroidApplication* application = nullptr;
	static GLDrawProxy* gl_draw_context = nullptr;
	typedef Display::Orientation Orientation;

	/**
	* @class AndroidApplication
	*/
	class AndroidApplication {
	public:
		typedef NonObjectTraits Traits;

		inline void start_render_task() {
			if ( API::is_screen_on() ) {
				_render_looper->start();
			}
		}

		inline void stop_render_task() {
			_render_looper->stop();
		}

		inline Orientation orientation() const { 
			return _current_orientation; 
		}

		inline AppInl* host() const { 
			return _host; 
		}

	private:
		AndroidApplication()
		: _activity(nullptr), _window(nullptr)
		, _host(nullptr), _queue(nullptr)
		, _looper(nullptr), _render_looper(nullptr), _dispatch( nullptr )
		, _current_orientation(Orientation::ORIENTATION_INVALID)
		, _is_init_ok(false)
		{
			Qk_Assert(!application); application = this;
			_looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
		}

	public:
		// -------------------------------static---------------------------------

		static void onCreate(ANativeActivity* activity, void* saved_state, size_t saved_state_size) {
			if ( !application ) {
				new AndroidApplication();
			}
			// ANativeActivity_setWindowFlags(activity, 0x00000400, 0);

			activity->callbacks->onDestroy                  = &AndroidApplication::onDestroy;
			activity->callbacks->onStart                    = &AndroidApplication::onStart;
			activity->callbacks->onResume                   = &AndroidApplication::onResume;
			activity->callbacks->onSaveInstanceState        = &AndroidApplication::onSaveInstanceState;
			activity->callbacks->onPause                    = &AndroidApplication::onPause;
			activity->callbacks->onStop                     = &AndroidApplication::onStop;
			activity->callbacks->onConfigurationChanged     = &AndroidApplication::onConfigurationChanged;
			activity->callbacks->onLowMemory                = &AndroidApplication::onLowMemory;
			activity->callbacks->onWindowFocusChanged       = &AndroidApplication::onWindowFocusChanged;
			activity->callbacks->onNativeWindowCreated      = &AndroidApplication::onNativeWindowCreated;
			activity->callbacks->onNativeWindowResized      = &AndroidApplication::onNativeWindowResized;
			activity->callbacks->onNativeWindowRedrawNeeded = &AndroidApplication::onNativeWindowRedrawNeeded;
			activity->callbacks->onNativeWindowDestroyed    = &AndroidApplication::onNativeWindowDestroyed;
			activity->callbacks->onInputQueueCreated        = &AndroidApplication::onInputQueueCreated;
			activity->callbacks->onInputQueueDestroyed      = &AndroidApplication::onInputQueueDestroyed;
			activity->callbacks->onContentRectChanged       = &AndroidApplication::onContentRectChanged;
			activity->instance = application;

			application->_activity = activity;
		}

		static void onDestroy(ANativeActivity* activity) {
			Qk_Assert(application->_activity);

			activity->callbacks->onDestroy                  = nullptr;
			activity->callbacks->onStart                    = nullptr;
			activity->callbacks->onResume                   = nullptr;
			activity->callbacks->onSaveInstanceState        = nullptr;
			activity->callbacks->onPause                    = nullptr;
			activity->callbacks->onStop                     = nullptr;
			activity->callbacks->onConfigurationChanged     = nullptr;
			activity->callbacks->onLowMemory                = nullptr;
			activity->callbacks->onWindowFocusChanged       = nullptr;
			activity->callbacks->onNativeWindowCreated      = nullptr;
			activity->callbacks->onNativeWindowResized      = nullptr;
			activity->callbacks->onNativeWindowRedrawNeeded = nullptr;
			activity->callbacks->onNativeWindowDestroyed    = nullptr;
			activity->callbacks->onInputQueueCreated        = nullptr;
			activity->callbacks->onInputQueueDestroyed      = nullptr;
			activity->callbacks->onContentRectChanged       = nullptr;

			application->_activity = nullptr;

			// application->_app->triggerUnload();
			// delete application; application = nullptr;
		}

		// ----------------------------------------------------------------------

		static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
			// ScopeLock scope(application->_mutex);
			typedef Callback<RunLoop::PostSyncData> Cb;
			application->_window = window;
			application->_host->render_loop()->post_sync(Cb([window](Cb::Data &ev) {
				bool ok = false;
				{ //
					// ScopeLock scope(application->_mutex);
					if ( window == application->_window ) {
						ok = gl_draw_context->create_surface(window);
						Qk_Assert(ok);
					}
				}
				if ( ok ) {
					if ( application->_is_init_ok ) {
						gl_draw_context->refresh_surface_size(nullptr);
						application->_host->display()->render_frame(true); // 更新画面
					} else {
						application->_is_init_ok = true;
						gl_draw_context->initialize();
						application->_host->triggerLoad();
					}
					application->start_render_task();
				}
				ev.data->complete();
			}));
		}
		
		static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
			// ScopeLock scope(application->_mutex);
			application->_window = nullptr;
			typedef Callback<RunLoop::PostSyncData> Cb;
			application->_host->render_loop()->post_sync(Cb([window](Cb::Data& ev) {
				gl_draw_context->destroy_surface(window);
				application->stop_render_task();
				ev.data->complete();
			}));
		}

		static void onStart(ANativeActivity* activity) {
			
			if ( application->_host == nullptr ) { // start gui
				Application::runMain(0, nullptr); // run gui application

				application->_host = Inl_Application(shared_app());
				application->_dispatch = application->_host->dispatch();
				application->_render_looper = new RenderLooper(application->_host);

				Qk_Assert(application->_activity);
				Qk_Assert(application->_host);
				Qk_Assert(application->_host->render_loop());
			}
			application->_host->triggerForeground();
			application->stop_render_task();
		}

		static void onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
			/*
			* 这里做Rect选择绘制，主要因为android的虚拟导航按键，
			* 虚拟导航按键区域绘制黑色背景颜色填充
			*/
			application->_rect = {
				Vec2(rect->left, 0),
				Vec2(rect->right - rect->left, rect->bottom)
			};

			// 屏幕旋转都会触发这个事件，所以不做更多的监听工作
			Orientation orientation = (Orientation)API::get_orientation();
			int targger_orientation = (orientation != application->_current_orientation);
			if ( targger_orientation ) { // 屏幕方向改变
				application->_current_orientation = orientation;
			}

			typedef Callback<RunLoop::PostSyncData> Cb_;
			application->_host->render_loop()->post_sync(Cb_([targger_orientation](Cb_::Data &ev) {
				// NOTE: **********************************
				// 这里有点奇怪，因为绘图表面反应迟钝，
				// 也就是说 `ANativeWindow_getWidth()` 返回值可能与当前真实值不相同，
				// 但调用eglSwapBuffers()会刷新绘图表面。
				application->_host->display()->render_frame(true); // 所以刷新绘图表面
				// ****************************************
				gl_draw_context->refresh_surface_size(&application->_rect);

				if ( targger_orientation ) { // 触发方向变化事件
					application->_host->main_loop()->post(Cb([](Cb::Data& e) {
						application->_host->display()->Qk_Trigger(orientation);
					}));
				}
				ev.data->complete();
			}));
		}

		// ----------------------------------------------------------------------
		
		static void onStop(ANativeActivity* activity) {
			application->_host->triggerBackground();
			application->stop_render_task();
		}

		static void onWindowFocusChanged(ANativeActivity* activity, int hasFocus) {
			Qk_DLog("onWindowFocusChanged");
			application->start_render_task();
		}

		static void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
			Qk_DLog("onNativeWindowRedrawNeeded");
			application->start_render_task();
		}

		// ----------------------------------------------------------------------
		
		static void onLowMemory(ANativeActivity* activity) {
			Qk_DLog("onLowMemory");
			application->_host->triggerMemorywarning();
		}

		static void onResume(ANativeActivity* activity) {
			application->_host->triggerResume();
		}

		static void onPause(ANativeActivity* activity) {
			application->_host->triggerPause();
		}

		// ----------------------------------------------------------------------
		
		static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
			return nullptr;
		}
		
		static void onConfigurationChanged(ANativeActivity* activity) {
			Qk_DLog("onConfigurationChanged");
		}
		
		static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
			Qk_DLog("onNativeWindowResized");
		}

		// --------------------------- Dispatch event ---------------------------

		static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
			if ( queue != application->_queue ) {
				if ( application->_queue ) {
					AInputQueue_detachLooper(application->_queue);
				}
				AInputQueue_attachLooper(queue, application->_looper,
																ALOOPER_POLL_CALLBACK,
																(ALooper_callbackFunc) &onInputEvent_callback, queue);
				application->_queue = queue;
			}
		}
		
		static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
			AInputQueue_detachLooper(queue);
			application->_queue = nullptr;
		}

		// ----------------------------------------------------------------------

		static int onInputEvent_callback(int fd, int events, AInputQueue* queue) {
			AInputEvent* event = nullptr;

			while( AInputQueue_getEvent(queue, &event) >= 0 ) {
				if (AInputQueue_preDispatchEvent(queue, event) == 0) {
					dispatchEvent(event);
					AInputQueue_finishEvent(queue, event, fd);
				} else {
					Qk_DLog("AInputQueue_preDispatchEvent(queue, event) != 0");
				}
			}
			return 1;
		}

		static void dispatchEvent(AInputEvent* event) {
			EventDispatch* dispatch = application->_dispatch;

			int type = AInputEvent_getType(event);
			int device = AInputEvent_getDeviceId(event);
			int source = AInputEvent_getSource(event);

			if (type == AINPUT_EVENT_TYPE_KEY)
			{
				int code = AKeyEvent_getKeyCode(event);
				int repeat = AKeyEvent_getRepeatCount(event);
				int action = AKeyEvent_getAction(event);

				Qk_DLog("code:%d, repeat:%d, action:%d, "
												"flags:%d, scancode:%d, metastate:%d, downtime:%ld, time:%ld",
								code, repeat, action,
								AKeyEvent_getFlags(event),
								AKeyEvent_getScanCode(event),
								AKeyEvent_getMetaState(event),
								AKeyEvent_getDownTime(event),
								AKeyEvent_getEventTime(event)
				);

				switch (action) {
					case AKEY_EVENT_ACTION_DOWN:
						if (code)
							dispatch->keyboard_adapter()->dispatch(code, 0, 1, repeat, device, source);
						break;
					case AKEY_EVENT_ACTION_UP:
						if (code)
							dispatch->keyboard_adapter()->dispatch(code, 0, 0, repeat, device, source);
						break;
					case AKEY_EVENT_ACTION_MULTIPLE:
						Qk_DLog("AKEY_EVENT_ACTION_MULTIPLE");
						break;
				}
			}
			else
			{ // AINPUT_EVENT_TYPE_MOTION
				int action = AMotionEvent_getAction(event);
				int pointer_index = action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

				List<UITouch> touchs;
				UITouch touch;

				switch (action & AMOTION_EVENT_ACTION_MASK) {
					case AMOTION_EVENT_ACTION_DOWN:
					case AMOTION_EVENT_ACTION_POINTER_DOWN:
						getGuiTouch(event, pointer_index, &touch);
						touchs.push(touch);
						dispatch->dispatch_touchstart( std::move(touchs) );
						break;
					case AMOTION_EVENT_ACTION_UP:
					case AMOTION_EVENT_ACTION_POINTER_UP:
						getGuiTouch(event, pointer_index, &touch);
						touchs.push(touch);
						dispatch->dispatch_touchend( std::move(touchs) );
						break;
					case AMOTION_EVENT_ACTION_MOVE:
						touchs = toGuiTouchs(event, true);
						if ( touchs.length() ) {
							// Qk_DLog("AMOTION_EVENT_ACTION_MOVE, %d", touchs.length());
							dispatch->dispatch_touchmove( std::move(touchs) );
						}
						break;
					case AMOTION_EVENT_ACTION_CANCEL:
						dispatch->dispatch_touchcancel( toGuiTouchs(event, false) );
						break;
				}
			}
		}

		static bool getGuiTouch(AInputEvent* motion_event, int pointer_index, UITouch* out) {
			Vec2 scale = application->_host->display_port()->scale();
			float left = application->_rect.origin.x();
			int id = AMotionEvent_getPointerId(motion_event, pointer_index);
			float x = AMotionEvent_getX(motion_event, pointer_index) - left;
			float y = AMotionEvent_getY(motion_event, pointer_index);
			float pressure = AMotionEvent_getPressure(motion_event, pointer_index);
			float h_x = AMotionEvent_getHistoricalX(motion_event, pointer_index, 0);
			float h_y = AMotionEvent_getHistoricalY(motion_event, pointer_index, 0);
			
			*out = {
				uint(id + 20170820),
				0, 0,
				x / scale.x(),
				y / scale.y(),
				pressure,
				false,
				nullptr,
			};
			return x != h_x || y != h_y;
		}

		static List<UITouch> toGuiTouchs(AInputEvent* motion_event, bool filter) {
			List<UITouch> rv;
			UITouch touch;
			int count = AMotionEvent_getPointerCount(motion_event);

			for (int i = 0; i < count; i++) {
				if ( filter ) {
					if ( getGuiTouch(motion_event, i, &touch) ) {
						rv.push(touch);
					}
				} else {
					getGuiTouch(motion_event, i, &touch);
					rv.push(touch);
				}
			}
			return rv;
		}

		// ----------------------------------------------------------------------

		void pending() {
			ANativeActivity_finish(_activity);
		}

	private:
		ANativeActivity* _activity;
		ANativeWindow* _window;
		AppInl* _host;
		AInputQueue* _queue;
		ALooper* _looper;
		RenderLooper* _render_looper;
		EventDispatch* _dispatch;
		Orientation _current_orientation;
		Rect _rect;
		// Mutex _mutex;
		bool _is_init_ok;
	};

	/**
	* @func pending() 挂起应用进程
	*/
	void Application::pending() {
		if ( application ) {
			application->pending();
		}
	}

	/**
	* @func open_url()
	*/
	void Application::open_url(cString& url) {
		API::open_url(url);
	}

	/**
	* @func send_email
	*/
	void Application::send_email(cString& recipient,
																	cString& subject,
																	cString& cc, cString& bcc, cString& body) {
		API::send_email(recipient, subject, cc, bcc, body);
	}

	void AppInl::initialize(cJSON& options) {
		Qk_Assert(!gl_draw_context);
		gl_draw_context = GLDrawProxy::create(this, options);
		_draw_ctx = gl_draw_context->host();
	}

	/**
	* @func ime_keyboard_open
	*/
	void AppInl::ime_keyboard_open(KeyboardOptions options) {
		API::ime_keyboard_open(options.is_clear, int(options.type), int(options.return_type));
	}

	/**
	* @func ime_keyboard_can_backspace
	*/
	void AppInl::ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
		API::ime_keyboard_can_backspace(can_backspace, can_delete);
	}

	/**
	* @func ime_keyboard_close
	*/
	void AppInl::ime_keyboard_close() {
		API::ime_keyboard_close();
	}

	/**
	* @func ime_keyboard_spot_location
	*/
	void AppInl::ime_keyboard_spot_location(Vec2 location) {
		// TODO...
	}

	/**
	* @func set_volume_up()
	*/
	void AppInl::set_volume_up() {
		API::set_volume_up();
	}

	/**
	* @func set_volume_down()
	*/
	void AppInl::set_volume_down() {
		API::set_volume_down();
	}

	/**
	* @func default_atom_pixel
	*/
	float Display::default_atom_pixel() {
		float v = API::get_display_scale();
		return 1.0f / v;
	}

	/**
	* @func keep_screen(keep)
	*/
	void Display::keep_screen(bool keep) {
		API::keep_screen(keep);
	}

	/**
	* @func status_bar_height()
	*/
	float Display::status_bar_height() {
		return API::get_status_bar_height() / _scale_value[1];
	}

	/**
	* @func default_status_bar_height
	*/
	float Display::default_status_bar_height() {
		if (application && application->host()) {
			return application->host()->display_port()->status_bar_height();
		} else {
			return 20;
		}
	}

	/**
	* @func set_visible_status_bar(visible)
	*/
	void Display::set_visible_status_bar(bool visible) {
		API::set_visible_status_bar(visible);
	}

	/**
	* @func set_status_bar_text_color(color)
	*/
	void Display::set_status_bar_style(StatusBarStyle style) {
		API::set_status_bar_style(style);
	}

	/**
	* @func request_fullscreen(fullscreen)
	*/
	void Display::request_fullscreen(bool fullscreen) {
		API::request_fullscreen(fullscreen);
	}

	/**
	* @func orientation()
	*/
	Orientation Display::orientation() {
		Orientation r = application->orientation();
		if ( r == ORIENTATION_INVALID )
			r = (Orientation)API::get_orientation();
		if ( r >= ORIENTATION_INVALID && r <= ORIENTATION_REVERSE_LANDSCAPE ) {
			return r;
		}
		return ORIENTATION_INVALID;
	}

	/**
	* @func set_orientation(orientation)
	*/
	void Display::set_orientation(Orientation orientation) {
		API::set_orientation(int(orientation));
	}

	extern "C" {

		Qk_Export void Java_org_quark_IMEHelper_dispatchIMEDelete(JNIEnv* env, jclass clazz, jint count) {
			shared_app()->dispatch()->dispatch_ime_delete(count);
		}

		Qk_Export void Java_org_quark_IMEHelper_dispatchIMEInsert(JNIEnv* env, jclass clazz, jstring text) {
			shared_app()->dispatch()->dispatch_ime_insert(JNI::jstring_to_string(text));
		}

		Qk_Export void Java_org_quark_IMEHelper_dispatchIMEMarked(JNIEnv* env, jclass clazz, jstring text) {
			shared_app()->dispatch()->dispatch_ime_marked(JNI::jstring_to_string(text));
		}

		Qk_Export void Java_org_quark_IMEHelper_dispatchIMEUnmark(JNIEnv* env, jclass clazz, jstring text) {
			shared_app()->dispatch()->dispatch_ime_unmark(JNI::jstring_to_string(text));
		}
		
		Qk_Export void Java_org_quark_IMEHelper_dispatchKeyboardInput(JNIEnv* env, jclass clazz,
			jint keycode, jboolean ascii, jboolean down, jint repeat, jint device, jint source) {
			shared_app()->dispatch()->keyboard_adapter()->
				dispatch(keycode, ascii, down, repeat, device, source);
		}

		Qk_Export void Java_org_quark_QuarkActivity_onStatucBarVisibleChange(JNIEnv* env, jclass clazz) {
			shared_app()->host()->main_loop()->post(Cb([](Cb::Data& ev) {
				shared_app()->host()->display_port()->Qk_Trigger(change);
			}));
		}

		Qk_Export void ANativeActivity_onCreate(ANativeActivity* activity, 
																						void* savedState, size_t savedStateSize)
		{
			AndroidApplication::onCreate(activity, savedState, savedStateSize);
		}
	}

}