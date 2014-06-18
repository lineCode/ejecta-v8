#ifndef __AJAXMODULE_H
#define __AJAXMODULE_H	1

#include "../BGJSModule.h"

/**
 * AjaxModule
 * AJAX BGJS extension
 *
 * Copyright 2014 Kevin Read <me@kevin-read.com> and BörseGo AG (https://github.com/godmodelabs/ejecta-v8/)
 * Licensed under the MIT license.
 */

class AjaxModule : public BGJSModule {
	bool initialize();
	~AjaxModule();

public:
	AjaxModule ();
	v8::Handle<v8::Value> initWithContext(BGJSContext* context);
	static void doRequire (v8::Handle<v8::Object> target);
	static v8::Handle<v8::Value> ajax(const v8::Arguments& args);
};


#endif
