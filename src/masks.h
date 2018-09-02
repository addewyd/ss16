#pragma once
struct SSClr {
	unsigned int r, g, b;
	double l, A, B;
	SSClr(unsigned int _r, unsigned int _g, unsigned int _b) : r(_r), g(_g), b(_b) {}
	SSClr(double _r, double _g, double _b) : l(_r), A(_g), B(_b) {}
	SSClr() {}
	SSClr& operator = (const SSClr& _c) {
		r = _c.r;
		g = _c.g;
		b = _c.b;
		l = _c.l;
		A = _c.A;
		B = _c.B;
		return *this;
	}
};

class SSMask : public GenericVideoFilter{

	double tolerance;
	unsigned int color;
	int cx, cy;
	int mode;
	unsigned int sr, sg, sb;
	SSClr c2;
	bool xy;
public:

	SSMask(PClip _child, unsigned int, double, int, IScriptEnvironment* env);
	SSMask(PClip _child, int, int, double, int, IScriptEnvironment* env);
	~SSMask();
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

