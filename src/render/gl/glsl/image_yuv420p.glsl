#vert
#import "_image.glsl"

#frag
in      lowp  vec2      coordF;
uniform lowp  float     opacity;
uniform       sampler2D image;
uniform       sampler2D image_u;
uniform       sampler2D image_v;

void main() {
	lowp float y = texture(image, coordF).r;
	lowp float u = texture(image_u, coordF).r;
	lowp float v = texture(image_v, coordF).r;
	fragColor = vec4( y + 1.4075 * (v - 0.5),
									y - 0.3455 * (u - 0.5) - 0.7169 * (v - 0.5),
									y + 1.779  * (u - 0.5),
									opacity);
}