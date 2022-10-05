{
	'targets': [
	{
		# 'product_prefix': '',
		# 'product_ext': 'so',
		'target_name': 'quark-util',
		'type': 'static_library',
		'dependencies': [
			'src/util/minizip.gyp:minizip',
			'deps/btree/btree.gyp:btree',
			'deps/libuv/libuv.gyp:libuv',
			'deps/node/deps/openssl/openssl.gyp:openssl',
			'deps/node/deps/http_parser/http_parser.gyp:http_parser',
		],
		'direct_dependent_settings': {
			'include_dirs': [ '../..' ],
			'mac_bundle_resources': [],
		},
		'include_dirs': [
			'../..',
			'../../deps/rapidjson/include',
			'../../deps/btree/include',
			'../../deps/node/deps/zlib',
			'../../deps/node/deps/zlib/contrib/minizip',
		],
		'sources': [
			'../../Makefile',
			'../../README.md',
			'../../configure',
			'../../tools/configure.js',
			# h
			'../version.h',
			'errno.h',
			'uv.h',
			'working.h',
			'fs.h',
			'http.h',
			'net.h',
			'loop.h',
			'string.h',
			'array.h',
			'cb.h',
			'codec.h',
			'error.h',
			'event.h',
			'handle.h',
			'hash.h',
			'json.h',
			'storage.h',
			'log.h',
			'macros.h',
			'numbers.h',
			'object.h',
			'util.h',
			'zlib.h',
			'iterator.h',
			'list.h',
			'dict.h',
			'dict.cc',
			# cc
			'uv.cc',
			'working.cc',
			'array.cc',
			'fs_async.cc',
			'fs_path.cc',
			'fs_reader.cc',
			'fs_sync.cc',
			'fs.cc',
			'stream.h',
			'http_cookie.cc',
			'http_helper.cc',
			'http_uri.cc',
			'http.cc',
			'net.cc',
			'loop.cc',
			'string.cc',
			'cb.cc',
			'codec.cc',
			'time.cc',
			'error.cc',
			'json.cc',
			'storage.cc',
			'object.cc',
			'util.cc',
			'zlib.cc',
			'numbers.cc',
			'hash.cc',
			'log.cc',
		],
		'conditions': [
			['os=="android"', {
				'conditions': [['<(android_api_level)<24', {
					'defines!': [ '_FILE_OFFSET_BITS=64' ],
				}]],
				'sources':[
					'platforms/android_jni.h',
					'platforms/android_jni.cc',
					'platforms/android_path.cc',
				],
				'link_settings': {
					'libraries': [
						'-latomic', 
						'-llog', 
						'-landroid',
						'-lz',
					],
				},
			}],
			['os=="linux"', {
				'sources': [
					'platforms/linux_path.cc',
				],
				'link_settings': {
					'libraries': [
						'-lz',
					]
				},
			}],
			['OS=="mac"', {
				'sources': [
					'platforms/apple_path.mm',
				],
				'link_settings': {'libraries': [ '$(SDKROOT)/usr/lib/libz.tbd' ]},
			}],
			['os=="ios"', {
				'link_settings': {'libraries': [ '$(SDKROOT)/System/Library/Frameworks/UIKit.framework' ]},
			}],
			['os=="osx"', {
				'link_settings': { 'libraries': [ '$(SDKROOT)/System/Library/Frameworks/AppKit.framework' ]},
			}],
		],
	}],
}
