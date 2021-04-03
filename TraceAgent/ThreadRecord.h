#include "jvmti.h"
#include <map>
#include <list>

using namespace std;

class ThreadRecord {
private:
	std::map<jlong, int> m_depth;
	std::list<jlong*> m_thread_ids;
	jlong m_thread_count;

	jvmtiEnv *m_jvmti;
	JNIEnv *m_jni;

	jrawMonitorID m_thread_count_monitor;
	jrawMonitorID m_depth_monitor;

public:
	ThreadRecord(jvmtiEnv *jvmti, JNIEnv* jni);
	~ThreadRecord();

	int GetDepth(jthread thread);
	void IncreaseDepth(jthread thread);
	void DecreaseDepth(jthread thread);
	jlong GetThreadID(jthread thread);
};