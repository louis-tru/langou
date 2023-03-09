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

#import "../../util/loop.h"
#import "../../app.h"
#import "../../event.h"
#import "../../display.h"
#import "./ios_app.h"

using namespace qk;

// ***************** Q k . A p p l i c a t i o n . D e l e g a t e *****************

typedef Display::Orientation Orientation;

QkApplicationDelegate *__appDelegate = nil; // global object

@interface QkRootViewController()
	@property (weak, nonatomic) QkApplicationDelegate* appDelegate;
@end

@implementation QkApplicationDelegate

	static void render_exec_func(Cb::Data& evt, Object* ctx) {
		__appDelegate.render_task_count--;
		__appDelegate.app->display()->render();
	}

	- (void)display_link_callback:(CADisplayLink*)displayLink {
		auto _ = self.app;
		#if Qk_USE_DEFAULT_THREAD_RENDER
			if (_->is_loaded()) {
				if (_fps == 0) { // 3 = 15, 1 = 30
					_->display()->render();
					_fps = 0;
				} else {
					_fps++;
				}
			}
		#else
			if (self.render_task_count == 0) {
				self.render_task_count++;
				_->loop()->post(_render_exec);
			} else {
				Qk_DEBUG("miss frame");
			}
		#endif
	}

	- (void)refresh_status {
		if ( self.window.rootViewController == self.root_ctr ) {
			self.window.rootViewController = nil;
			self.window.rootViewController = self.root_ctr;
		}
	}

	- (RootViewController*)root_ctr {
		if (!_root_ctr) // singleton mode
			self.root_ctr = [[RootViewController alloc] init];
		return _root_ctr;
	}

	- (UIWindow*)window {
		if (!_window) // singleton mode
			self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		return _window;
	}

	- (void)resize {
		auto render = dynamic_cast<AppleRender*>(self.host->render());
		render->resize(__appDelegate.view.frame);
	}

	- (BOOL)application:(UIApplication*)app didFinishLaunchingWithOptions:(NSDictionary*)options {
		Qk_ASSERT(!__appDelegate);
		__appDelegate = self;
		Qk_ASSERT(Application::shared());
		_host = Application::shared();

		//[app setStatusBarStyle:UIStatusBarStyleLightContent];
		//[app setStatusBarHidden:NO];
		_is_background = NO;
		_render_exec = Cb(render_exec_func);
		
		self.host = app;
		self.render_task_count = 0;
		self.setting_orientation = Orientation::ORIENTATION_USER;
		self.current_orientation = Orientation::ORIENTATION_INVALID;
		self.visible_status_bar = YES;
		self.status_bar_style = UIStatusBarStyleDefault;
		self.root_ctr.appDelegate = self;
		self.display_link = [CADisplayLink displayLinkWithTarget:self
																										selector:@selector(display_link_callback:)];
		self.window.backgroundColor = [UIColor blackColor];
		self.window.rootViewController = self.root_ctr;
		
		[self.window makeKeyAndVisible];
		
		UIView *rootView = self.window.rootViewController.view;
		auto render = dynamic_cast<AppleRender*>(self.host->render());

		self.view = render->init(rootView.bounds);
		self.view.contentScaleFactor = UIScreen.mainScreen.scale;
		self.view.translatesAutoresizingMaskIntoConstraints = NO;
		self.view.multipleTouchEnabled = YES;
		self.view.userInteractionEnabled = YES;

		self.ime = [[IOSIMEHelprt alloc] initWithApplication:self.host];

		[rootView addSubview:self.view];
		[rootView addSubview:self.ime];
		[rootView addConstraint:[NSLayoutConstraint
												constraintWithItem:self.view
												attribute:NSLayoutAttributeWidth
												relatedBy:NSLayoutRelationEqual
												toItem:rootView
												attribute:NSLayoutAttributeWidth
												multiplier:1
												constant:0]];
		[rootView addConstraint:[NSLayoutConstraint
												constraintWithItem:self.view
												attribute:NSLayoutAttributeHeight
												relatedBy:NSLayoutRelationEqual
												toItem:rootView
												attribute:NSLayoutAttributeHeight
												multiplier:1
												constant:0]];
		
		_host->display()->set_default_scale(UIScreen.mainScreen.scale);

		render->resize(self.view.frame);

		_inl_app(_host)->triggerLoad();

		[self.display_link addToRunLoop:[NSRunLoop mainRunLoop]
														forMode:NSDefaultRunLoopMode];
		return YES;
	}

	- (void)application:(UIApplication*)app didChangeStatusBarFrame:(CGRect)frame {
		if ( __appDelegate && !_is_background ) {
			[self resize];
		}
	}

	- (void)applicationWillResignActive:(UIApplication*) application {
		_inl_app(_host)->triggerPause();
	}

	- (void)applicationDidBecomeActive:(UIApplication*) application {
		_inl_app(_host)->triggerResume();
		[self resize];
	}

	- (void)applicationDidEnterBackground:(UIApplication*) application {
		_is_background = YES;
		_inl_app(_host)->triggerBackground();
	}

	- (void)applicationWillEnterForeground:(UIApplication*) application {
		_is_background = NO;
		_inl_app(_host)->triggerForeground();
	}

	- (void)applicationDidReceiveMemoryWarning:(UIApplication*) application {
		_inl_app(_host)->triggerMemorywarning();
	}

	- (void)applicationWillTerminate:(UIApplication*)application {
		[self.display_link removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		_inl_app(_host)->triggerUnload();
	}

	- (void) mailComposeController:(MFMailComposeViewController*)controller
						didFinishWithResult:(MFMailComposeResult)result
													error:(NSError*)error {
		[controller dismissViewControllerAnimated:YES completion:nil];
	}

@end


// ***************** A p p l i c a t i o n *****************

static NSArray<NSString*>* split_ns_array(cString& str) {
	auto arr = [NSMutableArray<NSString*> new];
	for (auto& i : str.split(','))
		[arr addObject: [NSString stringWithUTF8String:i.c_str()]];
	return arr;
}

void Application::pending() {
	// exit(0);
}

void Application::open_url(cString& url) {
	NSURL* url_ = [NSURL URLWithString:[NSString stringWithUTF8String:*url]];
	dispatch_async(dispatch_get_main_queue(), ^{
		[__appDelegate.app openURL:url_ options:@{} completionHandler:nil];
	});
}

void Application::send_email(cString& recipient,
															cString& subject,
															cString& cc, cString& bcc, cString& body) {
	id recipient_ = split_ns_array(recipient);
	id subject_ = [NSString stringWithUTF8String:*subject];
	id cc_ = split_ns_array(cc);
	id bcc_ = split_ns_array(bcc);
	id body_ = [NSString stringWithUTF8String:*body];

	dispatch_async(dispatch_get_main_queue(), ^{
		MFMailComposeViewController* mail = [MFMailComposeViewController new];
		[mail setToRecipients:recipient_];
		[mail setSubject:subject_];
		[mail setCcRecipients:cc_];
		[mail setBccRecipients:bcc_];
		[mail setMessageBody:body_ isHTML:NO];
		mail.mailComposeDelegate = __appDelegate;
		[__appDelegate.root_ctr presentViewController:mail animated:YES completion:nil];
	});
}

// ***************** E v e n t . D i s p a t c h *****************

void EventDispatch::set_volume_up() {
	// TODO ..
}

void EventDispatch::set_volume_down() {
	// TODO ..
}

void EventDispatch::set_ime_keyboard_open(KeyboardOptions options) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[__appDelegate.ime set_keyboard_type:options.type];
		[__appDelegate.ime set_keyboard_return_type:options.return_type];
		if ( options.is_clear ) {
			[__appDelegate.ime clear];
		}
		[__appDelegate.ime open];
	});
}

void EventDispatch::set_ime_keyboard_can_backspace(bool can_backspace, bool can_delete) {
	dispatch_async(dispatch_get_main_queue(), ^{
		[__appDelegate.ime set_keyboard_can_backspace:can_backspace can_delete:can_delete];
	});
}

void EventDispatch::set_ime_keyboard_close() {
	dispatch_async(dispatch_get_main_queue(), ^{
		[__appDelegate.ime close];
	});
}

void EventDispatch::set_ime_keyboard_spot_location(Vec2 location) {
	// noop
}
