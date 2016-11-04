#include <assert.h>

#include "../BGJSContext.h"
#include "AjaxModule.h"

#include "../ClientAbstract.h"

#define LOG_TAG	"AjaxModule"

using namespace v8;

AjaxModule::AjaxModule() : BGJSModule ("ajax") {
}

bool AjaxModule::initialize() {

	return true;
}

void AjaxModule::doRequire (v8::Isolate* isolate, v8::Handle<v8::Object> target) {
    v8::Locker l(isolate);
    HandleScope scope(isolate);

	// Handle<Object> exports = Object::New();
	Handle<FunctionTemplate> ft = FunctionTemplate::New(isolate, ajax);

	// NODE_SET_METHOD(exports, "ajax", this->makeStaticCallableFunc(ajax));
	/* exports->Set(String::New("ajax"), ft->GetFunction());
	exports->Set(String::New("bla"), String::New("test")); */

	target->Set(String::NewFromUtf8(isolate, "exports"), ft->GetFunction());
}

v8::Local<v8::Value> AjaxModule::initWithContext(v8::Isolate* isolate, const BGJSContext* context)
{
	doRegister(isolate, context);
	return v8::Undefined(isolate);
}

AjaxModule::~AjaxModule() {

}

void AjaxModule::ajax(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
	v8::Locker l(isolate);
	if (args.Length() < 1) {
		LOGE("Not enough parameters for ajax");
		isolate->ThrowException(v8::Exception::ReferenceError(v8::String::NewFromUtf8(isolate, "Not enough parameters for ajax")));
		args.GetReturnValue().SetUndefined();
		return;
	}

	/* Local<StackTrace> str = StackTrace::CurrentStackTrace(15);
	LogStackTrace(str); */

	HandleScope scope(isolate);

	Local<v8::Object> options = args[0]->ToObject();

	// Get callbacks
	Local<Function> callback = Local<Function>::Cast(
			options->Get(String::NewFromUtf8(isolate, "success")));
	Local<Function> errCallback = Local<Function>::Cast(
				options->Get(String::NewFromUtf8(isolate, "error")));

	Persistent<Function>* callbackPers = new Persistent<Function>(isolate, callback);
	Persistent<Function>* errorPers;
	if (!errCallback.IsEmpty() && !errCallback->IsUndefined())  {
		errorPers = new Persistent<Function>(isolate, errCallback);
	}
	// Persist the this object
	Persistent<Object>* thisObj = new Persistent<Object>(isolate, args.This());

	// Get url
	Local<String> url = Local<String>::Cast(options->Get(String::NewFromUtf8(isolate, "url")));
	String::Utf8Value urlLocal(url);
	// Get data
	Local<Value> data = options->Get(String::NewFromUtf8(isolate, "data"));

	Local<String> method = Local<String>::Cast(options->Get(String::NewFromUtf8(isolate, "type")));

	Local<String> processKey = String::NewFromUtf8(isolate, "processData");
	bool processData = true;
	if (options->Has(processKey)) {
		processData = options->Get(processKey)->BooleanValue();
	}

#ifdef ANDROID
	jstring dataStr, urlStr, methodStr;
	ClientAndroid* client = (ClientAndroid*)(_bgjscontext->_client);
	JNIEnv* env = JNU_GetEnv();
	if (env == NULL) {
		LOGE("Cannot execute AJAX request with no envCache");
        args.GetReturnValue().SetUndefined();
        return;
	}
	urlStr = env->NewStringUTF(*urlLocal);
	if (!data->IsUndefined()) {
		String::Utf8Value dataLocal(data);
		dataStr = env->NewStringUTF(*dataLocal);
	} else {
		dataStr = 0;
	}

	if (!method->IsUndefined()) {
		String::Utf8Value methodLocal(method);
		methodStr = env->NewStringUTF(*methodLocal);
	} else {
		methodStr = 0;
	}

	jclass clazz = env->FindClass("ag/boersego/bgjs/V8Engine");
	jmethodID ajaxMethod = env->GetStaticMethodID(clazz,
			"doAjaxRequest", "(Ljava/lang/String;JJJLjava/lang/String;Ljava/lang/String;Z)V");
	assert(ajaxMethod);
	assert(clazz);
	/* std::stringstream str;
	 str << "Clazz " << clazz << ", " << urlStr << ", " << dataStr << ", cbPers" << (jint)&callbackPers;
	 LOGI(str.str().c_str()); */
	env->CallStaticVoidMethod(clazz, ajaxMethod, urlStr,
			(jlong) callbackPers, (jlong) thisObj, (jlong) errorPers, dataStr, methodStr, (jboolean)processData);

#endif

	args.GetReturnValue().SetUndefined();
}

#ifdef ANDROID
extern "C" {
JNIEXPORT bool JNICALL Java_ag_boersego_bgjs_ClientAndroid_ajaxDone(
		JNIEnv * env, jobject obj, jlong ctxPtr, jstring data, jint responseCode, jlong cbPtr,
		jlong thisPtr, jlong errorCb, jboolean success, jboolean processData);
};

JNIEXPORT bool JNICALL Java_ag_boersego_bgjs_ClientAndroid_ajaxDone(
		JNIEnv * env, jobject obj, jlong ctxPtr, jstring dataStr, jint responseCode,
		jlong jsCbPtr, jlong thisPtr, jlong errorCb, jboolean success, jboolean processData) {

	BGJSContext* context = (BGJSContext*)ctxPtr;

	Isolate* isolate = context->getIsolate();
	Isolate::Scope isolateScope(isolate);
    v8::Locker l(isolate);
    HandleScope scope(isolate);

	Local<Context> v8Context = context->_context.Get(isolate);
	Context::Scope context_scope(v8Context);

	const char *nativeString = NULL;

	TryCatch trycatch;

	Persistent<Object>* thisObj = static_cast<Persistent<Object>*>((void*)thisPtr);
	Local<Object> thisObjLocal = (*reinterpret_cast<Local<Object>*>(thisObj));
	Persistent<Function>* errorP;
	if (errorCb) {
		errorP = static_cast<Persistent<Function>*>((void*)errorCb);
	}
	Persistent<Function>* callbackP = static_cast<Persistent<Function>*>((void*)jsCbPtr);

	// LOGD("ajaxDone thisObj %p local %p errorP %p callbackP %p processData %i data %p", thisObj, thisObjLocal, errorP, callbackP, processData, dataStr);

	Handle<Value> argarray[1];
	int argcount = 1;

	if (dataStr == 0) {
		argarray[0] = v8::Null(isolate);
	} else {
		nativeString = env->GetStringUTFChars(dataStr, 0);
		Handle<Value> resultObj;
		if (processData) {
			resultObj = context->JsonParse(thisObjLocal,
				String::NewFromUtf8(isolate,nativeString));
		} else {
			resultObj = String::NewFromUtf8(isolate, nativeString);
		}

		argarray[0] = resultObj;
	}


	Handle<Value> result;

	LOGD("ajaxDone callback");

	if (success) {
		result = (*reinterpret_cast<Local<Function>*>(callbackP))->Call(thisObjLocal, argcount, argarray);
	} else {
		if (!errorP->IsEmpty()) {
			result = (*reinterpret_cast<Local<Function>*>(errorP))->Call(thisObjLocal, argcount, argarray);
		} else {
			LOGI("Error signaled by java code but no error callback set");
			result = v8::Undefined(isolate);
		}
	}
	if (result.IsEmpty()) {
		BGJSContext::ReportException(&trycatch);
	}
	if (nativeString) {
		env->ReleaseStringUTFChars(dataStr, nativeString);
	}
	callbackP->Reset();
	thisObj->Reset();
	errorP->Reset();

	return true;
}

#endif
