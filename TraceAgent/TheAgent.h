
#include "jvmti.h"
#include "AgentException.h"
#include "Errors.h"
#include <atomic>

class TheAgent {
	jvmtiEnv* m_jvmti;

public:
	TheAgent();
	TheAgent(JavaVM *vm);

	void RegisterEvent() const throw(AgentException);

	void Allocate(jlong size, unsigned char **mem_ptr) {
		jvmtiError error = m_jvmti->Allocate(size, reinterpret_cast<unsigned char**>(mem_ptr));
		Errors::Check(error);
	}

	void Deallocate(unsigned char* mem) {
		m_jvmti->Deallocate(reinterpret_cast<unsigned char*>(mem));
	}

	static jlong ctag; // object tag
	static jlong ttag; // thread tag
};