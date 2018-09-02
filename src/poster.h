#pragma once

class SSPoster : public GenericVideoFilter {

	int count;
	bool clr;
public:

	SSPoster(PClip _child, int _SquareSize, bool _clr, IScriptEnvironment* env);
	~SSPoster();
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};

