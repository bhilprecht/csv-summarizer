#pragma once

#include <csvsum_base.h>
#include <fstream>
#include <queue>
#include <boost/lexical_cast.hpp>
#include <chrono>

namespace csvsum
{

    class FullCSVSummarizer : public CSVSummarizer
    {
    private:
    public:
        // Just read the file line per line into a vector of vectors (representing cells)
        // Also consider quoted and escaped newlines.
        vector<vector<std::string>> read_lines(vector<int> &row_sizes, long long &file_size)
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
            if (lines[lines.size()-1].size() == 1 && lines[lines.size()-1][0] == "") {
                lines.pop_back();
            }

            if (!in.eof() && in.fail())
                std::cout << "error reading " << path << std::endl;

            in.close();
            return lines;
        }

        // Compute the statistics per column
        void analyze_col(std::unordered_map<std::string, double> &cm, CellStats &c)
        {

            long no_rows = 0;
            long numeric_rows = 0;
            c.no_distinct_vals = cm.size();
            std::priority_queue<std::pair<int, std::string>> q;

            for (std::pair<const std::string, double> &value_count : cm)
            {
                std::string val = value_count.first;
                int count = static_cast<int>(value_count.second);

                // try to treat as numeric value and update stats
                try
                {
                    double fval = boost::lexical_cast<double>(val);

                    c.has_numeric_rows = true;
                    long no_num_rows = numeric_rows + count;
                    c.avg = (double)numeric_rows / no_num_rows * c.avg + (double)count / no_num_rows * fval;
                    numeric_rows = no_num_rows;
                    c.max = std::max(c.max, fval);
                    c.min = std::min(c.min, fval);
                }
                catch (boost::bad_lexical_cast &)
                {
                }

                q.push(std::make_pair(-count, val));
                if (q.size() > no_most_freq)
                {
                    q.pop();
                }
                no_rows += count;
            }

            c.float_frac = (double)numeric_rows / no_rows;
            while (!q.empty())
            {
                c.most_frequent.push_back(q.top().second);
                q.pop();
            }
        }

        void summarize()
        {

            std::cout << "Reading path: " << this->path << std::endl;

            long long file_size;
            vector<int> row_sizes;
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

        FullCSVSummarizer(std::string path, bool header, char sep) : CSVSummarizer(path, header, sep)
        {
        }
    };
}
