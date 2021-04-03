
#include "stdafx.h"

#include <iostream>
#include <regex>

#include "ClassTraceAgent.h"
#include "TheAgent.h"
#include "Errors.h"
#include "TraceFilter.h"

#include "jvmti.h"

using namespace std;

jvmtiEnv* ClassTraceAgent::m_jvmti = 0;
char ClassTraceAgent::m_lastSignature[1024];

extern TraceFilter* pFilter;
extern list<string> logList;

extern TheAgent *pTheAgent;

ClassTraceAgent::ClassTraceAgent(JavaVM *vm) throw(AgentException) {
	Init(vm);
    AddCapability();
    //RegisterEvent();
}

ClassTraceAgent::ClassTraceAgent(jvmtiEnv *jvmti) throw(AgentException)
{
	m_jvmti = jvmti;
    AddCapability();
    //RegisterEvent();
}
	
ClassTraceAgent::~ClassTraceAgent() throw(AgentException)
{
    
}

void ClassTraceAgent::Init(JavaVM *vm) const throw(AgentException){
    jvmtiEnv *jvmti = 0;
	jint ret = (vm)->GetEnv(reinterpret_cast<void**>(&jvmti), JVMTI_VERSION_1_2);
	if (ret != JNI_OK || jvmti == 0) {
		throw AgentException(JVMTI_ERROR_INTERNAL);
	}
	m_jvmti = jvmti;
}

void ClassTraceAgent::AddCapability() const throw(AgentException)
{
    // 创建一个新的环境
    jvmtiCapabilities caps;
    memset(&caps, 0, sizeof(caps));
	caps.can_generate_field_access_events = 1;
	caps.can_generate_field_modification_events = 1;

	caps.can_generate_method_entry_events = 1;
	caps.can_generate_method_exit_events = 1;
	caps.can_access_local_variables = 1;
	caps.can_tag_objects = 1;
    
    // 设置当前环境
    jvmtiError error = m_jvmti->AddCapabilities(&caps);
	Errors::Check(error);
}
  




