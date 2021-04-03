
#include "stdafx.h"

#include "Errors.h"
#include "AgentException.h"
#include <iostream>

void  Errors::Check(jvmtiError error) {
	if (error != JVMTI_ERROR_NONE) {
		throw AgentException(error);
	}
}

char* Errors::GetErrorName(jvmtiError error) {
	switch(error) {
		case 0: return "JVMTI_ERROR_NONE"; break;
		case 10: return "JVMTI_ERROR_INVALID_THREAD"; break;
		case 11: return "JVMTI_ERROR_INVALID_THREAD_GROUP"; break;
		case 12: return "JVMTI_ERROR_INVALID_PRIORITY"; break;
		case 13: return "JVMTI_ERROR_THREAD_NOT_SUSPENDED"; break;
		case 14: return "JVMTI_ERROR_THREAD_SUSPENDED"; break;
		case 15: return "JVMTI_ERROR_THREAD_NOT_ALIVE"; break;
		case 20: return "JVMTI_ERROR_INVALID_OBJECT"; break;
		case 21: return "JVMTI_ERROR_INVALID_CLASS"; break;
		case 22: return "JVMTI_ERROR_CLASS_NOT_PREPARED"; break;
		case 23: return "JVMTI_ERROR_INVALID_METHODID"; break;
		case 24: return "JVMTI_ERROR_INVALID_LOCATION"; break;
		case 25: return "JVMTI_ERROR_INVALID_FIELDID"; break;
		case 31: return "JVMTI_ERROR_NO_MORE_FRAMES"; break;
		case 32: return "JVMTI_ERROR_OPAQUE_FRAME"; break;
		case 34: return "JVMTI_ERROR_TYPE_MISMATCH"; break;
		case 35: return "JVMTI_ERROR_INVALID_SLOT"; break;
		case 40: return "JVMTI_ERROR_DUPLICATE"; break;
		case 41: return "JVMTI_ERROR_NOT_FOUND"; break;
		case 50: return "JVMTI_ERROR_INVALID_MONITOR"; break;
		case 51: return "JVMTI_ERROR_NOT_MONITOR_OWNER"; break;
		case 52: return "JVMTI_ERROR_INTERRUPT"; break;
		case 60: return "JVMTI_ERROR_INVALID_CLASS_FORMAT"; break;
		case 61: return "JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION"; break;
		case 62: return "JVMTI_ERROR_FAILS_VERIFICATION"; break;
		case 63: return "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED"; break;
		case 64: return "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED"; break;
		case 65: return "JVMTI_ERROR_INVALID_TYPESTATE"; break;
		case 66: return "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED"; break;
		case 67: return "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED"; break;
		case 68: return "JVMTI_ERROR_UNSUPPORTED_VERSION"; break;
		case 69: return "JVMTI_ERROR_NAMES_DONT_MATCH"; break;
		case 70: return "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED"; break;
		case 71: return "JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED"; break;
		case 79: return "JVMTI_ERROR_UNMODIFIABLE_CLASS"; break;
		case 98: return "JVMTI_ERROR_NOT_AVAILABLE"; break;
		case 99: return "JVMTI_ERROR_MUST_POSSESS_CAPABILITY"; break;
		case 100: return "JVMTI_ERROR_NULL_POINTER"; break;
		case 101: return "JVMTI_ERROR_ABSENT_INFORMATION"; break;
		case 102: return "JVMTI_ERROR_INVALID_EVENT_TYPE"; break;
		case 103: return "JVMTI_ERROR_ILLEGAL_ARGUMENT"; break;
		case 104: return "JVMTI_ERROR_NATIVE_METHOD"; break;
		case 106: return "JVMTI_ERROR_CLASS_LOADER_UNSUPPORTED"; break;
		case 110: return "JVMTI_ERROR_OUT_OF_MEMORY"; break;
		case 111: return "JVMTI_ERROR_ACCESS_DENIED"; break;
		case 112: return "JVMTI_ERROR_WRONG_PHASE"; break;
		case 113: return "JVMTI_ERROR_INTERNAL"; break;
		case 115: return "JVMTI_ERROR_UNATTACHED_THREAD"; break;
		case 116: return "JVMTI_ERROR_INVALID_ENVIRONMENT"; break;
		default: return "Unkonwn Error";
	}
}