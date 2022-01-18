#pragma once

#include <csvsum_base.h>
#include <stdlib.h>

namespace csvsum
{
    class SampleCSVSummarizer : public CSVSummarizer
    {
    private:
        int no_samples;
        int skip_value;

        std::string read_surrounding_line(long long &offset, std::ifstream &in)
        {

            in.seekg(offset);

            char c;
            bool escaped = false;
            std::string line = "";

            // forward search
            while (in.good())
            {
                in.get(c);
                line += c;
                if ((c == line_break) && !escaped)
                {
                    break;
                }
                else if (c == escape_char && !escaped)
                {
                    escaped = true;
                }
                else
                {
                    escaped = false;
                }
            }

            // backward search
            std::string prefix = "";
            escaped = false;
            bool done = false;
            long long lastpos = offset;

            while (!done)
            {
                line = prefix + line;
                prefix = "";

                offset -= this->skip_value;
                if (offset < 0)
                {
                    offset = 0;
                    done = true;
                }

                in.seekg(offset);
                for (int i = 0; i < lastpos - offset; i++)
                {
                    in.get(c);
                    prefix += c;
                    if ((c == line_break) && !escaped)
                    {
                        prefix = "";
                        done = true;
                    }
                    else if (c == escape_char && !escaped)
                    {
                        escaped = true;
                    }
                    else
                    {
                        escaped = false;
                    }
                }
                lastpos = offset;
            }
            line = prefix + line;
            return line;
        }

        // Just read the file line per line into a vector of vectors (representing cells)
        // Also consider quoted and escaped newlines.
        vector<vector<std::string>> read_lines(vector<int> &row_sizes, long long &file_size, long long &no_rows)
        {
            std::ifstream in(path);

            bool escaped = false;
            bool quoted = false;
            std::string cell;
            vector<vector<std::string>> lines;
            char c;
            vector<std::string> line;

            // skip header if applicable
            if (header)
            {
                while (in.good() && lines.size() == 0)
                {
                    in.get(c);
                    read_char(c, escaped, quoted, line, cell, lines);
                }
            }

            // find out where file can be read
            long long minlength = in.tellg();
            in.seekg(0, in.end);
            long long maxlength = in.tellg();
            file_size = maxlength - minlength;

            // sample rows and parse them
            std::string currline = "";
            // The goal is to estimate the avg number of chars per row. Since we observe a biased sample (larger rows are seen more often),
            // we have to keep track of inverted sum of row widths
            double inv_rwidth_sum = 0;

            for (int i = 0; i < this->no_samples; i++)
            {
                long long offset = (maxlength - minlength) * (rand() / (double)RAND_MAX) + minlength;
                currline = read_surrounding_line(offset, in);
                row_sizes.push_back(currline.size());

                inv_rwidth_sum += (double)1 / currline.size();

                for (char c : currline)
                {
                    read_char(c, escaped, quoted, line, cell, lines);
                }
            }

            double avg_row_width = this->no_samples / inv_rwidth_sum;
            no_rows = std::round(file_size / avg_row_width);

            in.close();
            return lines;
        }

    public:
        // this summarizer does not support quotechars (since in this case it is not clear when to stop reading).
        // Hence, this character defaults to \0
        SampleCSVSummarizer(std::string path, bool header, char sep, char line_break, char escape_char, int no_most_freq, int no_samples, int skip_value)
            : no_samples(no_samples), skip_value(skip_value), CSVSummarizer(path, header, sep, line_break, escape_char, '\0', no_most_freq)
        {
        }

        SampleCSVSummarizer(std::string path, bool header, char sep, int no_samples, int skip_value)
            : no_samples(no_samples), skip_value(skip_value), CSVSummarizer(path, header, sep, '\n', '\\', '\0', 3)
        {
        }

        SampleCSVSummarizer(std::string path, bool header, char sep) : SampleCSVSummarizer(path, header, sep, 1000, 100)
        {
        }

        void summarize(bool verbose)
        {
            _summarize(true, this->no_samples, verbose);
        }
    };

}