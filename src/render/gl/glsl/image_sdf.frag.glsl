#include "_util.glsl"

in      lowp  vec2      coord_f;
in      lowp  float     sdf_f;
uniform lowp  float     opacity;
uniform       sampler2D image;
void main() {
	fragColor = texture(image, coord_f) * vec4(1.0, 1.0, 1.0, opacity);
}