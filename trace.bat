
set agent=x64\Release\TraceAgent.dll
set logFile=trace.log

java -classpath . "-agentpath:%agent%=method=L(HelloWorld)#field=L(HelloWorld)#log=%logFile%" HelloWorld
