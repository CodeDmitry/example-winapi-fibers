#include <Windows.h>
#include <stdio.h>
#include <assert.h>

void far *fiberMain;
void far *fiber1;
void far *fiber2;
int counter = 0;

void __stdcall Fiber1Proc(void far *unused)
{
    for (;;) {
        puts("ping");
        ++counter;
        if (counter == 10) {
            SwitchToFiber(fiberMain);
        }
        Sleep(1000);
        SwitchToFiber(fiber2);
    }
}

void __stdcall Fiber2Proc(void far *unused)
{
    for(;;) {
        puts("pong");
        ++counter;
        if (counter == 10) {
            SwitchToFiber(fiberMain);
        }
        Sleep(1000);
        SwitchToFiber(fiber1);
    }
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);

    assert(fiber1 = CreateFiber(0, &Fiber1Proc, 0));
    assert(fiber2 = CreateFiber(0, &Fiber2Proc, 0));
    
    assert(fiberMain = ConvertThreadToFiber(0));

    SwitchToFiber(fiber1);

    DeleteFiber(fiber1);
    DeleteFiber(fiber2);

    return 0;
}

