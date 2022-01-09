#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <boost/lexical_cast.hpp>
#include "fort.hpp"
#include <chrono>

using boost::bad_lexical_cast;
using boost::lexical_cast;
using std::cout;
using std::endl;
using std::max;
using std::min;
using std::numeric_limits;
using std::pair;
using std::priority_queue;
using std::string;
using std::unordered_map;
using std::vector;

namespace csvsum
{

    class CSVSummarizer
    {
    private:
        string path;
        bool header;
        char sep;
        char line_break;
        char escape_char;
        char quotechar;
        int no_most_freq;

        struct CellStats
        {
            double max = numeric_limits<double>::min();
            double min = numeric_limits<double>::max();
            double avg = 0;
            long no_rows = 0;
            long numeric_rows = 0;
            double float_frac;
            long no_distinct_vals;
            vector<string> most_frequent;
        };

    public:
        // Just read the file line per line into a vector of vectors (representing cells)
        // Also consider quoted and escaped newlines.
        vector<vector<string>> read_lines()
        {
            std::ifstream in(path);

            bool escaped = false;
            bool quoted = false;

            int colIdx = 0;
            string cell;
            vector<vector<string>> lines;
            char c;
            vector<string> line;

            if (in.is_open())
            {
                while (in.good())
                {
                    in.get(c);
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
                            vector<string> newline;
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
            }

            if (!in.eof() && in.fail())
                cout << "error reading " << path << endl;

            in.close();
            return lines;
        }

        // Count how often cell values occur
        vector<unordered_map<string, int>> read_csv_cells(vector<vector<string>> &lines, vector<string> &col_names)
        {
            vector<unordered_map<string, int>> cell_contens;
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
                            unordered_map<string, int> cm;
                            cell_contens.push_back(cm);
                        }

                        if (cell_contens[j].count(lines[i][j]))
                        {
                            cell_contens[j][lines[i][j]]++;
                        }
                        else
                        {
                            cell_contens[j][lines[i][j]] = 1;
                        }
                    }
                }
            }
            return cell_contens;
        }

        // Compute the statistics per column
        void analyze_col(unordered_map<string, int> &cm, CellStats &c)
        {
            c.no_distinct_vals = cm.size();
            priority_queue<pair<int, string>> q;

            for (pair<const string, int> &value_count : cm)
            {
                string val = value_count.first;
                int count = value_count.second;

                // try to treat as numeric value and update stats
                try
                {
                    double fval = boost::lexical_cast<double>(val);
                    long no_num_rows = c.numeric_rows + count;
                    c.avg = (double)c.numeric_rows / no_num_rows * c.avg + (double)count / no_num_rows * fval;
                    c.numeric_rows = no_num_rows;
                    c.max = max(c.max, fval);
                    c.min = min(c.min, fval);
                }
                catch (bad_lexical_cast &)
                {
                }

                q.push(make_pair(-count, val));
                if (q.size() > no_most_freq)
                {
                    q.pop();
                }
                c.no_rows += count;
            }

            c.float_frac = (double)c.numeric_rows / c.no_rows;
            while (!q.empty())
            {
                c.most_frequent.push_back(q.top().second);
                q.pop();
            }
        }

        void print_summary(vector<CellStats> &stats, vector<string> &col_names)
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
                if (c.numeric_rows > 0)
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

                string mf = "";
                for (int i = c.most_frequent.size() - 1; i >= 0; i--)
                {
                    if (mf != "")
                        mf += ", ";
                    mf += c.most_frequent[i];
                }
                table << mf << fort::endr;
            }
            cout << table.to_string() << endl;
        }

        CSVSummarizer(string path, bool header, char sep)
        {
            this->path = path;
            this->header = header;
            this->sep = sep;
            this->line_break = '\n';
            this->escape_char = '\\';
            this->quotechar = '"';
            this->no_most_freq = 3;
        }

        void summarize()
        {

            std::cout << "Reading path: " << path << endl;

            /*auto begin = std::chrono::steady_clock::now();
            vector<vector<string>> lines = read_lines();
            auto end = std::chrono::steady_clock::now();
            std::cout << "Time to read file = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;*/

            vector<vector<string>> lines = read_lines();
            vector<string> col_names;
            vector<unordered_map<string, int>> cell_contents = read_csv_cells(lines, col_names);

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
