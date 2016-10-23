#ifndef __BGJSINFO_H
#define __BGJSINFO_H	1

#include <v8.h>
// #include "BGJSContext.h"

class BGJSContext;

#include "os-detection.h"

class BGJSInfo {
public:
	static v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context> > _context;
	static v8::Persistent<v8::ObjectTemplate, v8::CopyablePersistentTraits<v8::ObjectTemplate> > _global;
	static BGJSContext* _jscontext;
};

#endif
