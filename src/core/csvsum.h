#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <boost/lexical_cast.hpp>
#include "fort.hpp"

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

        vector<unordered_map<string, int>> read_csv_cells()
        {
            int colIdx = 0;
            string cell;
            vector<unordered_map<string, int>> cell_contens;

            char c;

            // todo header
            // todo escapechar
            std::fstream fin(path, std::fstream::in);
            while (fin >> std::noskipws >> c)
            {
                if (c == sep || c == line_break)
                {
                    // todo optimize away
                    if (colIdx >= cell_contens.size())
                    {
                        unordered_map<string, int> cm;
                        cell_contens.push_back(cm);
                    }

                    if (cell_contens[colIdx].count(cell))
                    {
                        cell_contens[colIdx][cell]++;
                    }
                    else
                    {
                        cell_contens[colIdx][cell] = 1;
                    }
                    cell = "";
                    colIdx = c == line_break ? 0 : colIdx + 1;
                }
                else
                {
                    cell += c;
                }
            }

            return cell_contens;
        }

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
                if (q.size() > 3)
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

    public:
        CSVSummarizer(string path, bool header, char sep)
        {
            this->path = path;
            this->header = header;
            this->sep = sep;
            this->line_break = '\n';
            this->escape_char = '\\';
        }

        void summarize()
        {
            fort::char_table table;
            table << fort::header
                  << "Numeric Vals (%)"
                  << "Avg"
                  << "Min"
                  << "Max"
                  << "No Distinct"
                  << "Frequent Vals" << fort::endr;

            std::cout << "Reading path: " << path << endl;
            vector<unordered_map<string, int>> cell_contens = read_csv_cells();
            for (int i = 0; i < cell_contens.size(); i++)
            {
                CellStats c;
                analyze_col(cell_contens[i], c);

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
                    if (i < c.most_frequent.size() - 1)
                        mf += ", ";
                    mf += c.most_frequent[i];
                }
                table << mf << fort::endr;
            }
            std::cout << table.to_string() << std::endl;
        }
    };
}
