#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "fort.hpp"

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

    public:
        void read_char(const char &c, bool &escaped, bool &quoted, vector<std::string> &line, std::string &cell, vector<vector<std::string>> &lines)
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

        // Count how often cell values occur
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
                            int rs = header ? row_sizes[i-1] : row_sizes[i];
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

        CSVSummarizer(std::string path, bool header, char sep, char line_break, char escape_char, char quotechar, int no_most_freq)
            : path(path), header(header), sep(sep), line_break(line_break), escape_char(escape_char), quotechar(quotechar), no_most_freq(no_most_freq)
        {
        }

        CSVSummarizer(std::string path, bool header, char sep) : CSVSummarizer(path, header, sep, '\n', '\\', '"', 3)
        {
        }

        virtual vector<vector<std::string>> read_lines(vector<int> &row_sizes, long long &file_size) = 0;
        virtual void summarize() = 0;

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
    };

}