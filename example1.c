#include <Windows.h>
#include <stdio.h>
#include <assert.h>

LPVOID fiberMain;
LPVOID fiber1;
LPVOID fiber2;
int counter = 0;

VOID WINAPI Fiber1Proc(LPVOID unused)
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

VOID WINAPI Fiber2Proc(LPVOID unused)
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
    setvbuf(stdout, (char *)0, _IONBF, 0);

    assert(fiber1 = CreateFiber(0, &Fiber1Proc, (LPVOID)0));
    assert(fiber2 = CreateFiber(0, &Fiber2Proc, (LPVOID)0));
    
    assert(fiberMain = ConvertThreadToFiber((LPVOID)0));

    SwitchToFiber(fiber1);

    DeleteFiber(fiber1);
    DeleteFiber(fiber2);

    return 0;
}

