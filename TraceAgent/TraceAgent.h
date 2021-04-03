#include "jvmti.h"
#include "AgentException.h"
#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "ThreadRecord.h"
#include "TraceFilter.h"

#define MAX_LOG_LENGTH 1024
#define LOG_LINE_NUM_SPACE 10

#define STR(a) #a
#define JOIN_STR(a, b, c) a##b##c


class TraceAgent 
{
 public:

	TraceAgent(JavaVM *vm, char *options);
	~TraceAgent();
	        
	void RegisterEvent() const;
    
	static void JNICALL HandleMethodEntry(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jmethodID method);
	static void JNICALL HandleMethodExit(jvmtiEnv *jvmti, JNIEnv* jni, jthread thread, jmethodID method, 
		jboolean was_popped_by_exception, jvalue return_value);

	static void JNICALL HandleFieldAccess(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method,
		jlocation location, jclass field_klass, jobject object, jfieldID field);
	static void JNICALL HandleFieldModification(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method,
        jlocation location, jclass field_klass, jobject object, jfieldID field, char signature_type, jvalue new_value);
	static void JNICALL HandleClassPrepare(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jclass klass);

	TraceFilter* GetFilter();
	jvmtiEnv* GetJvmtiEnv();
	jvmtiEnv* CreateJvmtiEnv(JavaVM *vm) const;

	list<string> GetLogList();

 private:
	static TraceFilter *m_filter;
	static jvmtiEnv *m_jvmti;
	static ThreadRecord *m_thread_record;
	static jlong m_ctag;
	static list<string> m_log_list;
	static int m_jni_access;

	void AddCapability() const;
	static jlong GetObjectTag(jobject object);

	static void PrintLineNum(size_t num);
};
