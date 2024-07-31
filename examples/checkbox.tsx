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

import { _CVD, Text, createCss, mainScreenScale } from 'quark';
import { Switch, Basic, Checkbox } from 'quark/checkbox';
import { Page } from './tool';
import { Event } from 'quark/event';

const resolve = require.resolve;
const px = 1 / mainScreenScale();

createCss({
	'.checkbox_page': {
		width: 'match',
	},
	'.checkbox_page .item': {
		width: 'match',
		borderBottom: `${px} #ccc`,
	},
	'.checkbox_page .text': {
		width: '100!',
		margin: 13,
		align: 'start',
	},
	'.checkbox_page .x_checkbox': {
		align: 'start',
	}
})

function change_handle(value: boolean, sender: Basic) {
	let checkbox = sender as Switch;
	let str = value ? 'YES' : 'NO';
	str += checkbox.disable ? ',Disable' : '';
	(checkbox.domAs().prev as Text).value = str;
}

export default (self: Page)=>{
	self.title = 'Checkbox';
	self.source = resolve(__filename);

	return (
		<box width="match" class="checkbox_page">
			<box class="item">
				<text class="text" value="YES" />
				<Switch onChange={change_handle} style={{margin:10}} initSelected={true} />
			</box>
			<box class="item">
				<text class="text" value="NO,Disable" />
				<Switch onChange={change_handle} style={{margin:10}} initSelected={false} />
			</box>
			<box class="item">
				<text class="text" value="NO" />
				<Switch onChange={change_handle} style={{margin:10}} />
			</box>
			<box class="item">
				<text class="text" value="YES" />
				<Checkbox onChange={change_handle} style={{margin:13}} initSelected={true} />
			</box>
			<box class="item">
				<text class="text" value="YES,Disable" />
				<Checkbox onChange={change_handle} style={{margin:13}} disable={true} initSelected={true} />
			</box>
			<box class="item">
				<text class="text" value="NO" />
				<Checkbox onChange={change_handle} style={{margin:13}} />
			</box>
		</box>
	)
}
