#include "../core/csvsum.h"
#include <iostream>

int main(int, char**) {
    auto s = new csvsum::CSVSummarizer("../src/examples/test.csv", false, ',');
 
    s->summarize();
    return 0;
}
