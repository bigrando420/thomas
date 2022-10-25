// No dependencies aside from sokol_time.h used in TH_CoroutineWait

// idea originated from: https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html

typedef struct TH_Coroutine TH_Coroutine;
struct TH_Coroutine
{
	int line;
	uint64_t start_time;
};

#define TH_CoroutineBegin(coro) switch (coro->line) {case 0: coro->line = 0;
#define TH_CoroutineYield(coro) do { coro->line = __LINE__; return; case __LINE__:;} while(0)
#define TH_CoroutineYieldUntil(coro, condition) while (!(condition)) { TH_CoroutineYield(coro); }
#define TH_CoroutineWait(coro, duration) do {if (coro->start_time == 0.0f) { coro->start_time = stm_now(); } TH_CoroutineYieldUntil(coro, stm_sec(stm_now()) > stm_sec(coro->start_time) + (double)duration); coro->start_time = 0; } while (0)
#define TH_CoroutineExit(coro) do { coro->line = __LINE__; } while (0); }
#define TH_CoroutineReset(coro) do { coro->line = 0; } while (0); }