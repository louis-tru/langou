
import { GUIApplication, Root, Div, Indep, default as ngui, _CVD, ViewController } from 'ngui';
import { Color } from 'ngui/value';
import util from 'ngui/util';
import * as css from 'ngui/css';
import * as uu from './uu';

const random = util.random;

var test_count = 100000;

css.create({
	'.root': {
		backgroundColor: '#000',
	},
});

class RootViewController extends ViewController {

	triggerMounted() {
		super.triggerMounted();

		var w = ngui.displayPort.width;
		var h = ngui.displayPort.height;
		var csss: Dict<css.StyleSheet> = {};
	
		for (var i = 0; i < test_count; i++) {
			var s = random(20, 30);
			var s2 = s / 2;
			csss['.root .css_' + i] = {
				backgroundColor: new Color(random(0, 255), random(0, 255), random(0, 255), 255),
				width: s,
				height: s,
				x: random(0, w + s) - s2,
				y: random(0, h + s) - s2,
			};
		}
	
		uu.start();
	
		css.create(csss);
	
		uu.log();

		var v = this.domAs();
	
		v.class = 'root';
	
		ngui.render(
			<Div width="full" height="full">
				{
					Array.from({length:test_count}, (j, i)=>{
						return <Indep class={'css_' + i} />;
					})
				}
			</Div>,
			v
		);
	
		uu.log();
	}
}

new GUIApplication({ multisample: 2 }).start(<RootViewController><Root /></RootViewController>);