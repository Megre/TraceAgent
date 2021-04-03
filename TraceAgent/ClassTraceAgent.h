#include "jvmti.h"
#include "AgentException.h"
#include <conio.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>

class ClassTraceAgent 
{
 public:

	ClassTraceAgent() throw(AgentException) {}

	ClassTraceAgent(JavaVM *vm) throw(AgentException);

	ClassTraceAgent(jvmtiEnv *jvmti) throw(AgentException);

	~ClassTraceAgent() throw(AgentException);

	void Init(JavaVM *vm) const throw(AgentException);

	void AddCapability() const throw(AgentException);
        
	//void RegisterEvent() const throw(AgentException);
	
	static void JNICALL HandleFieldAccess(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread,
            jmethodID method,
            jlocation location,
            jclass field_klass,
            jobject object,
            jfieldID field);

	static void JNICALL HandleFieldModification(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread,
            jmethodID method,
            jlocation location,
            jclass field_klass,
            jobject object,
            jfieldID field,
            char signature_type,
            jvalue new_value);

	static void JNICALL HandleClassPrepare(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread,
            jclass klass);

 private:
	    
	static jvmtiEnv * m_jvmti;

	static char m_lastSignature[1024];

	//static jlong m_tag;
};
