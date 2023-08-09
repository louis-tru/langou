#vert
// The solution is based on Wojciech Jarosz's Fast Image Convolution
// https://blog.ivank.net/fastest-gaussian-blur.html
// https://elynxsdk.free.fr/ext-docs/Blur/Fast_box_blur.pdf
// https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf

void main() {
	gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);
}

#frag
uniform lowp vec4 color;
void main() {
	fragColor = color;
}