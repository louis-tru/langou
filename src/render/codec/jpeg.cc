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

#include "./codec.h"
#include <setjmp.h>

extern "C" {
# include <jpeglib.h>
}

namespace qk {

	struct JPEGClientData {
		jmp_buf jmpbuf;
	};

	static void jpeg_error_output(j_common_ptr cinfo) {
		Qk_ERR("%s", "Invalid JPEG file structure: missing SOS marker");
		JPEGClientData* data = (JPEGClientData*)cinfo->client_data;
		longjmp(data->jmpbuf, 1);
	}

	bool img_jpeg_decode(cBuffer& data, Array<Pixel> *rv) {
		struct jpeg_decompress_struct jpeg;
		struct jpeg_error_mgr jerr;
		jpeg.err = jpeg_std_error(&jerr);
		jerr.error_exit = jpeg_error_output;
		
		JPEGClientData client_data;
		jpeg.client_data = &client_data;
		jpeg_create_decompress(&jpeg);

		jpeg_mem_src(&jpeg, (uint8_t*)*data, data.length());
		
		if ( setjmp(client_data.jmpbuf) ) {
			jpeg_destroy_decompress(&jpeg);
			return rv;
		}
		
		jpeg_read_header(&jpeg, TRUE);

		uint32_t w = jpeg.image_width;
		uint32_t h = jpeg.image_height;
		int num = jpeg.num_components;
		JSAMPROW row;

		jpeg_start_decompress(&jpeg);

		if (num == 1) {
			uint32_t rowbytes = w * num;
			uint32_t count = h * rowbytes;
			auto buff = Buffer::alloc(count);

			while(jpeg.output_scanline < jpeg.output_height) {
				row = (JSAMPROW)*buff + jpeg.output_scanline * rowbytes;
				jpeg_read_scanlines(&jpeg, &row, 1);
			}

			rv->push( Pixel(PixelInfo(w, h, kColor_Type_Gray_8, kAlphaType_Opaque), buff) );

		} else if (num == 3) {
			uint32_t rowbytes = w * 4;
			uint32_t count = h * rowbytes;
			auto buff = Buffer::alloc(count);

			JSAMPARRAY lines = (*jpeg.mem->alloc_sarray)((j_common_ptr) &jpeg, JPOOL_IMAGE, w * num, 1);

			while(jpeg.output_scanline < jpeg.output_height) {
				row = (JSAMPROW)*buff + jpeg.output_scanline * rowbytes;
				jpeg_read_scanlines(&jpeg, lines, 1);

				JSAMPROW row2 = lines[0];

				for (uint32_t column = 0; column < w; column++) {
					*((int*)row) = *((int*)row2);
					row[3] = 255;
					row += 4; row2 += 3;
				}
			}

			rv->push( Pixel(PixelInfo(w, h, kColor_Type_RGB_888X, kAlphaType_Opaque), buff) );
		} else {
			return false;
		}

		jpeg_start_decompress(&jpeg);
		jpeg_destroy_decompress(&jpeg);

		return true;
	}

	bool img_jpeg_test(cBuffer& data, PixelInfo *out) {
		struct jpeg_decompress_struct jpeg;
		struct jpeg_error_mgr jerr;
		jpeg.err = jpeg_std_error(&jerr);
		jerr.error_exit = jpeg_error_output;
		
		JPEGClientData client_data;
		jpeg.client_data = &client_data;
		jpeg_create_decompress(&jpeg);
		
		CPointer<jpeg_decompress_struct> clear(&jpeg, [](jpeg_decompress_struct *jpeg) {
			jpeg_destroy_decompress(jpeg);
		});

		jpeg_mem_src(&jpeg, (uint8_t*)*data, data.length());
		
		if ( setjmp(client_data.jmpbuf) ) {
			return false;
		}
		
		jpeg_read_header(&jpeg, TRUE);
		
		uint32_t w = jpeg.image_width;
		uint32_t h = jpeg.image_height;
		int num = jpeg.num_components;

		if (num == 1) {
			*out = PixelInfo(w, h, kColor_Type_Gray_8, kAlphaType_Opaque);
		} else if (num == 3) {
			*out = PixelInfo(w, h, kColor_Type_RGB_888X, kAlphaType_Opaque);
		} else {
			return false;
		}

		return true;
	}

}