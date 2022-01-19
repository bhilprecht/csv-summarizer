#pragma once

#include "csvsum.h"
#include <filesystem>
#include <iostream>

using namespace csvsum;

std::string resource_dir = TEST_RESOURCE_DIR;

void check_simple_no_quote(vector<std::string> &col_names, long long &no_rows, vector<CellStats> &stats)
{
    // Check that csv was parsed correctly
    CHECK(no_rows == 3);
    CHECK(stats.size() == 2);
    CHECK(col_names.size() == 2);
    CHECK(col_names[0] == "letter");
    CHECK(col_names[1] == "value");

    // Check that stats are correct
    CHECK(!stats[0].has_numeric_rows);
    CHECK(stats[0].no_distinct_vals == 2);
    CHECK(stats[0].most_frequent.size() == 2);
    CHECK(stats[0].most_frequent[0] == "A");
    CHECK(stats[0].most_frequent[1] == "BBBBBBBBB");

    CHECK(stats[1].max == 0.8);
    CHECK(stats[1].min == 0.2);
    CHECK(stats[1].avg == doctest::Approx(0.5).epsilon(0.01));
    CHECK(stats[1].float_frac == doctest::Approx((double)2 / 3).epsilon(0.01));
    CHECK(stats[1].no_distinct_vals == 3);
    CHECK(stats[1].has_numeric_rows);
}

void check_quoted_escaped(vector<std::string> &col_names, long long &no_rows, vector<CellStats> &stats)
{
    // Check that csv was parsed correctly
    CHECK(no_rows == 2);
    CHECK(stats.size() == 2);
    CHECK(col_names.size() == 2);
    CHECK(col_names[0] == "statement");
    CHECK(col_names[1] == "value");

    // Check that stats are correct
    CHECK(!stats[0].has_numeric_rows);
    CHECK(stats[0].no_distinct_vals == 2);
    CHECK(stats[0].most_frequent.size() == 2);

    CHECK(stats[1].max == 0.8);
    CHECK(stats[1].min == 0.2);
    CHECK(stats[1].avg == doctest::Approx(0.5).epsilon(0.01));
    CHECK(stats[1].float_frac == 1.0);
    CHECK(stats[1].no_distinct_vals == 2);
    CHECK(stats[1].has_numeric_rows);
}