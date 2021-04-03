#include "stdafx.h"
#include "ThreadRecord.h"
#include "Errors.h"
#include "AgentException.h"
#include <iostream>

ThreadRecord::ThreadRecord(jvmtiEnv *jvmti, JNIEnv* jni) {
	m_thread_count = 0;
	m_jvmti = jvmti;
	m_jni = jni;

	jvmtiError error = jvmti->CreateRawMonitor("lock thread count", &m_thread_count_monitor);
	error = jvmti->CreateRawMonitor("lock depth", &m_depth_monitor);
	Errors::Check(error);
}

ThreadRecord::~ThreadRecord() {
	m_jvmti->DestroyRawMonitor(m_thread_count_monitor);
	m_jvmti->DestroyRawMonitor(m_depth_monitor);

	list<jlong*>::iterator iter;
	for(iter = m_thread_ids.begin(); iter != m_thread_ids.end(); ++iter) {
		delete *iter;
	}
}

int ThreadRecord::GetDepth(jthread thread) {
	return m_depth[GetThreadID(thread)];
}

void ThreadRecord::IncreaseDepth(jthread thread) {
	jlong thread_id = GetThreadID(thread);
	m_jvmti->RawMonitorEnter(m_depth_monitor);
	m_depth[thread_id] += 2;
	m_jvmti->RawMonitorExit(m_depth_monitor);
}

void ThreadRecord::DecreaseDepth(jthread thread) {
	jlong thread_id = GetThreadID(thread);
	m_jvmti->RawMonitorEnter(m_depth_monitor);
	m_depth[thread_id] -= 2;
	m_jvmti->RawMonitorExit(m_depth_monitor);
}

jlong ThreadRecord::GetThreadID(jthread thread) 
{
	jlong *p_thread_id = NULL;
	try{
		jvmtiError error = m_jvmti->GetThreadLocalStorage(thread, (void**)&p_thread_id);
		Errors::Check(error);

		if(p_thread_id == NULL) {
			jlong *p_new_id = new jlong;
			error = m_jvmti->RawMonitorEnter(m_thread_count_monitor);	
			Errors::Check(error);
			*p_new_id = m_thread_count++;				
			m_jvmti->RawMonitorExit(m_thread_count_monitor);		
			error = m_jvmti->SetThreadLocalStorage(thread, p_new_id);	
			Errors::Check(error);				
			p_thread_id = p_new_id;

			m_thread_ids.push_back(p_new_id);
			m_depth.insert(std::pair<jlong, int>(*p_new_id, 0));
		}
	}
	catch(AgentException& e) {
		cout << "Error: " << e.ErrCode() << endl;
		return -1;
	}

	return *p_thread_id;
}