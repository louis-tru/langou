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

#include "./_event.h"
#include "./_view.h"
#include "../../media/media-codec.h"
#include "../../media/audio-player.h"

/**
 * @ns qk::js
 */

Js_BEGIN

typedef MultimediaSource::TrackInfo TrackInfo;

Local<JSValue> inl_track_to_jsvalue(const TrackInfo* track, Worker* worker) {
	if ( ! track ) {
		return worker->NewNull();
	}

	/**
	 * @object TrackInfo
	 * type {meun MediaType}
	 * mime {String}
	 * codecId {int}
	 * codecTag {uint}
	 * format {int}
	 * profile {int}
	 * level {int}
	 * width {uint}
	 * height {uint}
	 * language {String}
	 * bitrate {uint}
	 * sampleRate {uint}
	 * channelCount {uint64}
	 * channelLayout {uint}
	 * frameInterval {uint} ms
	 * @end
	 */
	Local<JSObject> obj = worker->NewObject().To<JSObject>();
	obj->Set(worker, worker->New("type",1), worker->New(track->type));
	obj->Set(worker, worker->New("mime",1), worker->New(track->mime));
	obj->Set(worker, worker->New("codecId",1), worker->New(track->codec_id));
	obj->Set(worker, worker->New("codecTag",1), worker->New(track->codec_tag));
	obj->Set(worker, worker->New("format",1), worker->New(track->format));
	obj->Set(worker, worker->New("profile",1), worker->New(track->profile));
	obj->Set(worker, worker->New("level",1), worker->New(track->level));
	obj->Set(worker, worker->New("width",1), worker->New(track->width));
	obj->Set(worker, worker->New("height",1), worker->New(track->height));
	obj->Set(worker, worker->New("language",1), worker->New(track->language));
	obj->Set(worker, worker->New("bitrate",1), worker->New(track->bitrate));
	obj->Set(worker, worker->New("sampleRate",1), worker->New(track->sample_rate));
	obj->Set(worker, worker->New("channelCount",1), worker->New(track->channel_count));
	obj->Set(worker, worker->New("channelLayout",1), worker->New(track->channel_layout));
	obj->Set(worker, worker->New("frameInterval",1), worker->New(track->frame_interval / 1000.0));
	
	return obj.To<JSValue>();
}

template<class T, class Self>
static void addEventListener_1(Wrap<Self>* wrap, const UIEventName& type, 
																 cString& func, int id, Cast* cast = nullptr) 
{
	auto f = [wrap, func, cast](typename Self::EventType& evt) {
		auto worker = wrap->worker();
		Js_Handle_Scope();
		Js_Callback_Scope();

		// arg event
		Wrap<T>* ev = Wrap<T>::pack(static_cast<T*>(&evt), Js_Typeid(T));
		
		if (cast)
			ev->setPrivateData(cast); // set data cast func
		
		Local<JSValue> args[2] = { ev->that(), worker->New(true) };
		// call js trigger func
		Local<JSValue> r = wrap->call( worker->New(func,1), 2, args );
		
		// test:
		//if (r->IsNumber(worker)) {
		//  Qk_LOG("--------------number,%s", *r->ToStringValue(wrap->worker()));
		//} else {
		//  Qk_LOG("--------------string,%s", *r->ToStringValue(wrap->worker()));
		//}
	};
	
	Self* self = wrap->self();
	self->add_event_listener(type, f, id);
}

/**
 * @class WrapAudioPlayer
 */
class WrapAudioPlayer: public WrapObject {
	public:

	/**
	 * @func overwrite
	 */
	virtual bool addEventListener(cString& name_s, cString& func, int id) {
		auto i = UI_EVENT_TABLE.find(name_s);
		if ( i.is_null() || !(i.value().flag() & UI_EVENT_FLAG_PLAYER) ) {
			return false;
		}
		
		UIEventName name = i.value();
		auto wrap = reinterpret_cast<Wrap<AudioPlayer>*>(this);

		switch ( UI_EVENT_FLAG_CAST & name.flag() ) {
			case UI_EVENT_FLAG_ERROR: cast = Cast::Entity<Error>(); break;
			case UI_EVENT_FLAG_FLOAT: cast = Cast::Entity<Float>(); break;
			case UI_EVENT_FLAG_UINT64: cast = Cast::Entity<Uint64>(); break;
		}

		addEventListener_1<Event<>>(wrap, name, func, id, cast);

		return true;
	}
	
	virtual bool removeEventListener(cString& name, int id) {
		auto i = UI_EVENT_TABLE.find(name);
		if ( i.is_null() || !(i.value().flag() & UI_EVENT_FLAG_PLAYER) ) {
			return false;
		}
		auto wrap = reinterpret_cast<Wrap<AudioPlayer>*>(this);
		wrap->self()->remove_event_listener(i.value(), id); // off event listener
		return true;
	}
	
	/**
	 * @constructor([src])
	 * @arg [src] {String}
	 */
	static void constructor(FunctionCall args) {
		Js_Worker(args);

		AudioPlayer* player = nullptr;
		if ( args.Length() > 0 && args[0]->IsString(worker) ) {
			String uri = args[0]->ToStringValue(worker);
			player = static_cast<AudioPlayer*>(module_audio_player->create(&uri));
		} else {
			player = static_cast<AudioPlayer*>(module_audio_player->create(nullptr));
		}
		if (!player) {
			Js_Throw("create AudioPlayer fail");
		}
		New<WrapAudioPlayer>(args, player);
	}
	
	/**
	 * @get auto_play {bool}
	 */
	static void auto_play(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->auto_play() );
	}
	
	/**
	 * @set auto_play {bool}
	 */
	static void set_auto_play(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->set_auto_play( value->ToBooleanValue(worker) );
	}
	
	/**
	 * @get source_status {enum MultimediaSourceStatus}
	 */
	static void source_status(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->source_status() );
	}
	
	/**
	 * @get status {enum PlayerStatus}
	 */
	static void status(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->status() );
	}
	
	/**
	 * @get mute {bool}
	 */
	static void mute(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->mute() );
	}
	
	/**
	 * @set mute {bool}
	 */
	static void set_mute(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->set_mute( value->ToBooleanValue(worker) );
	}
	
	/**
	 * @get volume {uint} 0-100
	 */
	static void volume(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->mute() );
	}
	
	/**
	 * @set volume {uint} 0-100
	 */
	static void set_volume(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw("* @set volume {uint} 0-100");
		}
		Js_Self(AudioPlayer);
		self->set_volume( value->ToNumberValue(worker) );
	}
	
	/**
	 * @get src {String}
	 */
	static void src(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->src() );
	}
	
	/**
	 * @set src {String}
	 */
	static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->set_src( value->ToStringValue(worker) );
	}
	
	/**
	 * @get time {uint64} ms
	 */
	static void time(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->time() / 1000.0 );
	}
	
	/**
	 * @get duration {uint64} ms
	 */
	static void duration(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->duration() / 1000.0 );
	}
	
	/**
	 * @get track_index {uint}
	 */
	static void track_index(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->track_index() );
	}
	
	/**
	 * @get track_count {uint}
	 */
	static void track_count(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->track_count() );
	}
	
	/**
	 * @func select_track(index)
	 * @arg index {uint} audio track index
	 */
	static void select_track(FunctionCall args) {
		Js_Worker(args); UILock lock;
		if (args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
			Js_Throw(
				"* @func selectTrack(index)\n"
				"* @arg index {uint} audio track index\n"
			);
		}
		Js_Self(AudioPlayer);
		self->select_track( args[0]->ToUint32Value(worker) );
	}
	
	/**
	 * @func track([index])
	 * @arg [track=curent_track] {uint} default use current track index
	 * @ret {object TrackInfo}
	 */
	static void track(FunctionCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		if (args.Length() < 1 || !args[0]->IsUint32(worker) ) {
			Js_Return( inl_track_to_jsvalue(self->track(), worker) );
		} else {
			Js_Return( inl_track_to_jsvalue(self->track(args[0]->ToUint32Value(worker)), worker) );
		}
	}
	
	/**
	 * @func start()
	 */
	static void start(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->start();
	}
	
	/**
	 * @func seek(time)
	 * @arg time {uint} ms
	 * @ret {bool} success
	 */
	static void seek(FunctionCall args) {
		Js_Worker(args); UILock lock;
		if (args.Length() < 1 || ! args[0]->IsNumber(worker) ) {
			Js_Throw(
				"* @func seek(time)\n"
				"* @arg time {uint} ms\n"
				"* @ret {bool} success\n"
			);
		}
		Js_Self(AudioPlayer);
		Js_Return( self->seek( args[0]->ToNumberValue(worker) * 1000.0 ));
	}
	
	/**
	 * @func pause()
	 */
	static void pause(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->pause();
	}
	
	/**
	 * @func resume()
	 */
	static void resume(FunctionCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->resume();
	}
	
	/**
	 * @func stop()
	 */
	static void stop(FunctionCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		self->stop();
	}
	
	/**
	 * @get disable_wait_buffer {bool}
	 */
	static void disable_wait_buffer(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(AudioPlayer);
		Js_Return( self->disable_wait_buffer() );
	}
	
	/**
	 * @set disable_wait_buffer {bool}
	 */
	static void set_disable_wait_buffer(Local<JSString> name,
																			Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(AudioPlayer);
		self->disable_wait_buffer( value->ToBooleanValue(worker) );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		
		Js_NEW_CLASS_FROM_ID(AudioPlayer, module_audio_player->tid, constructor, {
			Js_Set_Class_Accessor(autoPlay, auto_play, set_auto_play);
			Js_Set_Class_Accessor(sourceStatus, source_status);
			Js_Set_Class_Accessor(status, status);
			Js_Set_Class_Accessor(mute, mute, set_mute);
			Js_Set_Class_Accessor(volume, volume, set_volume);
			Js_Set_Class_Accessor(src, src, set_src);
			Js_Set_Class_Accessor(time, time);
			Js_Set_Class_Accessor(duration, duration);
			Js_Set_Class_Accessor(audioTrackIndex, track_index);
			Js_Set_Class_Accessor(audioTrackCount, track_count);
			Js_Set_Class_Accessor(disableWaitBuffer,
														disable_wait_buffer, set_disable_wait_buffer);
			Js_Set_Class_Method(selectAudioTrack, select_track);
			Js_Set_Class_Method(audioTrack, track);
			Js_Set_Class_Method(start, start);
			Js_Set_Class_Method(seek, seek);
			Js_Set_Class_Method(pause, pause);
			Js_Set_Class_Method(resume, resume);
			Js_Set_Class_Method(stop, stop);
		}, 0);
		cls->Export(worker, "AudioPlayer", exports);
	}
};

void binding_audio(Local<JSObject> exports, Worker* worker) {
	WrapAudioPlayer::binding(exports, worker);
}

Js_END
