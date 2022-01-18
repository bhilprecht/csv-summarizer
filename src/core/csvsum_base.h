#pragma once

#include "fort.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <chrono>

using std::vector;

namespace csvsum
{
    struct CellStats
    {
        double max = std::numeric_limits<double>::min();
        double min = std::numeric_limits<double>::max();
        double avg = 0;
        double float_frac;
        long no_distinct_vals;
        bool has_numeric_rows = false;
        vector<std::string> most_frequent;
    };

    class CSVSummarizer
    {
    protected:
        std::string path;
        bool header;
        char sep;
        char line_break;
        char escape_char;
        char quotechar;
        int no_most_freq;

        void _summarize(bool sample, int sample_size, bool verbose)
        {
            std::cout << "Reading path: " << this->path << std::endl;

            long long file_size;
            long long no_rows;
            vector<int> row_sizes;

            auto begin = std::chrono::steady_clock::now();
            vector<vector<std::string>> lines = read_lines(row_sizes, file_size, no_rows);
            auto end = std::chrono::steady_clock::now();
            if (verbose)
                std::cout << "Time to read file = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

            begin = std::chrono::steady_clock::now();
            vector<std::string> col_names;
            vector<std::unordered_map<std::string, double>> cell_contents = read_csv_cells(lines, col_names, row_sizes);

            vector<CellStats> stats;
            for (auto &cell_content : cell_contents)
            {
                CellStats c;
                analyze_col(cell_content, c);
                stats.push_back(c);
            }
            end = std::chrono::steady_clock::now();
            if (verbose)
                std::cout << "Time to compute statistics = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

            if (header && stats.size() > col_names.size()) {
                std::cerr << "Some rows contained more values than the number of columns in the header. This could be due to a parsing error (e.g., a misspecified quoting or espace character)." << std::endl;
            }

            std::cout << (sample ? "Estimated total" : "Total") << " no rows: " << no_rows << std::endl;
            if (sample)
            {
                std::cout << "Statistics on sample of size " << sample_size << ":" << std::endl;
            }

            print_summary(stats, col_names);
        }

        void print_summary(vector<CellStats> &stats, vector<std::string> &col_names)
        {
            fort::char_table table;
            table << fort::header
                  << "Column"
                  << "Numeric Vals (%)"
                  << "Avg"
                  << "Min"
                  << "Max"
                  << "No Distinct"
                  << "Frequent Vals" << fort::endr;

            for (int i = 0; i < stats.size(); i++)
            {
                CellStats c = stats[i];

                if (i < col_names.size())
                {
                    table << col_names[i];
                }
                else
                {
                    table << i;
                }

                table << std::setprecision(4) << c.float_frac * 100;
                if (c.has_numeric_rows)
                {
                    table << c.avg << c.min << c.max;
                }
                else
                {
                    table << ""
                          << ""
                          << "";
                }
                table << c.no_distinct_vals;

                std::string mf = "";
                for (int i = c.most_frequent.size() - 1; i >= 0; i--)
                {
                    if (mf != "")
                        mf += ", ";
                    mf += c.most_frequent[i];
                }
                table << mf << fort::endr;
            }

            

            std::cout << table.to_string() << std::endl;
        }

        void inline read_char(const char &c, bool &escaped, bool &quoted, vector<std::string> &line, std::string &cell, vector<vector<std::string>> &lines)
        {
            if (c == quotechar && !escaped)
            {
                quoted = !quoted;
            }
            else if ((c == line_break || c == sep) && !quoted && !escaped)
            {
                line.push_back(cell);
                cell = "";

                if (c == line_break)
                {
                    lines.push_back(line);
                    vector<std::string> newline;
                    line = newline;
                }
            }
            else if (c == escape_char && !escaped)
            {
                escaped = true;
            }
            else
            {
                cell += c;
                escaped = false;
            }
        }

        // Count how often cell values occur in each column
        vector<std::unordered_map<std::string, double>> read_csv_cells(vector<vector<std::string>> &lines, vector<std::string> &col_names, vector<int> &row_sizes)
        {
            vector<std::unordered_map<std::string, double>> cell_contens;
            char c;

            for (int i = 0; i < lines.size(); i++)
            {
                for (int j = 0; j < lines[i].size(); j++)
                {
                    if (i == 0 && header)
                    {
                        col_names.push_back(lines[i][j]);
                    }
                    else
                    {
                        if (j >= cell_contens.size())
                        {
                            std::unordered_map<std::string, double> cm;
                            cell_contens.push_back(cm);
                        }

                        // weight the occurence of each value. If we read the entire file, the weights are just 1. If we sample, we have to compensate that we are more likely to sample
                        // larger rows (i.e., with more characters) so we weight by inverse row size.
                        if (row_sizes.size() == 0)
                        {
                            if (cell_contens[j].count(lines[i][j]))
                            {
                                cell_contens[j][lines[i][j]] += 1;
                            }
                            else
                            {
                                cell_contens[j][lines[i][j]] = 1;
                            }
                        }
                        else
                        {
                            int rs = header ? row_sizes[i - 1] : row_sizes[i];
                            if (cell_contens[j].count(lines[i][j]))
                            {
                                cell_contens[j][lines[i][j]] += (double)1 / rs;
                            }
                            else
                            {
                                cell_contens[j][lines[i][j]] = (double)1 / rs;
                            }
                        }
                    }
                }
            }
            return cell_contens;
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
                    // compute weighted average
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
            c.float_frac = fwsum / wsum;

            while (!q.empty())
            {
                c.most_frequent.push_back(q.top().second);
                q.pop();
            }
        }

        virtual vector<vector<std::string>> read_lines(vector<int> &row_sizes, long long &file_size, long long &no_rows) = 0;

    public:
        CSVSummarizer(std::string path, bool header, char sep, char line_break, char escape_char, char quotechar, int no_most_freq)
            : path(path), header(header), sep(sep), line_break(line_break), escape_char(escape_char), quotechar(quotechar), no_most_freq(no_most_freq)
        {
        }

        virtual void summarize(bool verbose) = 0;
    };

}