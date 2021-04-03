
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <list>
#include <string>
#include "jvmti.h"

using namespace std;

class TraceFilter {

private:
	char* m_filter;
	jvmtiEnv *m_jvmti;

	boost::unordered_map<string, list<string>> m_filterMap;
	boost::unordered_map<string, list<boost::regex>> m_regMap;

	char* Get(char* key);

public:
	TraceFilter(jvmtiEnv *jvmti, /*JavaVM *vm*/ char* filter);
	~TraceFilter();

	bool Match(char* agent, char* str);
	bool IsInterested(char* eventType);

	char* GetLogPath();
	bool IsAppend();

	bool ContainsKey(char *key);
};