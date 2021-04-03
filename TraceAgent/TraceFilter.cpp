#include "stdafx.h"

#include "jvmti.h"
#include "Errors.h"
#include "TraceAgent.h"

#include <map>
#include <list>
#include <iostream>
#include <string>
#include <boost/regex.hpp>

extern TraceAgent *agent;

using namespace std;

/**
 * jvmti: jvmti environment
 * filter: [method=<REGEX>][#field=<REGEX>][#log=<LOG_PATH>][#append=true]
 *		<REGEX> (regular expression) matches the JNI (Java Native Interface) signature of a method or a field.
 *		A <REGEX> starting with "!" matches the negative of the following regular expression.
 *		If "append=true" is set, new log lines will be appended to existing log.
 */
TraceFilter::TraceFilter(jvmtiEnv *jvmti, char *filter) { 
	m_jvmti = jvmti; 
	
	if(filter != NULL) {
		
		size_t len = strlen(filter);
		if(len > 0) {
			m_jvmti->Allocate(len + 1, reinterpret_cast<unsigned char**>(&m_filter));
			strcpy_s(m_filter, len + 1, filter);
			
			map<string, string> m;
			char *ctx;
			char *arr = strtok_s(m_filter, "#", &ctx);
			while(arr != NULL) {
				string str(arr);
				size_t pos = str.find('=', 0);
				if(pos != std::string::npos) {
					string k = str.substr(0, pos), v = str.substr(pos+1);
					if(!v.empty())
						m.insert(pair<string,string>(k, v)); // e.g. ("method", "LCH/ifa/")
				}
				arr = strtok_s(NULL, "#", &ctx);
			}
			
			map<string, string>::iterator iter = m.begin();
			while(iter != m.end()) {
				string k = iter->first, v = iter->second; // k:"method", v:"LCH/ifa/"
				list<string> lst;
				list<boost::regex> rev;
				char* arr = strtok_s(const_cast<char*>(v.c_str()), "&", &ctx); // "&" is used to connect two regexes, e.g. method=action&!\$
				while(arr != NULL) {
					string str = string(arr);
					lst.push_back(str);
					rev.push_back(str[0]=='!'?boost::regex(str.substr(1)):boost::regex(arr)); // support "!": generate regex in advance
					arr = strtok_s(NULL, "&", &ctx);
				}
				m_filterMap.insert(pair<string,list<string>>(k, lst)); // ("method", ["action", "!\$"]): filter map
				m_regMap.insert(pair<string,list<boost::regex>>(k, rev)); // ("method", [regex("action"), regex("\$")]): regex map
				iter++;
			}

			// print filter
			cout << "filter:" << endl;
			boost::unordered_map<string, list<string>>::iterator iter1 = m_filterMap.begin();			
			while(iter1 != m_filterMap.end()) {
				string k = iter1->first;
				list<string> v = iter1->second;
				cout << "  " << k << ": ";
				list<string>::iterator iter2 = v.begin();
				while(iter2 != v.end()) {
					cout << *iter2;
					iter2++;
					if(iter2 != v.end())
						cout << " and ";
				}
				cout << endl;
				iter1++;
			}
		}
	}
}

TraceFilter::~TraceFilter() {
	m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(m_filter));
}

bool TraceFilter::IsInterested(char* eventType) {
	string setype(eventType);
	boost::unordered_map<string, list<string>>::iterator iter = m_filterMap.find(setype);
	return iter != m_filterMap.end();
}

bool TraceFilter::Match(char* key, char* str) { 
	string skey(key);
	boost::unordered_map<string, list<string>>::iterator iter_filter_map = m_filterMap.find(skey);
	if(iter_filter_map != m_filterMap.end()) { // find value of key(e.g. "method") in filter map
		boost::unordered_map<string, list<boost::regex>>::iterator iter_reg_map = m_regMap.find(skey); // find regex in regex map
		list<string>::iterator iter_filter = iter_filter_map->second.begin(); // filter iteration
		list<boost::regex>::iterator iter_reg = iter_reg_map->second.begin(); // regex iteration
		while(iter_filter != iter_filter_map->second.end()) { // does not reach the last filter (& operation)
			string v = *iter_filter; 
			bool rev = (v[0]=='!'); // reverse flag
			bool is_match = boost::regex_search(str, *iter_reg);
			if(is_match == rev)
				return false;
			iter_filter++;
			iter_reg++;
		}
		return true;
	}
	return false;
}

char* TraceFilter::Get(char* key) {
	string k(key);
	boost::unordered_map<string, list<string>>::iterator iter = m_filterMap.find(k);
	if(iter != m_filterMap.end()) {
		list<string>::iterator iter_lst = iter->second.begin();		
		if(iter_lst != iter->second.end()) 
			return const_cast<char*>((*iter_lst).c_str());
	}
	return NULL;
}

bool TraceFilter::ContainsKey(char *key) 
{
	string k(key);
	boost::unordered_map<string, list<string>>::iterator iter = m_filterMap.find(k);
	return iter != m_filterMap.end();
}

char* TraceFilter::GetLogPath()
{
	char *log = Get("log");
	return log==NULL?"trace.log":log;
}

bool TraceFilter::IsAppend()
{
	char *append = Get("append");
	return append != NULL && strcmp(append, "true")==0;
}

