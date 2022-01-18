#pragma once

#include <csvsum_base.h>
#include <fstream>

namespace csvsum
{

    class FullCSVSummarizer : public CSVSummarizer
    {
    private:
        // Just read the file line per line into a vector of vectors (representing cells)
        // Also consider quoted and escaped newlines.
        vector<vector<std::string>> read_lines(vector<int> &row_sizes, long long &file_size, long long &no_rows)
        {
            std::ifstream in(path);

            bool escaped = false;
            bool quoted = false;

            int colIdx = 0;
            std::string cell;
            vector<vector<std::string>> lines;
            char c;
            vector<std::string> line;

            if (in.is_open())
            {
                while (in.good())
                {
                    in.get(c);
                    read_char(c, escaped, quoted, line, cell, lines);
                }
            }

            // remove last line if applicable
            if (lines[lines.size() - 1].size() == 1 && lines[lines.size() - 1][0] == "")
            {
                lines.pop_back();
            }

            no_rows = lines.size();
            if (header)
                no_rows--;

            if (!in.eof() && in.fail())
                std::cout << "error reading " << path << std::endl;

            in.close();
            return lines;
        }

    public:
        FullCSVSummarizer(std::string path, bool header, char sep, char line_break, char escape_char, char quotechar, int no_most_freq)
            : CSVSummarizer(path, header, sep, line_break, escape_char, quotechar, no_most_freq)
        {
        }

        FullCSVSummarizer(std::string path, bool header, char sep) : CSVSummarizer(path, header, sep, '\n', '\\', '"', 3)
        {
        }

        void summarize(bool verbose)
        {
            _summarize(false, 0, verbose);
        }
    };
}
