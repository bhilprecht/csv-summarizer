#include "../core/csvsum.h"
#include <iostream>

int main(int, char**) {
    //auto s = new csvsum::FullCSVSummarizer("../src/examples/test2.csv", true, ',');
    //s->summarize();

    auto s = new csvsum::SampleCSVSummarizer("../src/examples/test2.csv", true, ',', 1000000, 20);
    s->summarize();

    //auto s = new csvsum::SampleCSVSummarizer("/home/bhilprecht/Documents/zero-shot-data/datasets/imdb/cast_info.csv", true, ',', 1000, 100);
    //s->summarize();

    return 0;
}
