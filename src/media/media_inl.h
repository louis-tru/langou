// @private head
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

#ifndef __quark__media_inl__
#define __quark__media_inl__

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
#include "./media.h"
#include "../util/thread.h"

namespace qk {
	typedef MediaSource::Program   Program;
	typedef MediaSource::Extractor Extractor;
	typedef MediaSource::Inl       Inl;
	typedef MediaSource::Stream    AvStream;

	class SoftwareMediaCodec;

	class MediaSource::Inl {
	public:
		Inl(MediaSource*, cString& uri);
		~Inl();

		/**
		* @method set_delegate
		*/
		void set_delegate(Delegate* delegate);

		/**
		* @method switch_program
		*/
		bool switch_program(uint32_t index);

		/**
		* @method extractor
		*/
		Extractor* extractor(MediaType type);

		/**
		* @method seek
		* */
		bool seek(uint64_t timeUs);

		/**
		* @method start running
		* */
		void open();

		/**
		* @method stop running
		* */
		void stop();

	private:
		void read_stream(cThread *t, AVFormatContext* fmt_ctx, cString& uri);
		bool packet_push(AVPacket& avpkt);
		Packet* advance(Extractor* ex);
		void trigger_fferr(int err, cChar *f, ...);
		void trigger_error(cError& err);
		void thread_abort();
		void switch_program_by(uint32_t index);
		bool switch_stream(Extractor *ex, uint32_t index);
		bool advance_eof();

		ThreadID               _tid;
		MediaSource*           _host;
		URI                    _uri;
		MediaSourceStatus      _status;
		Delegate*              _delegate;
		uint32_t               _program_idx;
		Array<Program>         _programs;
		Dict<int, Extractor*>  _extractors; // MediaType => Extractor*
		Extractor             *_video_ex, *_audio_ex;
		uint64_t               _duration, _seek, _fixed_packet_duration;
		AVFormatContext*       _fmt_ctx;
		Mutex                  _mutex;

		friend class MediaSource;
		friend class Extractor;
		friend class MediaCodec;
		friend class SoftwareMediaCodec;
	};

}
#endif
