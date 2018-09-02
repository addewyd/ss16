#include "windows.h"
#include "avisynth.h"
#include "subsample.h"

#define avg4(x,y,z,t) (((x) + (y) + (z) + (t)) / 4)

SubSample::SubSample(PClip _child, SS_MODE _mode, int _wc,
	IScriptEnvironment* env) :
	GenericVideoFilter(_child), mode(_mode), wc(_wc) {
	if (!(vi.width % 2) == 0) {
		env->ThrowError("bad width %n", vi.width);
	}
	if (!(vi.height % 2) == 0) {
		env->ThrowError("bad height %n", vi.height);
	}
	if (!vi.IsRGB32()) {
		env->ThrowError("RGB32 only");
	}
	vi.width = vi.width / 2;
	vi.height = vi.height / 2;
}


SubSample::~SubSample() {
}


PVideoFrame __stdcall SubSample::GetFrame(int n,
	IScriptEnvironment* env) {
	PVideoFrame src = child->GetFrame(n, env);
	PVideoFrame dst = env->NewVideoFrame(vi);

	const unsigned char* srcp = src->GetReadPtr();

	unsigned char* dstp = dst->GetWritePtr();

	const int dst_pitch = dst->GetPitch();

	const int dst_width = dst->GetRowSize();
	const int dst_height = dst->GetHeight();

	const int src_pitch = src->GetPitch();
	const unsigned char* srcp2 = srcp + src_pitch;

	const int src_width = src->GetRowSize();
	const int src_height = src->GetHeight();

	int w, h;

	if (vi.IsRGB32()) {

		for (h = 0; h < dst_height; h+=1) {
			for (w = 0; w < dst_width / 4; w += 1) {
				//				*((unsigned int *)dstp + w) = *((unsigned int *)srcp + w);

				unsigned int lt = *((unsigned int *)srcp + w * 2);
				unsigned int rt = *((unsigned int *)srcp + w * 2 + 1);
				unsigned int lb = *((unsigned int *)(srcp + src_pitch) + w * 2);
				unsigned int rb = *((unsigned int *)(srcp + src_pitch) + w * 2 + 1);

				unsigned int ltr = (lt >> 24) & 0xff;
				unsigned int ltg = (lt >> 16) & 0xff;
				unsigned int ltb = (lt >> 8) & 0xff;
				unsigned int lta = (lt) & 0xff;

				unsigned int rtr = (rt >> 24) & 0xff;
				unsigned int rtg = (rt >> 16) & 0xff;
				unsigned int rtb = (rt >> 8) & 0xff;
				unsigned int rta = (rt) & 0xff;

				unsigned int lbr = (lb >> 24) & 0xff;
				unsigned int lbg = (lb >> 16) & 0xff;
				unsigned int lbb = (lb >> 8) & 0xff;
				unsigned int lba = (lb) & 0xff;

				unsigned int rbr = (rb >> 24) & 0xff;
				unsigned int rbg = (rb >> 16) & 0xff;
				unsigned int rbb = (rb >> 8) & 0xff;
				unsigned int rba = (rb) & 0xff;

				unsigned int r, g, b, a, rc;

				if (mode == ss_max) {
					r = max(max(max(ltr, rtr), lbr), rbr);
					g = max(max(max(ltg, rtg), lbg), rbg);
					b = max(max(max(ltb, rtb), lbb), rbb);
					a = max(max(max(lta, rta), lba), rba);
					rc = ((r << 24) & 0xff000000) | ((g << 16) & 0xff0000) | ((b << 8) & 0xff00)| (a & 0xff);
				}
				else if (mode == ss_min) {
					r = min(min(min(ltr, rtr), lbr), rbr);
					g = min(min(min(ltg, rtg), lbg), rbg);
					b = min(min(min(ltb, rtb), lbb), rbb);
					a = min(min(min(lta, rta), lba), rba);
					rc = (r << 24) | (g << 16) | (b << 8) | a;
				}
				else if(mode == ss_avg) {
					r = avg4(ltr, rtr, lbr, rbr);
					g = avg4(ltg, rtg, lbg, rbg);
					b = avg4(ltb, rtb, lbb, rbb);
					a = avg4(lta, rta, lba, rba);
					rc = (r << 24) | (g << 16) | (b << 8) | a;
				}
				else if (mode == ss_lt) {
					rc = lt;
				}
				else if (mode == ss_rt) {
					rc = rt;
				}
				else if (mode == ss_lb) {
					rc = lb;
				}
				else if (mode == ss_rb) {
					rc = rb;
				}
				else {
					rc = 0xff;
				}

				if (wc == 1) {
					rc = r << 24; //a
				}
				else if (wc == 2) {
					rc = g << 16; //r 
				}
				else if (wc == 3) { //g 
					rc = b << 8;
				}
				else if (wc == 4) { //b
					rc = a;
				}

				*((unsigned int *)dstp + w) = rc;
			}

			srcp = srcp + src_pitch * 2;
			dstp = dstp + dst_pitch;
		}
	}

	return dst;
}

