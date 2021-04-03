
#include "stdafx.h"

/*
 *  Created on: 2018-06
 *	Author: renhao.x@seu.edu.cn
 */
#include <iostream>
#include <fstream>
#include <list>
#include "jvmti.h"
#include "TraceAgent.h"
#include "Output.h"

using namespace std;

TraceAgent *agent;

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved)
{
    cout << "Agent_OnLoad" << endl;

    try{		
		agent = new TraceAgent(vm, options);		
		agent->RegisterEvent();

    } catch (AgentException& e) {
        cout << "Error when creating trace agent: " 
			<< e.What() 
			<< " [" << e.ErrCode() << "]" 
			<< endl;
		return JNI_ERR;
	}
    
	return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
	list<string> logList = agent->GetLogList();
	TraceFilter *filter = agent->GetFilter();
	ofstream out(filter->GetLogPath(), filter->IsAppend()?ios::app:ios::trunc);
	list<string>::iterator iter = logList.begin();
	while(iter != logList.end()) {
		out << *iter << endl;
		iter++;
	}
	out << flush;
    cout << "\nAgent_OnUnload" << endl << flush;

	delete agent;
}
