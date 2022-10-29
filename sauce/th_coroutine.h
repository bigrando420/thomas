// No dependencies aside from sokol_time.h used in TH_CoroutineWait
// idea originated from: https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
// Implementation by Andrew Harter

/*

- [x] local state
- [ ] issue with local declartions being skipped by the case label
- [ ] a more robust coroutine, not dependant on lines (this will break when we do hot-reload changes)

did somebody say goto statement?
this would also solve the label naming issue hmmmmmm

*/

typedef struct Coroutine Coroutine;
struct Coroutine
{
	S32 line;
	U64 start_time;
	void* data;
};

#define CoroutineBegin(coro) switch (coro->line) {case 0: coro->line = 0;
#define CoroutineYield(coro) do { coro->line = __LINE__; return; case __LINE__:;} while(0)
#define CoroutineYieldUntil(coro, condition) while (!(condition)) { CoroutineYield(coro); }
#define CoroutineWait(coro, duration) do {if (coro->start_time == 0.0f) { coro->start_time = stm_now(); } CoroutineYieldUntil(coro, stm_sec(stm_now()) > stm_sec(coro->start_time) + (F64)duration); coro->start_time = 0; } while (0)
#define CoroutineEnd(coro) do { coro->line = __LINE__; } while (0); }
#define CoroutineReset(coro) do { coro->line = 0; } while (0); }
#define CoroutineIsFinished(coro) (coro->line == -1)