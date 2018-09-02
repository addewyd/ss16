#include "windows.h"
#include "avisynth.h"
#include "masks.h"

#include "math.h"
/*
CIELAB space coordinate ranges are as follows :

L in[0, 100]

A in[-86.185, 98.254]

B in[-107.863, 94.482]
*/
static void lab2rgb(SSClr &lab) {
	double y = (lab.l + 16) / 116.0;
	double x = lab.A / 500. + y;
	double z = y - lab.B / 200.0;
	double	r, g, b;

	x = 0.95047 * ((x * x * x > 0.008856) ? x * x * x : (x - 16 / 116) / 7.787);
	y = 1.00000 * ((y * y * y > 0.008856) ? y * y * y : (y - 16 / 116) / 7.787);
	z = 1.08883 * ((z * z * z > 0.008856) ? z * z * z : (z - 16 / 116) / 7.787);

	r = x * 3.2406 + y * -1.5372 + z * -0.4986;
	g = x * -0.9689 + y * 1.8758 + z * 0.0415;
	b = x * 0.0557 + y * -0.2040 + z * 1.0570;

	r = (r > 0.0031308) ? (1.055 * pow(r, 1 / 2.4) - 0.055) : 12.92 * r;
	g = (g > 0.0031308) ? (1.055 * pow(g, 1 / 2.4) - 0.055) : 12.92 * g;
	b = (b > 0.0031308) ? (1.055 * pow(b, 1 / 2.4) - 0.055) : 12.92 * b;


	lab.r = (unsigned int)(max(0, min(1, r)) * 255);
	lab.g = (unsigned int)(max(0, min(1, g)) * 255);
	lab.b = (unsigned int)(max(0, min(1, b)) * 255);
}


static void rgb2lab(SSClr &rgb) {
	double	r = rgb.r / 255.;
	double	g = rgb.g / 255.;
	double	b = rgb.b / 255.;
	double	x, y, z;

	r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
	g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
	b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

	x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047;
	y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000;
	z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883;

	x = (x > 0.008856) ? pow(x, 1 / 3.) : (7.787 * x) + 16 / 116.;
	y = (y > 0.008856) ? pow(y, 1 / 3.) : (7.787 * y) + 16 / 116.;
	z = (z > 0.008856) ? pow(z, 1 / 3.) : (7.787 * z) + 16 / 116.;

	rgb.l = ((116 * y) - 16);
	rgb.A = (500 * (x - y));
	rgb.B = (200 * (y - z));
}


static double  deltaE(const SSClr& labA, const SSClr& labB) {
	double deltaL = labA.l - labB.l;
	double deltaA = labA.A - labB.A;
	double deltaB = labA.B - labB.B;
	double c1 = sqrt(labA.A * labA.A + labA.B * labA.B);
	double c2 = sqrt(labB.A * labB.A + labB.B * labB.B);
	double deltaC = c1 - c2;
	double deltaH = deltaA * deltaA + deltaB * deltaB - deltaC * deltaC;
	deltaH = deltaH < 0 ? 0 : sqrt(deltaH);
	double sc = 1.0 + 0.045 * c1;
	double sh = 1.0 + 0.015 * c1;
	double deltaLKlsl = deltaL / (1.0);
	double deltaCkcsc = deltaC / (sc);
	double deltaHkhsh = deltaH / (sh);
	double i = deltaLKlsl * deltaLKlsl + deltaCkcsc * deltaCkcsc + deltaHkhsh * deltaHkhsh;
	return i < 0 ? 0 : sqrt(i);
}

SSMask::SSMask(PClip _child, unsigned int _c, double _t, int _m,
	IScriptEnvironment* env) :
	GenericVideoFilter(_child), color(_c), mode(_m), tolerance(_t), xy(false) {
	sr = (color >> 16) & 0xff;
	sg = (color >> 8) & 0xff;
	sb = (color) & 0xff;
	c2 = SSClr(sr, sg, sb);
	rgb2lab(c2);
}


SSMask::SSMask(PClip _child, int _x, int _y, double _t, int _m,
	IScriptEnvironment* env) :
	GenericVideoFilter(_child), cx(_x), cy(_y), mode(_m), tolerance(_t), xy(true) {
	if (cx >= vi.width || cy < 0) {
		env->ThrowError("bad x");
	}
	if (cy >= vi.height || cy < 0) {
		env->ThrowError("bad y");
	}
}


SSMask::~SSMask() {
}


PVideoFrame __stdcall SSMask::GetFrame(int n,
	IScriptEnvironment* env) {
	PVideoFrame src = child->GetFrame(n, env);
	PVideoFrame dst = env->NewVideoFrame(vi);

	const unsigned char* srcp = src->GetReadPtr();

	unsigned char* dstp = dst->GetWritePtr();

	const int dst_pitch = dst->GetPitch();

	const int dst_width = dst->GetRowSize();
	const int dst_height = dst->GetHeight();

	const int src_pitch = src->GetPitch();

	const int src_width = src->GetRowSize();
	const int src_height = src->GetHeight();

	int w, h;

	if (xy) {
		unsigned int p =
			*((unsigned int *)srcp + cy * src_pitch + cx / 4);
		unsigned int r = (p >> 16) & 0xff;
		unsigned int g = (p >> 8) & 0xff;
		unsigned int b = (p) & 0xff;
		c2 = SSClr(sr, sg, sb);
		rgb2lab(c2);
	}

	if (vi.IsRGB32()) {

		for (h = 0; h < dst_height; h += 1) {
			for (w = 0; w < dst_width / 4; w += 1) {

				unsigned int lt = *((unsigned int *)srcp + w);

				unsigned int a = (lt >> 24) & 0xff;
				unsigned int r = (lt >> 16) & 0xff;
				unsigned int g = (lt >> 8) & 0xff;
				unsigned int b = (lt) & 0xff;
				SSClr c1 = SSClr(r, g, b);
				
				rgb2lab(c1);
				double de = deltaE(c1, c2);
				unsigned int aval = 0xff;

				switch (mode) {
				case 0:
					a = de < tolerance ? aval : 0;
					break;
				case 1:
					aval = 255 - (unsigned int)((de / 150.) * 255.);
					a = aval;
					break;
				case 2:

					double k = (1.0 - de / tolerance) * 255.;
					

					aval = (unsigned int)k;
					a = de < tolerance ? aval : 0;
					break;
				}			

				unsigned int rc;
				rc = (a << 24) | (r << 16) | (g << 8) | b;

				*((unsigned int *)dstp + w) = rc;


			}

			srcp = srcp + src_pitch;
			dstp = dstp + dst_pitch;
		}
	}
	else {
		env->ThrowError("RGB32Only");
	}

	return dst;
}

