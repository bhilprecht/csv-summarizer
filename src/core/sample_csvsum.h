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

    public:
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
        vector<vector<std::string>> read_lines(vector<int> &row_sizes, long long &file_size)
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
            for (int i = 0; i < this->no_samples; i++)
            {
                long long offset = (maxlength - minlength) * (rand() / (double)RAND_MAX) + minlength;
                currline = read_surrounding_line(offset, in);
                row_sizes.push_back(currline.size());

                for (char c : currline)
                {
                    read_char(c, escaped, quoted, line, cell, lines);
                }
            }

            in.close();
            return lines;
        }

        SampleCSVSummarizer(std::string path, bool header, char sep, int no_samples, int skip_value) : no_samples(no_samples), skip_value(skip_value), CSVSummarizer(path, header, sep)
        {
        }

        SampleCSVSummarizer(std::string path, bool header, char sep) : SampleCSVSummarizer(path, header, sep, 1000, 100)
        {
        }

        // Compute the statistics per column
        void analyze_col(std::unordered_map<std::string, double> &cm, CellStats &c)
        {
            c.no_distinct_vals = cm.size();
            std::priority_queue<std::pair<int, std::string>> q;

            double fwsum = 0;
            double wsum = 0;

            for (auto &value_count : cm)
            {
                std::string val = value_count.first;
                double w = value_count.second;
                wsum += w;

                // try to treat as numeric value and update stats
                try
                {
                    double fval = boost::lexical_cast<double>(val);

                    c.has_numeric_rows = true;
                    c.avg += w * fval;
                    fwsum += w;

                    c.max = std::max(c.max, fval);
                    c.min = std::min(c.min, fval);
                }
                catch (boost::bad_lexical_cast &)
                {
                }

                q.push(std::make_pair(-w, val));
                if (q.size() > no_most_freq)
                {
                    q.pop();
                }
            }
            c.avg /= fwsum;
            c.float_frac = fwsum/wsum;

            while (!q.empty())
            {
                c.most_frequent.push_back(q.top().second);
                q.pop();
            }
        }

        vector<CellStats> analyze_columns(vector<std::string> &col_names, vector<vector<std::string>> &lines, vector<int> &row_sizes, long long &file_size)
        {
            vector<CellStats> stats;
            for (int j = 0; j < col_names.size(); j++)
            {
                CellStats c;
                for (int i = 0; i < lines.size(); i++)
                {
                    if (lines[i].size() >= j)
                        continue;

                    std::string val = lines[i][j];
                    int rs = row_sizes[i];
                }

                stats.push_back(c);
            }
            return stats;
        }

        void summarize()
        {

            std::cout << "Reading path: " << this->path << std::endl;

            /*auto begin = std::chrono::steady_clock::now();
            auto end = std::chrono::steady_clock::now();
            std::cout << "Time to read file = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;*/

            vector<int> row_sizes;
            long long file_size;
            vector<vector<std::string>> lines = read_lines(row_sizes, file_size);
            vector<std::string> col_names;
            vector<std::unordered_map<std::string, double>> cell_contents = read_csv_cells(lines, col_names, row_sizes);

            vector<CellStats> stats;
            for (auto &cell_content : cell_contents)
            {
                CellStats c;
                analyze_col(cell_content, c);
                stats.push_back(c);
            }

            print_summary(stats, col_names);
        }
    };

}