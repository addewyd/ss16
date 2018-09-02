#pragma once

enum SS_MODE {
	ss_min,
	ss_max,
	ss_avg,
	ss_lt,
	ss_rt,
	ss_lb,
	ss_rb
};


class SubSample : public GenericVideoFilter {

	SS_MODE mode;
	int wc;

public:

	SubSample(PClip _child, SS_MODE, int wc, IScriptEnvironment* env);
	~SubSample();
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};


