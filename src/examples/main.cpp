#include "../core/csvsum.h"
#include <iostream>

int main(int, char**) {
    auto s = new csvsum::FullCSVSummarizer("../src/examples/test2.csv", true, ',');
    s->summarize(false);

    auto s2 = new csvsum::SampleCSVSummarizer("../src/examples/test2.csv", true, ',', 1000, 20);
    s2->summarize(false);

    return 0;
}
