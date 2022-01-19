#include "../core/csvsum.h"
#include <iostream>

int main(int, char**) {
    auto s = new csvsum::FullCSVSummarizer("../test/data/simple_no_quote.csv", true, ',');
    s->summarize(false);

    return 0;
}
