#include <assert.h>

#include "../BGJSContext.h"
#include "AjaxModule.h"

#include "../ClientAbstract.h"

#define LOG_TAG	"AjaxModule"

using namespace v8;

AjaxModule::AjaxModule() : BGJSModule ("ajax") {
}

static Handle<Value> BlaCallback(const Arguments& args) {

	return v8::Undefined();
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

	target->Set(String::New("exports"), ft->GetFunction());
}

v8::Local<v8::Value> AjaxModule::initWithContext(v8::Isolate* isolate, const BGJSContext* context)
{
	doRegister(isolate, context);
	return v8::Undefined(isolate);
}

AjaxModule::~AjaxModule() {

}

void AjaxModule::ajax(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
	v8::Locker l(isolate);
	if (args.Length() < 1) {
		LOGE("Not enough parameters for ajax");
		return v8::Undefined();
	}

	/* Local<StackTrace> str = StackTrace::CurrentStackTrace(15);
	LogStackTrace(str); */

	HandleScope scope(isolate);

	Local<v8::Object> options = args[0]->ToObject();

	// Get callbacks
	Local<Function> callback = Local<Function>::Cast(
			options->Get(String::NewStringFromUtf8(isolate, "success")));
	Local<Function> errCallback = Local<Function>::Cast(
				options->Get(String::NewFromUtf8(isolate, "error")));

	Persistent<Function>* callbackPers = new Persistent<Function>::New(isolate, callback);
	Persistent<Function>* errorPers;
	if (!errCallback.IsEmpty() && !(errCallback.IsUndefined() || errCallback.isNull())  {
		errorPers = new Persistent<Function>::New(isolate, errCallback);
	}
	// Persist the this object
	Persistent<Object>* thisObj = Persistent<Object>::New(isolate, args.This());

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
		return v8::Undefined(isolate);
	}
	urlStr = env->NewStringUTF(*urlLocal);
	if (!data.IsUndefined()) {
		String::Utf8Value dataLocal(data);
		dataStr = env->NewStringUTF(*dataLocal);
	} else {
		dataStr = 0;
	}

	if (!method.IsUndefined()) {
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

	args.getReturnValue().SetUndefined();
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
	Isolate* isolate = Isolate::GetCurrent();
	v8::Locker l(isolate);
	Context::Scope context_scope((*reinterpret_cast<Local<Context>*>(&context->_context)));
	const char *nativeString = NULL;

	HandleScope scope(isolate);
	TryCatch trycatch;

	Persistent<Object>* thisObj = static_cast<Persistent<Object>*>((void*)thisPtr);
	Persistent<Function>* errorP;
	if (errorCb) {
		errorP = static_cast<Persistent<Function>*>((void*)errorCb);
	}
	Persistent<Function>* callbackP = static_cast<Persistent<Function>*>((void*)jsCbPtr);

	Handle<Value> argarray[1];
	int argcount = 1;

	if (dataStr == 0) {
		argarray[0] = v8::Null();
	} else {
		nativeString = env->GetStringUTFChars(dataStr, 0);
		Handle<Value> resultObj;
		if (processData) {
			resultObj = context->JsonParse(thisObj,
				String::NewFromUtf8(isolate,nativeString));
		} else {
			resultObj = String::NewFromUtf8(isolate, nativeString);
		}

		argarray[0] = resultObj;
	}


	Handle<Value> result;

    Local<Object> thisObjLocal = (*reinterpret_cast<Local<Object>*>(thisObj));
	if (success) {
		result = (*reinterpret_cast<Local<Object>*>(callbackP))->Call(thisObjLocal, argcount, argarray);
	} else {
		if (!errorP.IsEmpty()) {
			result = (*reinterpret_cast<Local<Object>*>(errorP))->Call(thisObjLocal, argcount, argarray);
		} else {
			LOGI("Error signaled by java code but no error callback set");
			result = v8::Undefined();
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
