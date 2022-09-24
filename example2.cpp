// | Uses oleaut32(flag -loleaut32) 
// | Demonstrates using fibers as coroutines that can give responses on yield
// |    (and even accept requests, but that is not done here).
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stack>

// | Our coroutine needs a stack to place results onto, and potentially read 
// |     requests from the caller who called SwitchToFiber,
// |     we use a VARIANT to be able to send and receive "any" typed argument. 
// | Our coroutine needs to know who called it(in order to yield to it using 
// |     SwitchToFiber).
struct CustomFiberProcArgs1 {
    std::stack<VARIANT> *fiberStack;
    LPVOID callerFiber;
};

// | Weak contract: memory used by parg is released by the caller.
void WINAPI Fiber1Proc(LPVOID pArg)
{
    int counter = 0;
    CustomFiberProcArgs1 args;    
        
    // | Copy the entire structure pointed to by pArg into args.
    memcpy(&args, pArg, sizeof(args));
    
    for (;;) {        
        VARIANT response;
    
        ++counter;
        
        if (counter == 50000) {
            // | No more iterations, prepare to respond with null.
            // | VariantInit initializes to VT_EMPTY akin to "undefined".
            VariantInit(&response);
            // | Our value is defined but null.
            response.vt = VT_NULL;
        } else {        
            // | Prepare to respond with the counter.
            response.vt = VT_INT;
            response.intVal = counter;
        }
        
        // "yield response"
        {
            args.fiberStack->push(response);        
            SwitchToFiber(args.callerFiber);
        }
    }
}

int main(int argc, char **argv)
{
    LPVOID fiberMain = (LPVOID)0;
    LPVOID fiber1 = (LPVOID)0;
    std::stack<VARIANT> *fiberStack = new std::stack<VARIANT>;    
    CustomFiberProcArgs1 *args;
 
    // | Convert ourselves to a fiber and pass 
    // |     the stack and ourselves to the fiber.
    assert(fiberMain = ConvertThreadToFiber(0));
    args = new CustomFiberProcArgs1{fiberStack, fiberMain};
    assert(fiber1 = CreateFiber(0, &Fiber1Proc, args));

    // | Interact with our coroutine.
    for (;;) {            
        VARIANT response;
        SwitchToFiber(fiber1);   
        response = fiberStack->top();
        if (response.vt == VT_INT) {        
            printf("%d.\n", response.intVal);
        } else {
            puts("coroutine completed.");
            break;
        }
        fiberStack->pop();
    }

    DeleteFiber(fiber1);
    
    // | clear our stack of any potential remaining items.
    while (fiberStack->size() > 0) {
        VARIANT arg = fiberStack->top();
        VariantClear(&arg);
        fiberStack->pop();
    }
    
    delete fiberStack;
    delete args;

    return 0;
}
