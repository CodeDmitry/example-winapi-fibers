# poc-winapi-fibers
Demonstrates basic usage of WINAPI Fibers.

Why do fibers/coroutines matter? Because nesting state machines gets really ugly really fast
(think a stoppable process, containing a subprocess, containing a subprocess, each of which needs
to only do one step at a time), and fibers/coroutines makes nesting state machines much cleaner, and without needing
to explicitly specify the state transitions, despite being incredibly difficult to initially wrap your head around.

