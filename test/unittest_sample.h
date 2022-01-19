#pragma once

#include "csvsum.h"
#include "unittest_csvsum.h"
#include <filesystem>
#include <iostream>

using namespace csvsum;

TEST_SUITE("csvsum_sample")
{
    TEST_CASE("no_quote")
    {
        vector<std::string> col_names;
        long long no_rows;
        std::unique_ptr<csvsum::SampleCSVSummarizer> full_sum(new csvsum::SampleCSVSummarizer(resource_dir + "simple_no_quote.csv", true, ','));

        vector<CellStats> stats = full_sum->obtain_stats(false, col_names, no_rows);

        check_simple_no_quote(col_names, no_rows, stats);
    }
}