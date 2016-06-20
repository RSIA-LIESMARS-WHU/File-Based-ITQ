//////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2014 Gefu Tang <tanggefu@gmail.com>. All Rights Reserved.
///
/// This file is part of LSHBOX.
///
/// LSHBOX is free software: you can redistribute it and/or modify it under
/// the terms of the GNU General Public License as published by the Free
/// Software Foundation, either version 3 of the License, or(at your option)
/// any later version.
///
/// LSHBOX is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along
/// with LSHBOX. If not, see <http://www.gnu.org/licenses/>.
///
/// @version 0.1
/// @author Gefu Tang & Zhifeng Xiao
/// @date 2014.6.30
//////////////////////////////////////////////////////////////////////////////

/**
 * @file config.h
 *
 * @brief Analysis a config file.
 */
#include <string>
#include <fstream>
#include <iostream>
#include <map>
namespace lshbox
{
class op_config
{
public:
    op_config(std::string filename_): filename(filename_)
    {
        analysis_config();
    };
    ~op_config() {};
    void print_config()
    {
        for (std::map<std::string, std::string>::const_iterator mite = infos.begin(); mite != infos.end(); ++mite)
        {
            std::cout << mite->first << "=" << mite->second << std::endl;
        }
    }
    std::string get_value(std::string key)
    {
        return infos.find(key) == infos.end() ? "" : infos[key];
    }
private:
    std::map<std::string, std::string> infos;
    std::string filename;
    bool is_space(char c)
    {
        return ' ' == c || '\t' == c;
    }
    void trim(std::string &str)
    {
        if (str.empty())
        {
            return;
        }
        int i, start_pos, end_pos;
        for (i = 0; i != str.size(); ++i)
        {
            if (!is_space(str[i]))
            {
                break;
            }
        }
        if (i == str.size())
        {
            str = "";
            return;
        }
        start_pos = i;
        for (i = int(str.size()) - 1; i >= 0; --i)
        {
            if (!is_space(str[i]))
            {
                break;
            }
        }
        end_pos = i;
        str = str.substr(start_pos, end_pos - start_pos + 1);
    }
    bool analysis_line(const std::string &line, std::string &key, std::string &value)
    {
        if (line.empty())
        {
            return false;
        }
        int start_pos = 0, end_pos = int(line.size()) - 1, pos;
        if ((pos = int(line.find('#'))) != -1)
        {
            if (0 == pos)
            {
                return false;
            }
            end_pos = pos - 1;
        }
        std::string new_line = line.substr(start_pos, start_pos + 1 - end_pos);
        if ((pos = int(new_line.find('='))) == -1)
        {
            return false;
        }
        key = new_line.substr(0, pos);
        value = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));
        trim(key);
        if (key.empty())
        {
            return false;
        }
        trim(value);
        return true;
    }
    bool analysis_config()
    {
        infos.clear();
        std::ifstream infile(filename.c_str());
        if (!infile)
        {
            std::cout << "file open error" << std::endl;
            return false;
        }
        std::string line, key, value;
        while (std::getline(infile, line))
        {
            if (analysis_line(line, key, value))
            {
                infos[key] = value;
            }
        }
        infile.close();
        return true;
    }
};
}