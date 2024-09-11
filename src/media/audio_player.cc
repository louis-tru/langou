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
 * ***** END LICENSE BLOCK *****/

#include "./audio_player.h"
#include "../errno.h"
#include "../util/fs.h"

namespace qk {
	typedef MediaSource::TrackInfo TrackInfo;

	Qk_DEFINE_INLINE_MEMBERS(AudioPlayer, Inl) {
	public:
		#define _this _inl(this)
		#define _inl(self) static_cast<AudioPlayer::Inl*>(self)

		bool write_audio_pcm(uint64_t st) {
			WeakBuffer buff((char*)_audio_buffer.data[0], _audio_buffer.linesize[0]);
			bool r = _pcm->write(buff.buffer());
			if ( !r ) {
				Qk_DEBUG("Discard, audio PCM frame, %lld", _audio_buffer.time);
			} else {
				_prev_presentation_time = st;
			}
			_audio->release(_audio_buffer);
			return r;
		}

		void run_play_audio_loop() {
		 loop:
			uint64_t sys_time = time_monotonic();
			{ //
				ScopeLock scope(_mutex);

				if ( _status == kStop_PlayerStatus ) { // stop
					return; // stop audio
				}

				if ( !_audio_buffer.total ) {
					if ( _status == kPlaying_PlayerStatus || _status == kStart_PlayerStatus ) {
						_audio_buffer = _audio->output();

						if ( _audio_buffer.total ) {
							if ( _waiting_buffer ) {
								_loop->post(Cb([this](auto evt) {
									trigger(UIEvent_WaitBuffer, Float32(1.0F)); /* trigger source WAIT event */
								}));
								_waiting_buffer = false;
							}
						} else {
							MediaSourceStatus status = _source->status();
							if ( status == kWait_MediaSourceStatus ) { // 源..等待数据
								if ( _waiting_buffer == false ) {
									_loop->post(Cb([this](auto evt) {
										trigger(UIEvent_WaitBuffer, Float32(0.0F)); /* trigger source WAIT event */
									}));
									_waiting_buffer = true;
								}
							} else if ( status == kEOF_MediaSourceStatus ) {
								_loop->post(Cb([this](auto evt) { stop(); }));
								return;
							}
						}
						//
					}
				}

				if ( _audio_buffer.total ) {
					uint64_t pts = _audio_buffer.time;
					
					if (_uninterrupted_play_start_systime &&         // 0表示还没开始
							pts &&                                        // 演示时间为0表示开始或(即时渲染如视频电话)
							sys_time - _prev_presentation_time < 300000  // 距离上一帧超过300ms重新记时(如应用程序从休眠中恢复或数据缓冲)
					) {
						int64_t st =  (sys_time - _uninterrupted_play_start_systime) -     // sys
												(pts - _uninterrupted_play_start_time); // frame
						int delay = _audio->frame_interval() * _pcm->compensate();

						if (st >= delay) { // 是否达到播放声音时间。输入pcm到能听到声音会有一些延时,这里设置补偿
							write_audio_pcm(sys_time);
						}
					} else {
						if ( _status == kStart_PlayerStatus ) {
							_status = kPlaying_PlayerStatus;
							_loop->post(Cb([this](auto e) {
								trigger(UIEvent_StartPlay); /* trigger start_play event */
							}));
						}
						_uninterrupted_play_start_systime = sys_time;
						_uninterrupted_play_start_time = _audio_buffer.time;
						write_audio_pcm(sys_time);
					}
				}

				if (_uninterrupted_play_start_systime) {
					_time = sys_time - _uninterrupted_play_start_systime + _uninterrupted_play_start_time;
				}

				_audio->advance();
			}

			int frame_interval = 1000.0 / 30.0 * 1000; // 30fsp
			int64_t sleep_st = frame_interval - time_monotonic() + sys_time;
			if ( sleep_st > 0 ) {
				thread_sleep(sleep_st);
			}

			goto loop;
		}

		void trigger(const UIEventName& type, const Object& data = Object()) {
			Sp<Event<>> evt = New<Event<>>(data);
			Notification::trigger(type, **evt);
		}

		bool stop_from(Lock& lock, bool is_event) {
			if ( _status == kStop_PlayerStatus ) {
				return false;
			}
			_status = kStop_PlayerStatus;
			_uninterrupted_play_start_time = 0;
			_uninterrupted_play_start_systime = 0;
			_prev_presentation_time = 0;
			_time = 0;

			if ( _audio ) {
				_audio->release(_audio_buffer);
				_audio->extractor()->set_disable(true);
				_audio->close();
				Release(_audio); _audio = nullptr;
			}

			PCMPlayer::Traits::Release(_pcm); _pcm = nullptr;

			_source->stop();

			auto loop_id = _run_loop_id;
			lock.unlock();
			thread_join_for(loop_id); // wait audio thread end

			if ( is_event ) {
				_loop->post(Cb([this](auto e) {
					_this->trigger(UIEvent_Stop); /* trigger stop event */
				}));
			}
			lock.lock();
			
			return true;
		}

		void stop_and_release(Lock& lock, bool is_event) {
			if ( _task_id ) {
				_loop->work_cancel(_task_id);
				_task_id = 0;
			}

			stop_from(lock, is_event);

			Release(_audio); _audio = nullptr;
			Release(_source); _source = nullptr;
			PCMPlayer::Traits::Release(_pcm); _pcm = nullptr;

			_time = 0;
			_duration = 0;
		}

		bool is_active() {
			return _status == kPaused_PlayerStatus || _status == kPlaying_PlayerStatus;
		}

		void start_run() {
			Lock lock(_mutex);

			Qk_Assert( _source && _audio && _pcm );
			Qk_Assert( _source->is_active() );
			Qk_Assert_Eq(_status, kStart_PlayerStatus);

			_waiting_buffer = false;

			if ( _audio->open() ) {
				_source->seek(0);
				_audio->release( _audio_buffer );
				_audio->flush();
				_audio->extractor()->set_disable(false);
				_pcm->flush();
				_pcm->set_volume(_volume);
				_pcm->set_mute(_mute);

				_run_loop_id = thread_new([](auto t, void* self) {
					static_cast<Inl*>(self)->run_play_audio_loop();
				}, this, "audio_loop");
			} else {
				stop_from(lock, true);
			}
		}
	};

	AudioPlayer::AudioPlayer()
		: _source(nullptr)
		, _pcm(nullptr)
		, _audio(nullptr)
		, _loop(RunLoop::current())
		, _status(kStop_PlayerStatus)
		, _audio_buffer()
		, _duration(0), _time(0)
		, _uninterrupted_play_start_time(0)
		, _uninterrupted_play_start_systime(0)
		, _prev_presentation_time(0)
		, _task_id(0)
		, _volume(100)
		, _mute(false)
		, _auto_play(true)
		, _disable_wait_buffer(false)
		, _waiting_buffer(false)
	{
		Qk_Fatal_Assert(_loop, "AudioPlayer::AudioPlayer(), cannot find the current run loop object");
	}

	void AudioPlayer::media_source_ready(MediaSource* src) {
		Qk_Assert_Eq(_source, src);

		if (_audio) {
			_this->trigger(UIEvent_Ready); // trigger event ready
			if ( _status == kStart_PlayerStatus ) {
				_this->start_run();
			}
			return;
		}

		// Creating a decoder is time-consuming and can cause delays in the main thread, 
		// so the task is sent to the worker thread
		struct Running: CallbackCore<Object> {
			void create() {
				auto self = *hold;
				if (self->_source != src) return; // 源已被更改,所以取消

				auto audio = MediaCodec::create(kAudio_MediaType, self->_source);
				auto pcm = self->_pcm;

				if ( audio && !self->_pcm ) {
					pcm = PCMPlayer::create(audio->channel_count(),
																	audio->extractor()->track(0).sample_rate );
				}
				ScopeLock scope(self->_mutex);
				self->_pcm = pcm;

				if ( self->_source != src ) {
					Release(audio);
				} else {
					self->_audio = audio;
				}
			}
			void call(Data& evt) override { // done
				auto self = *hold;
				self->_task_id = 0; //
				if ( self->_source != src ) return;

				if (self->_audio && self->_pcm) {
					{ //
						ScopeLock scope(self->_mutex);
						self->_duration = self->_source->duration();
					}
					_inl(self)->trigger(UIEvent_Ready); // trigger event ready

					if ( self->_status == kStart_PlayerStatus ) {
						_inl(self)->start_run();
					} else {
						if (self->_auto_play) {
							self->start();
						}
					}
				} else {
					Error e(ERR_AUDIO_NEW_CODEC_FAIL, "Unable to create video decoder");
					Qk_ERR("%s", *e.message());
					_inl(self)->trigger(UIEvent_Error, e); // trigger event error
					self->stop();
				}
			}
			Running(AudioPlayer *self, MediaSource* src): hold(self), src(src) {
				self->_task_id =
				self->_loop->work(Cb((Cb::Member<Running>)&Running::create, this), this);
			}
			Sp<AudioPlayer> hold;
			MediaSource*    src;
		};

		New<Running>(this, src);
	}

	void AudioPlayer::media_source_wait_buffer(MediaSource* so, float process) {
		if ( _waiting_buffer ) {/* 开始等待数据缓存不触发事件,因为在解码器队列可能还存在数据,
															* 所以等待解码器也无法输出数据时再触发事件
															*/
			if ( process < 1.0 ) {
				// trigger event wait_buffer
				_this->trigger(UIEvent_WaitBuffer, Float32(process));
			}
		}
	}

	void AudioPlayer::media_source_eof(MediaSource* so) {
		_this->trigger(UIEvent_SourceEnd); // trigger event eof
	}

	void AudioPlayer::media_source_error(MediaSource* so, cError& err) {
		_this->trigger(UIEvent_Error, err); // trigger event error
		stop();
	}

	AudioPlayer::~AudioPlayer() {
		Lock lock(_mutex);
		_this->stop_and_release(lock, false);
	}

	bool AudioPlayer::auto_play() {
		ScopeLock scope(_mutex);
		return _auto_play;
	}

	bool AudioPlayer::mute() {
		ScopeLock scope(_mutex);
		return _mute;
	}

	bool AudioPlayer::disable_wait_buffer() {
		ScopeLock scope(_mutex);
		return _disable_wait_buffer;
	}

	uint32_t AudioPlayer::volume() {
		ScopeLock scope(_mutex);
		return _volume;
	}

	uint64_t AudioPlayer::duration() {
		ScopeLock scope(_mutex);
		return _duration;
	}

	uint64_t AudioPlayer::time() {
		ScopeLock scope(_mutex);
		return _time;
	}

	/**
	 * @method set_auto_play() setting auto play
	 */
	void AudioPlayer::set_auto_play(bool value) {
		ScopeLock scope(_mutex);
		_auto_play = value;
	}

	String AudioPlayer::src() {
		ScopeLock lock(_mutex);
		if ( _source ) {
			return _source->uri().href();
		} else {
			return String(); //::empty;
		}
	}

	void AudioPlayer::set_src(String value) {
		if ( value.isEmpty() ) {
			return;
		}
		String src = fs_reader()->format(value);

		Lock lock(_mutex);

		if ( _source ) {
			if ( _source->uri().href() == src ) {
				return;
			}
			_this->stop_and_release(lock, true);
		}
		_source = new MediaSource(src, _loop);
		_source->set_delegate(this);
		_source->disable_wait_buffer(_disable_wait_buffer);
		_source->start();
	}

	void AudioPlayer::start() {
		Lock scope(_mutex);
		if ( _status == kStop_PlayerStatus && _source ) {
			_status = kStart_PlayerStatus;
			_uninterrupted_play_start_time = 0;
			_uninterrupted_play_start_systime = 0;
			_prev_presentation_time = 0;
			_time = 0;
			_source->start();
			if ( _audio && _pcm ) {
				if ( _source->is_active() ) {
					scope.unlock();
					_this->start_run();
				}
			}
		}
	}

	void AudioPlayer::stop() {
		Lock lock(_mutex);
		_this->stop_from(lock, true);
	}

	/**
	 * @method source_status()
	 */
	MediaSourceStatus AudioPlayer::source_status() {
		ScopeLock scope(_mutex);
		if ( _source ) {
			return _source->status();
		}
		return kUninitialized_MediaSourceStatus;
	}

	/**
	 * @method status() getting play status
	 */
	PlayerStatus AudioPlayer::status() {
		ScopeLock scope(_mutex);
		return _status; 
	}

	/**
	 * @method seek to target time
	 */
	bool AudioPlayer::seek(uint64_t timeUs) {
		ScopeLock scope(_mutex);
		if ( _this->is_active() && timeUs < _duration ) {
			Qk_Assert(_source);
			if ( _source->seek(timeUs) ) {
				_uninterrupted_play_start_systime = 0;
				_time = timeUs;
				_audio->release( _audio_buffer );
				_audio->flush();
				_pcm->flush();
				_loop->post(Cb([this](auto e){
					_this->trigger(UIEvent_Seek, Uint64(_time)); // trigger seek event
				}));
				return true;
			}
		}
		return false;
	}

	/**
	 * @method pause play
	 */
	void AudioPlayer::pause() {
		ScopeLock scope(_mutex);
		if ( _status == kPlaying_PlayerStatus && _duration /*没有长度信息不能暂停*/ ) {
			_status = kPaused_PlayerStatus;
			_uninterrupted_play_start_systime = 0;
			_loop->post(Cb([this](auto e){
				_this->trigger(UIEvent_Pause); // trigger pause event
			}));
		}
	}

	/**
	 * @method resume() play
	 */
	void AudioPlayer::resume() {
		ScopeLock scope(_mutex);
		if ( _status == kPaused_PlayerStatus ) {
			_status = kPlaying_PlayerStatus;
			_uninterrupted_play_start_systime = 0;
			_loop->post(Cb([this](auto e){
				_this->trigger(UIEvent_Resume); // trigger resume event
			}));
		}
	}

	/**
	 * @method set_mute() setting mute status
	 */
	void AudioPlayer::set_mute(bool value) {
		ScopeLock scope(_mutex);
		if ( value != _mute ) {
			_mute = value;
			if ( _pcm ) { // action
				_pcm->set_mute(_mute);
			}
		}
	}

	void AudioPlayer::set_volume(uint32_t value) {
		ScopeLock scope(_mutex);
		value = Qk_MIN(value, 100);
		_volume = value;
		if ( _pcm ) {
			_pcm->set_volume(value);
		}
	}

	void AudioPlayer::set_disable_wait_buffer(bool value) {
		ScopeLock scope(_mutex);
		_disable_wait_buffer = value;
		if (_source) {
			_source->disable_wait_buffer(value);
		}
	}

	uint32_t AudioPlayer::audio_track_count() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return _audio->extractor()->track_count();
		}
		return 0;
	}

	uint32_t AudioPlayer::audio_track_index() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return _audio->extractor()->track_index();
		}
		return 0;
	}

	const TrackInfo* AudioPlayer::audio_track() {
		ScopeLock lock(_mutex);
		if ( _audio ) {
			return &_audio->extractor()->track();
		}
		return nullptr;
	}

	const TrackInfo* AudioPlayer::audio_track_at(uint32_t index) {
		ScopeLock lock(_mutex);
		if ( _audio && index < _audio->extractor()->track_count() ) {
			return &_audio->extractor()->track(index);
		}
		return nullptr;
	}

	void AudioPlayer::select_audio_track(uint32_t index) {
		ScopeLock scope(_mutex);
		if ( _audio && index < _audio->extractor()->track_count() ) {
			_audio->extractor()->select_track(index);
		}
	}

}
