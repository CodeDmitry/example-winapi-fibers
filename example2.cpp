// | Uses oleaut32(flag -loleaut32) 
// | Demonstrates using fibers as coroutines that can give responses on yield
// |    (and even accept requests, but that is not done here).
#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <stack>
#include <stdlib.h>

// | Our coroutine needs a stack to place results onto, and potentially read 
// |     requests from the caller who called SwitchToFiber,
// |     we use a VARIANT to be able to send and receive "any" typed argument. 
// | Our coroutine needs to know who called it(in order to yield to it using 
// |     SwitchToFiber).
struct co_args_t {
    std::stack<VARIANT> *fiberStack;
    void far *callerFiber;
};

// | Weak contract: memory used by parg is released by the caller.
void __stdcall Fiber1Proc(void far *parg)
{
    int counter = 0;
    co_args_t args;    
        
    // | Copy the entire structure pointed to by parg into args.
    memcpy(&args, parg, sizeof(args));
    
    for (;;) {        
        VARIANT response;
    
        ++counter;
        
        if (counter == 50000) {
            // | make the variant VT_EMPTY, no more.
            VariantInit(&response);
        } else {        
            response.vt = VT_INT;
            response.intVal = counter;
        }
        
        args.fiberStack->push(response);        
        
        SwitchToFiber(args.callerFiber);
    }
}

int main(int argc, char **argv)
{
    void far *fiberMain = nullptr;
    void far *fiber1 = nullptr;
    std::stack<VARIANT> *fiberStack = new std::stack<VARIANT>;    
    co_args_t *args;
 
    // | Convert ourselves to a fiber and pass 
    // |     the stack and ourselves to the fiber.
    assert(fiberMain = ConvertThreadToFiber(0));
    args = new co_args_t {fiberStack, fiberMain};
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

