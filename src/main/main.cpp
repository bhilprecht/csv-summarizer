#include "../core/csvsum.h"
#include <argparse/argparse.hpp>
#include <iostream>

char to_char(std::string str, std::string arg)
{
    if (str.size() > 1)
    {
        std::cerr << "Only a single character can be specified. However, received input " << str << " for arg " << arg << "." << std::endl;
        std::exit(1);
    }
    else if (str.size() == 0)
    {
        return '\0';
    }

    char c = static_cast<char>(str[0]);
    return c;
}

int main(int argc, char *argv[])
{

    argparse::ArgumentParser program("csv_summarizer");
    program.add_argument("path")
        .help("the location of the csv file to be summarized.");

    program.add_argument("--sep")
        .default_value(std::string(","))
        .help("specify the seperator character used for parsing.");

    program.add_argument("-l", "--line_break")
        .default_value(std::string("\n"))
        .help("specify the seperator character marking a line break.");

    program.add_argument("-e", "--escape_char")
        .default_value(std::string("\\"))
        .help("specify the escape character.");

    program.add_argument("-q", "--quote_char")
        .default_value(std::string(""))
        .help("specify the quote character.");

    program.add_argument("-s", "--sample")
        .default_value(0)
        .required()
        .scan<'d', int>()
        .help("number of rows to sample.");

    program.add_argument("--no_header")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--verbose")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-n", "--no_most_freq")
        .default_value(3)
        .required()
        .scan<'d', int>()
        .help("specify the number of frequent cell values to be printed.");

    program.add_argument("-n", "--block_read")
        .default_value(100)
        .required()
        .scan<'d', int>()
        .help("Number of characters read in a batch in the sample mode.");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    auto path = program.get<std::string>("path");
    char sep = to_char(program.get<std::string>("--sep"), "sep");
    char line_break = to_char(program.get<std::string>("--line_break"), "line_break");
    char escape_char = to_char(program.get<std::string>("--escape_char"), "escape_char");
    char quotechar = to_char(program.get<std::string>("--quote_char"), "quote_char");
    int no_samples = program.get<int>("--sample");
    bool header = !program.get<bool>("--no_header");
    bool verbose = program.get<bool>("--verbose");
    int no_most_freq = program.get<int>("--no_most_freq");
    int block_read = program.get<int>("--block_read");

    if (no_samples == 0)
    {
        auto s = new csvsum::FullCSVSummarizer(path, header, sep, line_break, escape_char, quotechar, no_most_freq);
        s->summarize(verbose);
    }
    else
    {
        if (quotechar != '\0')
        {
            std::cerr << "Quotechars are not supported for sampling mode." << std::endl;
            std::exit(1);
        }
        auto s = new csvsum::SampleCSVSummarizer(path, header, sep, line_break, escape_char, no_most_freq, no_samples, block_read);
        s->summarize(verbose);
    }

    return 0;
}
