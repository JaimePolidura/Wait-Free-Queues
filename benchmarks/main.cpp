#include "mpsc_bench.h"
#include "spsc_bench.h"

int main() {
    start_spsc_bench();
    start_mpsc_bench();

    return 0;
}