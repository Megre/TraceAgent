
#include "stdafx.h"
#include "jvmti.h"
#include <iostream>
#include <regex>
#include "TraceAgent.h"
#include "Errors.h"
#include "Output.h"

using namespace std;

jvmtiEnv* TraceAgent::m_jvmti;
ThreadRecord *TraceAgent::m_thread_record;
TraceFilter *TraceAgent::m_filter;
list<string> TraceAgent::m_log_list;
jlong TraceAgent::m_ctag;
int TraceAgent::m_jni_access;

TraceAgent::TraceAgent(JavaVM *vm, char *options) {
	m_jvmti = CreateJvmtiEnv(vm);
	m_filter = new TraceFilter(m_jvmti, options);
	AddCapability();

	m_ctag = 1;
	m_jni_access = 0;

	m_thread_record = new ThreadRecord(m_jvmti, NULL);
}


TraceAgent::~TraceAgent()
{
    delete m_thread_record;
	delete m_filter;
}

jvmtiEnv* TraceAgent::CreateJvmtiEnv(JavaVM *vm) const {
    jvmtiEnv *jvmti = NULL;

	// creates a new JVM TI connection and thus a new JVMTI environment
	jint ret = (vm)->GetEnv(reinterpret_cast<void**>(&jvmti), JVMTI_VERSION);
	if (ret != JNI_OK || jvmti == NULL) {
		throw AgentException(JVMTI_ERROR_INTERNAL);
	}

	return jvmti;
}

jvmtiEnv* TraceAgent::GetJvmtiEnv() 
{
	return m_jvmti;
}

void TraceAgent::AddCapability() const
{
    jvmtiCapabilities caps;

    memset(&caps, 0, sizeof(caps));
    caps.can_generate_method_entry_events = 1;
	caps.can_generate_method_exit_events = 1;
	caps.can_access_local_variables = 1;
	caps.can_tag_objects = 1;
	caps.can_generate_field_access_events = 1;
	caps.can_generate_field_modification_events = 1;
    
    jvmtiError error = m_jvmti->AddCapabilities(&caps);
	Errors::Check(error);
}

void TraceAgent::RegisterEvent() const
{
    jvmtiEventCallbacks callbacks;
    jvmtiError error;

    memset(&callbacks, 0, sizeof(callbacks));

	if(m_filter->IsInterested("method")) {
		callbacks.MethodEntry = &TraceAgent::HandleMethodEntry;
		callbacks.MethodExit = &TraceAgent::HandleMethodExit;

		error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, 0);
		Errors::Check(error);
		error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, 0);
		Errors::Check(error);
	}

	if(m_filter->IsInterested("field")) {
		callbacks.ClassPrepare = &TraceAgent::HandleClassPrepare;
		callbacks.FieldAccess = &TraceAgent::HandleFieldAccess; // must enable HandleClassPrepare first
		callbacks.FieldModification = &TraceAgent::HandleFieldModification; // must enable HandleClassPrepare first

		error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, 0);
		Errors::Check(error);
		error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_FIELD_ACCESS, 0);
		Errors::Check(error);
		error = m_jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION, 0);
		Errors::Check(error);
	}
	    
    error = m_jvmti->SetEventCallbacks(&callbacks, static_cast<jint>(sizeof(callbacks)));
	Errors::Check(error);	
}
  
void TraceAgent::PrintLineNum(size_t num) {
	printf(JOIN_STR("\r# %-", STR(LOG_LINE_NUM_SPACE), "d"), num);
}

void JNICALL TraceAgent::HandleMethodEntry(jvmtiEnv* jvmti, JNIEnv* jni, jthread thread, jmethodID method)
{
	try {
        jvmtiError error;
        jclass clazz;
        char* name;
		char* signature;
        
        error = m_jvmti->GetMethodDeclaringClass(method, &clazz); 
		Errors::Check(error);
        error = m_jvmti->GetClassSignature(clazz, &signature, 0);
		Errors::Check(error);
        error = m_jvmti->GetMethodName(method, &name, NULL, NULL);
		Errors::Check(error);

		char tmp[MAX_LOG_LENGTH]; 
		sprintf_s(tmp, "%s%s", signature, name); 

		if(m_filter->Match("method", tmp)) {
			char out[MAX_LOG_LENGTH], *pout = out;
			jint param_size = 0;
			
			TabBuf(&pout, out + sizeof(out) - pout, m_thread_record->GetDepth(thread));
			m_thread_record->IncreaseDepth(thread);

			// param_size is slot number that the arguments take
			m_jvmti->GetArgumentsSize(method, &param_size); 
			
			FillBuf(&pout, out + sizeof(out) - pout, "method_entry: %s%s", signature, name);

			// get the tag of the parameter object
			jint entry_count = 0;
			jvmtiLocalVariableEntry *table_ptr = NULL;		

			// current execution location: the argument variables are just available

			// entry_count equals to the local variable number, including the arguments
			m_jvmti->GetLocalVariableTable(method, &entry_count, &table_ptr); 			
			for(int j = 0; j < entry_count && table_ptr[j].slot < param_size; j++) {				
				// for non-static method, the first argument is "this"
				if(table_ptr[j].signature[0] == 'L') { // object
					jobject param_obj;
					jlong param_obj_tag = 0; 
					
					error = m_jvmti->GetLocalObject(thread, 0, table_ptr[j].slot, &param_obj); // frame at depth zero is the current frame
					Errors::Check(error);
					
					param_obj_tag = GetObjectTag(param_obj);
					
					if(strcmp("this", table_ptr[j].name) ==0)
						FillBuf(&pout, out + sizeof(out) - pout, ", obj_tag: %ld", param_obj_tag);
					else
						FillBuf(&pout, out + sizeof(out) - pout, ", param_obj_tag: %ld", param_obj_tag);
						
					jni->DeleteLocalRef(param_obj);
					m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr[j].signature));
					m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr[j].name));
					m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr[j].generic_signature));
				}
				
			} 

			m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(table_ptr));
			
			// thread id			
			FillBuf(&pout, out + sizeof(out) - pout, ", thread_id: %ld", m_thread_record->GetThreadID(thread));

			// output to log
			m_log_list.push_back(out);

			PrintLineNum(m_log_list.size());
		}

		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));

	} catch (AgentException& e) {
		cout << "\nError inside HandleMethodEntry: " << Errors::GetErrorName(e.ErrCode()) << " (" << e.ErrCode() << ")";
    }
}

void JNICALL TraceAgent::HandleMethodExit(jvmtiEnv *jvmti, JNIEnv* jni, jthread thread, jmethodID method, jboolean was_popped_by_exception, jvalue return_value) 
{
	try {
        jvmtiError error;
        jclass clazz;
        char* name;
		char* signature;
        
        error = m_jvmti->GetMethodDeclaringClass(method, &clazz);
        Errors::Check(error);
        error = m_jvmti->GetClassSignature(clazz, &signature, 0);
        Errors::Check(error);
        error = m_jvmti->GetMethodName(method, &name, NULL, NULL);
        Errors::Check(error);

		char tmp[MAX_LOG_LENGTH]; 
		sprintf_s(tmp, "%s%s", signature, name);
		if(m_filter->Match("method", tmp)) {
			char out[MAX_LOG_LENGTH], *pout = out;

			m_thread_record->DecreaseDepth(thread);
			TabBuf(&pout, out + sizeof(out) - pout, m_thread_record->GetDepth(thread));
		
			FillBuf(&pout, out + sizeof(out) - pout, "method_exit: %s%s", signature, name);

			// thread id
			FillBuf(&pout, out + sizeof(out) - pout, ", thread_id: %ld", m_thread_record->GetThreadID(thread));	

			m_log_list.push_back(out);

			PrintLineNum(m_log_list.size());
		}

        m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
        m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));

	} catch (AgentException& e) {
		cout << "\nError when enter HandleMethodExit: " << e.What() << " [" << e.ErrCode() << "]";
	} 	
}

void JNICALL TraceAgent::HandleClassPrepare(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jclass klass) 
{
	if(!m_filter->ContainsKey("field")) {
		return;
	}

	jvmtiError error;
    char *signature;	

	m_jvmti->GetClassSignature(klass, &signature, NULL);

	char out[MAX_LOG_LENGTH]; 
	sprintf_s(out, "%s", signature);	

	if(m_filter->Match("field", out)) { // monitor all fields of this class
		jint field_num;
		jfieldID * fieldIDs;
		error = jvmti_env->GetClassFields(klass, &field_num, &fieldIDs);
		Errors::Check(error);

		for(int i=0; i< field_num; i++)
		{
			error = jvmti_env->SetFieldAccessWatch(klass, fieldIDs[i]);
			Errors::Check(error);
			error = jvmti_env->SetFieldModificationWatch(klass, fieldIDs[i]);
			Errors::Check(error);
		}
		jvmti_env->Deallocate((unsigned char*)fieldIDs);
	}

	m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
}

void JNICALL TraceAgent::HandleFieldAccess(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method,
            jlocation location, jclass field_klass, jobject object, jfieldID field)
{
	jvmtiError error;
    char* name, *signature;
	jclass dec_cls;
	char* dec_cls_sig;		
	jclass method_dec_cls;
	char* method_name, *method_dec_cls_sig;
	jlong obj_size = 0;
	jint obj_hash = 0;
	jlong field_obj_tag = 0;
	jlong owner_obj_tag = 0;

	if(m_jni_access == 1) {
		m_jni_access = 0;
		return;
	}

	try {	
   
		// get the signature of the field
		error = m_jvmti->GetFieldName(field_klass, field, &name, &signature, NULL);
        Errors::Check(error);

		// get the signature of the class that declares the field
		m_jvmti->GetFieldDeclaringClass(field_klass, field, &dec_cls);
		m_jvmti->GetClassSignature(dec_cls, &dec_cls_sig, NULL);

		// get the signature of the method where the access occurs
		m_jvmti->GetMethodName(method, &method_name, NULL, NULL);
		m_jvmti->GetMethodDeclaringClass(method, &method_dec_cls);
		m_jvmti->GetClassSignature(method_dec_cls, &method_dec_cls_sig, NULL);

		
		if(object != NULL) {	
			owner_obj_tag = GetObjectTag(object);

			m_jni_access = 1;
			jobject field_obj = jni_env->GetObjectField(object, field);
			if(field_obj != 0) {
				field_obj_tag = GetObjectTag(field_obj);
			}
		}		

		char out[MAX_LOG_LENGTH]; 
		sprintf_s(out, "%s%s%, %s, %s%s", signature, name, dec_cls_sig, method_dec_cls_sig, method_name); 
	
		if(m_filter->Match("field", out)) {
			sprintf_s(out, 
				"field_access: %s%s, dec_cls: %s, in_method: %s%s, field_obj_tag: %ld, owner_obj_tag: %ld, field_obj_id: %lld:%ld", 
				signature, name, dec_cls_sig, method_dec_cls_sig, method_name, field_obj_tag, owner_obj_tag,
				obj_size, obj_hash); 
			m_log_list.push_back(out);

			PrintLineNum(m_log_list.size()); 
		}
			

        // release 
        m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
        m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(dec_cls_sig));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_name));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_dec_cls_sig));

	} catch (AgentException& e) {
		cout << "\nError when enter HandleFieldAccess: " << e.What() << " [" << e.ErrCode() << "]";
    }
}

void JNICALL TraceAgent::HandleFieldModification(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method,
            jlocation location, jclass field_klass, jobject object, jfieldID field, char signature_type, jvalue new_value)
{
	jvmtiError error;
    char* name, *signature;
	jclass dec_cls;
	char* dec_cls_sig;		
	jclass method_dec_cls;
	char* method_name, *method_dec_cls_sig;
	jlong field_obj_tag = 0;
	jlong owner_obj_tag = 0;

	try {	
		// get the signature of the field
		error = m_jvmti->GetFieldName(field_klass, field, &name, &signature, NULL);

		// get the signature of the field's declaring class
		m_jvmti->GetFieldDeclaringClass(field_klass, field, &dec_cls);
		m_jvmti->GetClassSignature(dec_cls, &dec_cls_sig, NULL);

		// get the signature of the method where the modification occurs
		m_jvmti->GetMethodName(method, &method_name, NULL, NULL);
		m_jvmti->GetMethodDeclaringClass(method, &method_dec_cls);
		m_jvmti->GetClassSignature(method_dec_cls, &method_dec_cls_sig, NULL);
		
		if(object != NULL) { // owner object of the field, NULL for static fields
			owner_obj_tag = GetObjectTag(object);
		}		

		if(JNIInvalidRefType != jni_env->GetObjectRefType(new_value.l)) {
			field_obj_tag = GetObjectTag(new_value.l);
		}

		char tmp[MAX_LOG_LENGTH], out[MAX_LOG_LENGTH], *pout = out; 
		sprintf_s(tmp, "%s%s, %s, %s%s", signature, name, dec_cls_sig, method_dec_cls_sig, method_name); 
	
		if(m_filter->Match("field", tmp)) {
			TabBuf(&pout, out + sizeof(out) - pout, m_thread_record->GetDepth(thread) + 2);

			FillBuf(&pout, out + sizeof(out) - pout, "field_modi: %s%s, dec_cls: %s, in_method: %s%s, " \
				"field_obj_tag: %ld, owner_obj_tag: %ld, thread_id: %ld", signature, name, dec_cls_sig, 
				method_dec_cls_sig, method_name, field_obj_tag, owner_obj_tag, m_thread_record->GetThreadID(thread)); 

			m_log_list.push_back(out);

			PrintLineNum(m_log_list.size());
		}			

        // release
        m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(name));
        m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(signature));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(dec_cls_sig));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_name));
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(method_dec_cls_sig));

	} catch (AgentException& e) {
		cout << "\nError inside HandleFieldModification: " << Errors::GetErrorName(e.ErrCode()) << " (" << e.ErrCode() << ")";
    }
}


TraceFilter* TraceAgent::GetFilter() 
{
	return m_filter;
}

list<string> TraceAgent::GetLogList() 
{
	return m_log_list;
}

jlong TraceAgent::GetObjectTag(jobject object) {
	jlong tag;

	m_jvmti->GetTag(object, &tag);
	if(tag == 0) {
		m_jvmti->SetTag(object, m_ctag);
		tag = m_ctag;
		++m_ctag;
	}

	return tag;
}