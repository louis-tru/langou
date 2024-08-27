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

#include "./action.h"
#include "../view/view.h"
#include "../../errno.h"
#include "../window.h"
#include "../app.h"
#include "../event.h"

#define _async_call _window->preRender().async_call

namespace qk {

	const Action::Id playingFlag(reinterpret_cast<List<qk::Action *>::Node*>(1));

	Action::Action(Window *win)
		: _window(win)
		, _loop(0)
		, _duration(0)
		, _speed(1)
		, _parent(nullptr)
		, _target(nullptr)
		, _looped(0)
	{
		Qk_ASSERT(win);
	}

	Action::~Action() {
		Qk_ASSERT( _id == Id() );
		_window = nullptr;
	}

	void Action::release_for_only_center_Rt() {
		Qk_ASSERT(refCount() == 1);
		_window = nullptr;
		release();
	}

	void Action::destroy() {
		_duration = 0;
		clear();
		if (_window) {
			_async_call([](auto self, auto arg) {
				// To ensure safety and efficiency,
				// it should be Completely destroyed in RT (render thread)
				self->Object::destroy();
			}, this, 0);
			_window = nullptr;
		} else {
			Object::destroy();
		}
	}

	bool Action::playing() const {
		return _parent ? _parent->playing(): _id != Id();
	}

	void Action::set_playing(bool val) {
		if (val)
			play();
		else
			stop();
	}

	void Action::set_speed(float value) {
		_speed = Qk_MIN(10, Qk_MAX(value, 0.1));
	}

	void Action::set_loop(uint32_t value) {
		_loop = value;
	}

	void Action::seek_play(uint32_t time) {
		seek(time);
		play();
	}

	void Action::seek_stop(uint32_t time) {
		seek(time);
		stop();
	}

	void Action::seek(uint32_t time) {
		_async_call([](auto self, auto arg) { self->seek_Rt(arg.arg); }, this, time);
	}

	void Action::play() {
		if (_parent) {
			_parent->play();
		} else {
			if (_target && _id == Id()) {
				_id = playingFlag;
				_async_call([](auto self, auto arg) { self->play_Rt(); }, this, 0);
			}
		}
	}

	void Action::stop() {
		if (_parent) {
			_parent->stop();
		} else {
			if (_id != Id()) {
				_async_call([](auto self, auto arg) { self->stop_Rt(); }, this, 0);
			}
		}
	}

	void Action::play_Rt() {
		if (_parent == nullptr && _target) {
			if (_id == Id() || _id == playingFlag) {
				auto id = _window->actionCenter()->_actions_Rt.pushBack({this,false});
				_id = *reinterpret_cast<Id*>(&id);
			}
		}
	}

	void Action::stop_Rt() {
		if (_parent == nullptr) {
			if (_id != Id() && _id != playingFlag) {
				typedef List<ActionCenter::Action_Wrap>::Iterator Id1;
				_window->actionCenter()->_actions_Rt.erase(*reinterpret_cast<Id1*>(&_id));
				_id = Id();
			}
		}
	}

	void Action::seek_Rt(uint32_t time) {
		time = Qk_MIN(time, _duration);
		auto parent = dynamic_cast<ActionGroup*>(_parent); // safe use parent ptr
		if (parent) {
			parent->seek_before_Rt(time, this);
		} else {
			seek_time_Rt(time, this);
		}
	}

	void ActionGroup::append(Action* child) {
		Qk_ASSERT(child);
		if (child->set_parent(this) == 0) {
			insertChild(_actions_Rt.end(), child);
		}
	}

	void Action::before(Action *act) {
		Qk_ASSERT(act);
		Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::before, illegal parent empty");
		if (act->set_parent(_parent) == 0) {
			_parent->insertChild(_id, act);
		}
	}

	void Action::after(Action *act) {
		Qk_ASSERT(act);
		Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::after, illegal parent empty");
		if (act->set_parent(_parent) == 0) {
			auto id = _id;
			_parent->insertChild(++id, act);
		}
	}

	void Action::remove() {
		// Qk_Check(_parent, ERR_ACTION_ILLEGAL_PARENT, "Action::remove, illegal parent empty");
		if (_parent == nullptr) {
			return Qk_ERR("Action::remove, illegal parent empty");
		}
		del_parent();
		_parent->removeChild(_id);
	}

	void Action::set_target(View *target) {
		Qk_ASSERT(target);
		Qk_ASSERT(target->window() == _window);
		if (_parent)
			return Qk_ERR("Action::set_target, cannot set non root action");
		if (_target)
			return Qk_ERR("Action::set_target, action cannot set multiple target");
		_target = target;
	}

	void Action::del_target(View* target) {
		Qk_ASSERT(target);
		if (target == _target) {
			stop(); // stop action
			_target = nullptr;
		}
	}

	int Action::set_parent(ActionGroup *parent) {
		if (parent->_window != _window) {
			Qk_ERR("Action::set_parent_Rt, set action window not match");
			return ERR_ACTION_SET_WINDOW_NO_MATCH;
		}
		// Qk_Check(!_parent, ERR_ACTION_ILLEGAL_CHILD, "illegal child action");
		if (_parent) {
			Qk_ERR("Action::set_parent, illegal child action");
			return ERR_ACTION_ILLEGAL_CHILD;
		}
		if (_id != Id()) {
			Qk_ERR("Action::set_parent, action playing state conflict");
			return ERR_ACTION_PLAYING_CONFLICT;
		}
		_parent = parent;
		retain(); // retain for parent
		return 0;
	}

	void Action::del_parent() {
		Qk_ASSERT(_parent);
		_parent = nullptr;
		release(); // release for parent
	}

	void Action::setDuration(int32_t diff) {
		_duration += diff;
		if (_parent) {
			_parent->setDuration(diff);
		}
	}

	void SpawnAction::setDuration(int32_t diff) {
		int32_t new_duration = 0;
		for ( auto &i : _actions ) {
			new_duration = Qk_MAX(i.key->_duration, new_duration);
		}
		diff = new_duration - _duration;
		if ( diff ) {
			Action::setDuration(diff);
		}
	}

	void Action::trigger_ActionLoop_Rt(uint32_t delay, Action* root) {
		auto v = root->_target;
		auto loop = _loop;
		v->preRender().post(Cb([this,v,delay,loop](auto& e) {
			Sp<ActionEvent> h(new ActionEvent(this, v, delay, 0, loop));
			v->trigger(UIEvent_ActionLoop, **h);
		}), v);
	}

	void Action::trigger_ActionKeyframe_Rt(uint32_t delay, uint32_t frameIndex, Action* root) {
		auto v = root->_target;
		auto loop = _loop;
		v->preRender().post(Cb([this,v,delay,frameIndex,loop](auto&e) {
			Sp<ActionEvent> h(new ActionEvent(this, v, delay, frameIndex, loop));
			v->trigger(UIEvent_ActionKeyframe, **h);
		}), v);
	}

}
