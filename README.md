# TraceAgent

TraceAgent is a Java agent library used for runtime trace. It is deployed as a Dynamic Link Library (.dll) file on the Windows platform.

# Usage

    java -classpath <CLASS_PATH> "-agentpath:<AGENT_PATH>=<PARAM>" <ENTRY_CLASS>

- `CLASS_PATH`: the path to search for classes.
- `AGENT_PATH`: the path of the trace agent.
- `PARAM`: the parameter passed to the trace agent.
- `ENTRY_CLASS`: the Java class to execute.
    
### Parameter

    [method=<REGEX>][#field=<REGEX>][#log=<LOG_PATH>][#append=true]

- `REGEX (regular expression)` matches the [JNI (Java Native Interface)](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/types.html) signature of a method or a field. A `REGEX` starting with "!" matches the negative of the regular expression followed.
- `LOG_PATH`: the path to save the log. If "append=true" is set, new log lines will be appended to the existing log.

# Example

Here is a [HelloWorld](https://github.com/Megre/TraceAgent/blob/main/HelloWorld.java) class:

    public class HelloWorld {
	
    	private final String word = "Hello World!";
    	
    	public static void main(String[] args) {
    		new HelloWorld().sayHello();
    	}
    	
    	public void sayHello() {
    		System.out.println(word);
    	}
    }
    
The following commands can be used (e.g. in the batch file [trace.bat](https://github.com/Megre/TraceAgent/blob/main/trace.bat)) to trace the method and field events during its execution. 

    set agent=x64\Release\TraceAgent.dll
    set logFile=trace.log
    
    java -classpath . "-agentpath:%agent%=method=L(HelloWorld)#field=L(HelloWorld)#log=%logFile%" HelloWorld

Traced events are recorded in [trace.log](https://github.com/Megre/TraceAgent/blob/main/trace.log):

    method_entry: LHelloWorld;main, thread_id: 0
      method_entry: LHelloWorld;<init>, thread_id: 0
          field_modi: Ljava/lang/String;word, dec_cls: LHelloWorld;, in_method: LHelloWorld;<init>, field_obj_tag: 2, owner_obj_tag: 1, thread_id: 0
      method_exit: LHelloWorld;<init>, thread_id: 0
      method_entry: LHelloWorld;sayHello, thread_id: 0
      method_exit: LHelloWorld;sayHello, thread_id: 0
    method_exit: LHelloWorld;main, thread_id: 0
