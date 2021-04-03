#pragma once

class AgentException 
{
 public:
	AgentException(jvmtiError err) {
		m_error = err;
	}

	char* What() const throw() { 
		return "AgentException"; 
	}

	jvmtiError ErrCode() const throw() {
		return m_error;
	}

 private:
	jvmtiError m_error;
};