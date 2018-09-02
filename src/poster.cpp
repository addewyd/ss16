#include "windows.h"
#include "avisynth.h"
#include "poster.h"


SSPoster::SSPoster(PClip _child, int _count, bool _clr,
	IScriptEnvironment* env) :
	GenericVideoFilter(_child), count(_count), clr(_clr) {
}


SSPoster::~SSPoster() {
}



PVideoFrame __stdcall SSPoster::GetFrame(int n,
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

	if (vi.IsRGB24()) {

		for (h = 0; h < src_height; h++) {
			for (w = 0; w < src_width; w += 3) {
				*(dstp + w) = *(srcp + w);            // Copy each Blue byte from source to destination.
				*(dstp + w + 1) = *(srcp + w + 1);    // Copy Green.
				*(dstp + w + 2) = *(srcp + w + 2);    // Copy Red
			}

			srcp = srcp + src_pitch; // Add the pitch (note use of pitch and not width) of one line (in bytes) to the source pointer
			dstp = dstp + dst_pitch; // Add the pitch to the destination pointer.
		}
	}

	if (vi.IsRGB32()) {

		for (h = 0; h < src_height; h++) {                                            // Loop from bottom line to top line.
			for (w = 0; w < src_width / 4; w += 1) {                               // a
				*((unsigned int *)dstp + w) = *((unsigned int *)srcp + w);
			}                                                             
																			   
			srcp = srcp + src_pitch;                                           
			dstp = dstp + dst_pitch;                                           
		}
	}

	// ...................................................................................

	if (vi.IsYUY2()) {

		for (h = 0; h < src_height; h++) {                                            // Loop from top line to bottom line (opposite of RGB colourspace).
			for (w = 0; w < src_width / 4; w += 1) {                               // and from leftmost double-pixel to rightmost one.
				*((unsigned int *)dstp + w) = *((unsigned int *)srcp + w); // Copy 2 pixels worth of information from source to destination.
			}                                                                  // at a time by temporarily treating the src and dst pointers as
																			   // 32bit (4 byte) pointers intead of 8 bit (1 byte) pointers
			srcp = srcp + src_pitch;                                           // Add the pitch (note use of pitch and not width) of one line (in bytes) to the source pointer
			dstp = dstp + dst_pitch;                                           // Add the pitch to the destination pointer.
		}
	}

	// .......................................................................................................

	if (vi.IsYV12()) {
		int c = count - 1;
		double dv = 256.0 / ((c > 0 ? c : 1));

		for (h = 0; h < src_height; h++) {
			for (w = 0; w < src_width; w++) {
				unsigned int val = *(srcp + w);

				val = unsigned(val / dv * (1 + 1.0 / c)) * dv;
				val = val > 255 ? 255 : val;

				*(dstp + w) = val;
			}
			srcp = srcp + src_pitch;
			dstp = dstp + dst_pitch;
		}


		const int dst_pitchUV = dst->GetPitch(PLANAR_U);    // The pitch,height and width information
		const int dst_widthUV = dst->GetRowSize(PLANAR_U);  // is guaranted to be the same for both
		const int dst_heightUV = dst->GetHeight(PLANAR_U);  // the U and V planes so we only the U
		const int src_pitchUV = src->GetPitch(PLANAR_U);    // plane values and use them for V as
		const int src_widthUV = src->GetRowSize(PLANAR_U);  // well
		const int src_heightUV = src->GetHeight(PLANAR_U);  //

															//Copy U plane src to dst
		srcp = src->GetReadPtr(PLANAR_U);
		dstp = dst->GetWritePtr(PLANAR_U);

		for (h = 0; h < src_heightUV; h++) {
			for (w = 0; w < src_widthUV; w++) {
				*(dstp + w) = 128; //*(srcp + w);
				if (clr) {
					unsigned int val = *(srcp + w);

					val = unsigned(val / dv * (1 + 1.0 / c)) * dv;
					val = val > 127 ? 127 : val;

					*(dstp + w) = val;

				}
			}
			srcp = srcp + src_pitchUV;
			dstp = dstp + dst_pitchUV;
		}

		srcp = src->GetReadPtr(PLANAR_V);
		dstp = dst->GetWritePtr(PLANAR_V);

		for (h = 0; h < src_heightUV; h++) {
			for (w = 0; w < src_widthUV; w++) {
				*(dstp + w) = 128;// *(srcp + w);

				if (clr) {
					unsigned int val = *(srcp + w);

					val = unsigned(val / dv * (1 + 1.0 / c)) * dv;
					val = val > 127 ? 127 : val;

					*(dstp + w) = val;

				}
			}
			srcp = srcp + src_pitchUV;
			dstp = dstp + dst_pitchUV;
		}

	}

	return dst;
}

