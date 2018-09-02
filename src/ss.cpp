
#include "windows.h"
#include "avisynth.h"
#include "poster.h"
#include "subsample.h"
#include "masks.h"


AVSValue __cdecl Create_SSPoster(AVSValue args, void* user_data,
	IScriptEnvironment* env) {
	return new SSPoster(args[0].AsClip(),
		args[1].AsInt(0), 
		args[2].AsBool(false),
		env);        
}


AVSValue __cdecl Create_SubSample(AVSValue args, void* user_data,
	IScriptEnvironment* env) {

	int _s = args[1].AsInt(0);
	SS_MODE s;
	//SS_MODE s = _s == 0 ? ss_avg : (_s > 0 ? ss_max : ss_min);
	switch (_s) {
	case 0: s = ss_avg; break;
	case -1: s = ss_min; break;
	case  1: s = ss_max; break;
	case 2: s = ss_lt; break;
	case 3: s = ss_rt; break;
	case 4: s = ss_lb; break;
	case 5: s = ss_rb; break;
	default: s = ss_avg;
	}
	return new SubSample(args[0].AsClip(),
		s, args[2].AsInt(0),
		env);
}


AVSValue __cdecl Create_SSMask(AVSValue args, void* user_data,
	IScriptEnvironment* env) {
	return new SSMask(args[0].AsClip(),
		args[1].AsInt(0),
		args[2].AsFloat(1.0),
		args[3].AsInt(1),
		env);
}


AVSValue __cdecl Create_SSMaskP(AVSValue args, void* user_data,
	IScriptEnvironment* env) {
	return new SSMask(args[0].AsClip(),
		args[1].AsInt(0),
		args[1].AsInt(0),
		args[2].AsFloat(1.0),
		args[3].AsInt(1),
		env);
}




const AVS_Linkage *AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall
AvisynthPluginInit3(IScriptEnvironment* env, AVS_Linkage* vectors) {
	AVS_linkage = vectors;

	env->AddFunction("SSPoster", "c[COUNT]i[CLR]b",
		Create_SSPoster, 0);
	env->AddFunction("SubSample", "c[MODE]i[CLR]i",
		Create_SubSample, 0);
	env->AddFunction("tlrmask", "c[color]i[tolerance]f[MODE]i",
		Create_SSMask, 0);

	env->AddFunction("tlrmaskP", "c[x]i[y]i[tolerance]f[MODE]i", // at the first frame
		Create_SSMaskP, 0);


	// The AddFunction has the following paramters:
	// AddFunction(Filtername , Arguments, Function to call,0);

	// Arguments is a string that defines the types and optional names of the arguments for you filter.
	// c - Video Clip
	// i - Integer number
	// f - Float number
	// s - String
	// b - boolean

	// The word inside the [ ] lets you used named parameters in your script
	// e.g last=SimpleSample(last,size=100).
	// but last=SimpleSample(last,100) will also work automagically

	return "`SSFs' SSFs plugin";
}